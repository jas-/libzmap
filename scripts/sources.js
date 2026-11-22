'use strict';
// Lists the upstream zmap sources for binding.gyp, without the files that src/ replaces.
const fs = require('node:fs');
const path = require('node:path');

const root = path.join(__dirname, '..');
const zmap = path.join(root, 'src', 'zmap');

if (!fs.existsSync(path.join(zmap, 'src', 'zmap.c'))) {
  console.error('src/zmap is empty. Run: node scripts/fetch-zmap.js');
  process.exit(1);
}

const linux = process.platform === 'linux';

// lib/logger.c: replaced by src/hooks.c. lib/blocklist.c: compiled through src/blocklist-hooks.c.
const lib = fs.readdirSync(path.join(zmap, 'lib'))
  .filter((f) => f.endsWith('.c') && f !== 'logger.c' && f !== 'blocklist.c')
  .sort()
  .map((f) => `src/zmap/lib/${f}`);

// zmap.c: compiled through src/runner.c. monitor.c: through src/monitor.c.
const core = [
  'aesrand.c', 'cyclic.c', 'expression.c', 'fieldset.c', 'filter.c', 'get_gateway.c',
  'iterator.c', 'ports.c', 'recv.c', 'recv-pcap.c', 'send.c', 'shard.c', 'socket.c',
  'state.c', 'summary.c', 'utility.c', 'validate.c', 'zopt_compat.c',
  ...(linux ? ['socket-linux.c', 'send-linux.c'] : ['socket-bsd.c', 'send-bsd.c']),
].map((f) => `src/zmap/src/${f}`);

const probes = fs.readdirSync(path.join(zmap, 'src', 'probe_modules'))
  .filter((f) => f.endsWith('.c'))
  .sort()
  .map((f) => `src/zmap/src/probe_modules/${f}`);

// output_modules.c: replaced by src/output.c, which adds the module for Node.js.
const outputs = ['module_csv.c', 'module_json.c'].map((f) => `src/zmap/src/output_modules/${f}`);

const generated = ['src/gen/lexer.c', 'src/gen/parser.c'];

for (const file of [...lib, ...core, ...probes, ...outputs, ...generated]) console.log(file);
