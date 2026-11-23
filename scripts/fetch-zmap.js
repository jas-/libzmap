'use strict';
// Gets the zmap sources before the build: existing files, git submodule, or pinned download.

const fs = require('node:fs');
const os = require('node:os');
const path = require('node:path');
const crypto = require('node:crypto');
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

// SHA-256 of the file tree: each file's path and SHA-256, sorted by path. Ignores .git.
// Hashes the files, not the archive: GitHub does not keep archive bytes the same.
function treeHash(dir) {
  const lines = [];
  const walk = (rel) => {
    for (const entry of fs.readdirSync(path.join(dir, rel), { withFileTypes: true })) {
      const name = rel ? `${rel}/${entry.name}` : entry.name;
      if (entry.name === '.git') continue;
      if (entry.isDirectory()) walk(name);
      else if (entry.isFile()) {
        const sum = crypto.createHash('sha256').update(fs.readFileSync(path.join(dir, name))).digest('hex');
        lines.push(`${name} ${sum}`);
      }
    }
  };
  walk('');
  return crypto.createHash('sha256').update(lines.sort().join('\n')).digest('hex');
}

async function download() {
  const url = `${pin.repository}/archive/${pin.commit}.tar.gz`;
  console.log(`libzmap: downloading zmap ${pin.version} (${pin.commit.slice(0, 12)})`);
  const res = await fetch(url);
  if (!res.ok) throw new Error(`download of ${url} failed: HTTP ${res.status}`);

  const tmp = fs.mkdtempSync(path.join(os.tmpdir(), 'libzmap-'));
  try {
    const archive = path.join(tmp, 'zmap.tar.gz');
    const tree = path.join(tmp, 'zmap');
    fs.writeFileSync(archive, Buffer.from(await res.arrayBuffer()));
    fs.mkdirSync(tree);
    execFileSync('tar', ['-xzf', archive, '-C', tree, '--strip-components=1'], { stdio: 'inherit' });

    const actual = treeHash(tree);
    if (actual !== pin.sha256) {
      throw new Error(`the zmap sources from ${url} do not match package.json ` +
                      `(sha256 ${actual}, expected ${pin.sha256})`);
    }
    fs.mkdirSync(dest, { recursive: true });
    fs.cpSync(tree, dest, { recursive: true });
  } finally {
    fs.rmSync(tmp, { recursive: true, force: true });
  }
}

// Before a publish: the submodule must be the pinned commit, and its files must give the pinned hash.
function check() {
  const actual = git(['-C', dest, 'rev-parse', 'HEAD']);
  if (actual !== pin.commit) {
    console.error(`libzmap: src/zmap is at ${actual}, but package.json pins ${pin.commit}.`);
    console.error('Set "zmap.commit" in package.json to the submodule commit.');
    process.exit(1);
  }
  if (git(['-C', dest, 'status', '--porcelain'])) {
    console.error('libzmap: src/zmap has changes. The published sources must be the pinned commit.');
    process.exit(1);
  }
  const sum = treeHash(dest);
  if (sum !== pin.sha256) {
    console.error(`libzmap: src/zmap has sha256 ${sum}, but package.json pins ${pin.sha256}.`);
    console.error('Set "zmap.sha256" in package.json to that value.');
    process.exit(1);
  }
}

if (process.argv.includes('--hash')) {
  console.log(treeHash(process.argv[process.argv.indexOf('--hash') + 1] || dest));
  process.exit(0);
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
