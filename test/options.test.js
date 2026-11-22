'use strict';

// The option table in lib/options.js must match the zmap sources.

const test = require('node:test');
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');

const ggo = fs.readFileSync(path.join(__dirname, '..', 'src', 'zmap', 'src', 'zopt.ggo.in'), 'utf8');
const source = fs.readFileSync(path.join(__dirname, '..', 'lib', 'options.js'), 'utf8');

function upstream() {
  // option "name" short "description" ... optional [type]. No type: a flag.
  const options = {};
  const blocks = ggo.split(/\n(?=option )/).filter((b) => b.startsWith('option '));
  for (const block of blocks) {
    const name = /^option\s+"([^"]+)"/.exec(block)[1];
    const text = block.split(/\n(?=section |text )/)[0];
    options[name] = !/\b(string|int|longlong|float|double)\b\s*$/m.test(text.replace(/"(?:[^"\\]|\\.)*"/g, '""'));
  }
  return options;
}

function local(name) {
  const table = new RegExp(`const ${name} = \\{([\\s\\S]*?)\\n\\};`).exec(source)[1];
  const out = {};
  for (const m of table.matchAll(/^\s+'?([a-z-]+)'?: (true|false),/gm)) out[m[1]] = m[2] === 'true';
  return out;
}

function reserved() {
  const set = /const RESERVED = new Set\(\[([\s\S]*?)\]\);/.exec(source)[1];
  return [...set.matchAll(/'([a-z-]+)'/g)].map((m) => m[1]);
}

test('each zmap option is in OPTIONS or RESERVED, with the right kind', () => {
  const zmapOptions = upstream();
  const options = local('OPTIONS');
  const skip = new Set(reserved());
  for (const [name, flag] of Object.entries(zmapOptions)) {
    if (skip.has(name)) continue;
    assert.ok(name in options, `zmap option ${name} is not in OPTIONS`);
    assert.equal(options[name], flag, `${name} must be ${flag ? 'a flag' : 'an option with a value'}`);
  }
  for (const name of Object.keys(options)) {
    assert.ok(name in zmapOptions, `OPTIONS has ${name}, which zmap does not have`);
  }
});
