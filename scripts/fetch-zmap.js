'use strict';
// Gets the zmap sources before the build: existing files, git submodule, or pinned download.

const fs = require('node:fs');
const os = require('node:os');
const path = require('node:path');
const { execFileSync } = require('node:child_process');

const root = path.join(__dirname, '..');
const dest = path.join(root, 'src', 'zmap');
const srcDir = path.join(dest, 'src');
const { zmap: pin } = require(path.join(root, 'package.json'));

function hasSources() {
  return fs.existsSync(path.join(srcDir, 'zmap.c'));
}

function git(args, opts = {}) {
  return execFileSync('git', args, { cwd: root, stdio: ['ignore', 'pipe', 'inherit'], ...opts })
    .toString().trim();
}

function isGitCheckout() {
  if (!fs.existsSync(path.join(root, '.git')) || !fs.existsSync(path.join(root, '.gitmodules'))) {
    return false;
  }
  try {
    return git(['rev-parse', '--show-toplevel']) === fs.realpathSync(root);
  } catch {
    return false;
  }
}

async function download() {
  const url = `${pin.repository}/archive/${pin.commit}.tar.gz`;
  console.log(`libzmap: downloading zmap ${pin.version} (${pin.commit.slice(0, 12)})`);
  const res = await fetch(url);
  if (!res.ok) throw new Error(`download of ${url} failed: HTTP ${res.status}`);

  const tmp = fs.mkdtempSync(path.join(os.tmpdir(), 'libzmap-'));
  try {
    const archive = path.join(tmp, 'zmap.tar.gz');
    fs.writeFileSync(archive, Buffer.from(await res.arrayBuffer()));
    fs.mkdirSync(dest, { recursive: true });
    execFileSync('tar', ['-xzf', archive, '-C', dest, '--strip-components=1'], { stdio: 'inherit' });
  } finally {
    fs.rmSync(tmp, { recursive: true, force: true });
  }
}

function check() {
  const actual = git(['-C', dest, 'rev-parse', 'HEAD']);
  if (actual !== pin.commit) {
    console.error(`libzmap: src/zmap is at ${actual}, but package.json pins ${pin.commit}.`);
    console.error('Set "zmap.commit" in package.json to the submodule commit.');
    process.exit(1);
  }
}

// npm only warns about "engines". Stop the install here instead.
function checkNode() {
  const major = Number(process.versions.node.split('.')[0]);
  if (major < 22) {
    throw new Error(`Node.js 22 or later is necessary. This is Node.js ${process.versions.node}.`);
  }
}

async function main() {
  checkNode();
  if (process.argv.includes('--check')) {
    check();
    return;
  }
  if (hasSources()) return;

  if (isGitCheckout()) {
    console.log('libzmap: running git submodule update --init');
    git(['submodule', 'update', '--init', '--recursive'], { stdio: 'inherit' });
  } else {
    await download();
  }

  if (!hasSources()) {
    throw new Error(`no zmap sources in ${srcDir}`);
  }
}

main().catch((err) => {
  console.error(`libzmap: ${err.message}`);
  process.exit(1);
});
