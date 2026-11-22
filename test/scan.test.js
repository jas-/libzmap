'use strict';

// Dry runs send no packets, so the tests need no root.

const test = require('node:test');
const assert = require('node:assert/strict');
const zmap = require('..');

// A dry run does not look up the interface, addresses or MAC addresses that it is given.
const DRY = {
  dryrun: true,
  iface: process.platform === 'darwin' ? 'lo0' : 'lo',
  sourceIp: '10.9.9.9',
  sourceMac: '00:11:22:33:44:55',
  gatewayMac: '66:55:44:33:22:11',
  cooldown: 0,
};

function dry(options) {
  return zmap.scan({ ...DRY, ...options }).done;
}

test('options become zmap settings', async () => {
  const report = await dry({ range: ['10.0.0.0/30', '10.0.1.0/30'], ports: '80,443', rate: 5000, threads: 1 });
  const m = report.metadata;
  assert.equal(m.probe_module, 'tcp_synscan');
  assert.equal(m.output_module, 'node');
  assert.deepEqual(m.target_ports, [80, 443]);
  assert.equal(m.rate, 5000);
  assert.equal(m.senders, 1);
  assert.equal(m.packets_sent, 16);
  // zmap's metadata drops the prefix length here: blocklist parsing edits the string in place.
  assert.deepEqual(m.cli_cidr_destinations.map((c) => c.split('/')[0]).sort(), ['10.0.0.0', '10.0.1.0']);
  assert.equal(m.output_filter, 'success = 1 && repeat = 0');
  assert.deepEqual(report.hosts, []);
  assert.equal(typeof report.durationMs, 'number');
});

test('without ports the probe module is icmp_echoscan', async () => {
  const report = await dry({ range: '10.0.0.0/30' });
  assert.equal(report.metadata.probe_module, 'icmp_echoscan');
  assert.equal(report.metadata.packets_sent, 4);
});

test('the option names of libzmap 0.x still work', async () => {
  const { sourceIp, gatewayMac, iface, ...rest } = DRY;
  const report = await zmap.scan({
    ...rest,
    iface,
    ipaddr: '10.0.2.15-10.0.2.18',
    mac: '52:54:00:12:34:56',
    shards: 2,
    shardtotal: 4,
    seed: 1234,
    threads: 2,
    range: '10.0.2.0/24',
    blacklist: `${__dirname}/../examples/blacklist.conf`,
    whitelist: `${__dirname}/../examples/whitelist.conf`,
  }).done;
  const m = report.metadata;
  assert.equal(m.shard_num, 2);
  assert.equal(m.total_shards, 4);
  assert.equal(m.seed, 1234);
  assert.equal(m.senders, 2);
  assert.equal(m.gateway_mac, '52:54:00:12:34:56');
  assert.deepEqual(m.source_ips, ['10.0.2.15', '10.0.2.16', '10.0.2.17', '10.0.2.18']);
  assert.match(m.blocklist_filename, /blacklist\.conf$/);
  assert.match(m.allowlist_filename, /whitelist\.conf$/);
  // 512 addresses in the allowlist and range, 4 in the blocklist.
  assert.equal(m.blocklist_total_allowed, 508);
});

test('bad options throw a TypeError before the scan starts', () => {
  assert.throws(() => zmap.scan('x'), TypeError);
  assert.throws(() => zmap.scan(null), TypeError);
  assert.throws(() => zmap.scan({ range: '10.0.0.1', bogus: 1 }), /unknown option: bogus/);
  assert.throws(() => zmap.scan({ range: '10.0.0.1', outputModule: 'csv' }), /not available/);
  assert.throws(() => zmap.scan({ range: '10.0.0.1', logFile: '/tmp/x' }), /not available/);
  assert.throws(() => zmap.scan({ range: [{}] }), TypeError);
  assert.throws(() => zmap.scan({ range: '--help' }), TypeError);
  assert.throws(() => zmap.scan({ range: '10.0.0.1', rate: {} }), TypeError);
  assert.throws(() => zmap.scan({ range: '10.0.0.1', dryrun: 'yes' }), /true or false/);
  assert.throws(() => zmap.scan({ range: '10.0.0.1', ports: 80, targetPorts: 81 }), /more than one time/);
  assert.throws(() => zmap.scan({ range: '10.0.0.1', verbose: 9 }), TypeError);
  assert.equal(zmap.isRunning(), false);
});

