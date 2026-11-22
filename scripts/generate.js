'use strict';
// Makes the generated zmap sources (option parser, filter lexer and parser) in src/gen.
// The npm package includes src/gen, so an install from npm needs no gengetopt, flex or byacc.

const fs = require('node:fs');
const path = require('node:path');
const { execFileSync } = require('node:child_process');

const root = path.join(__dirname, '..');
const zsrc = path.join(root, 'src', 'zmap', 'src');
const gen = path.join(root, 'src', 'gen');
const { zmap: pin } = require(path.join(root, 'package.json'));

const outputs = ['zopt.c', 'zopt.h', 'lexer.c', 'lexer.h', 'parser.c', 'parser.h'].map((f) => path.join(gen, f));
const inputs = ['zopt.ggo.in', 'lexer.l', 'parser.y'].map((f) => path.join(zsrc, f));

function mtime(file) {
  try {
    return fs.statSync(file).mtimeMs;
  } catch {
    return 0;
  }
}

function upToDate() {
  const oldest = Math.min(...outputs.map(mtime));
  return oldest > 0 && inputs.every((f) => !fs.existsSync(f) || mtime(f) <= oldest);
}

function have(tool) {
  try {
    execFileSync(tool, ['--version'], { stdio: 'ignore' });
    return true;
  } catch {
    return false;
  }
}

function run(tool, args) {
  if (!have(tool)) {
    throw new Error(`${tool} is necessary to generate src/gen. Install it (see the README) and try again.`);
  }
  execFileSync(tool, args, { cwd: gen, stdio: 'inherit' });
}

function main() {
  if (upToDate()) return;
  if (!fs.existsSync(path.join(zsrc, 'zopt.ggo.in'))) {
    throw new Error('no zmap sources in src/zmap. Run: node scripts/fetch-zmap.js');
  }
  fs.mkdirSync(gen, { recursive: true });

  // The same steps as upstream src/CMakeLists.txt.
  const ggo = fs.readFileSync(path.join(zsrc, 'zopt.ggo.in'), 'utf8').replace(/@ZMAP_VERSION@/g, pin.version);
  fs.writeFileSync(path.join(gen, 'zopt.ggo'), ggo);
  run('gengetopt', ['-C', '--no-help', '--no-version', '--unamed-opts=SUBNETS', '-i', 'zopt.ggo', '-F', 'zopt']);
  run('flex', ['-o', 'lexer.c', '--header-file=lexer.h', path.join(zsrc, 'lexer.l')]);

  // byacc, or bison in yacc mode when byacc is not installed.
  if (have('byacc')) run('byacc', ['-d', '-o', 'parser.c', path.join(zsrc, 'parser.y')]);
  else run('bison', ['-y', '-d', '-o', 'parser.c', path.join(zsrc, 'parser.y')]);
  console.log('libzmap: generated src/gen');
}

try {
  main();
} catch (err) {
  console.error(`libzmap: ${err.message}`);
  process.exit(1);
}