test('a scan needs targets', () => {
  assert.throws(() => zmap.scan({ ...DRY }), /give the targets/);
  assert.throws(() => zmap.scan({ ...DRY, range: [] }), /give the targets/);
});

test('zmap errors reject scan.done, and the next scan works', async () => {
  await assert.rejects(dry({ range: '10.0.0.0/30', rate: 'abc' }), (err) => {
    assert.equal(err.code, 'ERR_ZMAP_SCAN');
    assert.match(err.message, /invalid numeric value: abc/);
    assert.ok(err.report);
    return true;
  });
  await assert.rejects(dry({ range: '10.0.0.0/30', shard: 0, shards: 2 }), /seed/);
  await assert.rejects(dry({ range: '10.0.0.0/30', probeModule: 'nope' }), /probe module \(nope\) does not exist/);
  await assert.rejects(dry({ range: '10.0.0.0/30', blocklist: '/does/not/exist' }), /unable to open/);
  const report = await dry({ range: '10.0.0.0/30' });
  assert.equal(report.metadata.packets_sent, 4);
});

test('one scan at a time', async () => {
  const first = zmap.scan({ ...DRY, range: '10.0.0.0/24', rate: 2000 });
  assert.equal(zmap.isRunning(), true);
  assert.throws(() => zmap.scan({ ...DRY, range: '10.0.0.1' }), (err) => err.code === 'ERR_ZMAP_BUSY');
  await first.done;
  assert.equal(zmap.isRunning(), false);
});

test('stop() ends a scan early', async () => {
  const scan = zmap.scan({ ...DRY, range: '10.0.0.0/16', rate: 100 });
  const started = Date.now();
  setTimeout(() => scan.stop(), 300);
  const report = await scan.done;
  assert.ok(Date.now() - started < 5000, `took ${Date.now() - started} ms`);
  assert.ok(report.metadata.packets_sent < 65536);
});

test('for await ends with the scan', async () => {
  const seen = [];
  for await (const result of zmap.scan({ ...DRY, range: '10.0.0.0/30' })) seen.push(result);
  assert.deepEqual(seen, []);
});

test('log events, and verbose 4 shows the probes of a dry run', async () => {
  const scan = zmap.scan({ ...DRY, range: '10.0.0.0/31', verbose: 4 });
  const logs = [];
  scan.on('log', (l) => logs.push(l));
  await scan.done;
  assert.ok(logs.every((l) => typeof l.message === 'string' && typeof l.module === 'string'));
  assert.ok(logs.some((l) => l.level === 'debug'));
  assert.ok(logs.some((l) => /daddr: 10\.0\.0\.[01]/.test(l.message)), 'probe dump');
});

test('zmap(options, callback), the API of 0.x', async () => {
  const report = await new Promise((resolve, reject) => {
    zmap.zmap({ ...DRY, range: '10.0.0.0/30' }, (err, r) => (err ? reject(err) : resolve(r)));
  });
  assert.equal(report.metadata.packets_sent, 4);
  const err = await new Promise((resolve) => zmap.zmap({ ...DRY, range: '10.0.0.0/30', rate: 'x' }, resolve));
  assert.equal(err.code, 'ERR_ZMAP_SCAN');
});

test('the report is JSON', async () => {
  const report = await dry({ range: '10.0.0.0/30', seed: '9223372036854775807' });
  assert.deepEqual(JSON.parse(JSON.stringify(report)), report);
  // A 64-bit seed is kept as a string, not rounded.
  assert.equal(report.metadata.seed, '9223372036854775807');
});
