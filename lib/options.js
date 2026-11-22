/*!
 * libzmap
 * Copyright(c) 2014 Jason Gerfen <jason.gerfen@gmail.com>
 * License: Apache-2.0
 */
'use strict';

// Converts scan options to zmap arguments. zmap parses them in-process, as its main() does.

// zmap options that the module accepts, from src/zmap/src/zopt.ggo.in. true: the option is a
// flag with no value. test/options.test.js checks this list against the zmap sources.
const OPTIONS = {
  'target-ports': false,
  'blocklist-file': false,
  'allowlist-file': false,
  'list-of-ips-file': false,
  rate: false,
  bandwidth: false,
  batch: false,
  'max-targets': false,
  'max-runtime': false,
  'max-results': false,
  probes: false,
  'cooldown-time': false,
  seed: false,
  retries: false,
  dryrun: true,
  shards: false,
  shard: false,
  'source-port': false,
  'validate-source-port': false,
  'source-ip': false,
  'gateway-mac': false,
  'source-mac': false,
  interface: false,
  iplayer: true,
  'probe-module': false,
  'probe-args': false,
  'probe-ttl': false,
  'output-fields': false,
  'output-filter': false,
  'dedup-method': false,
  'dedup-window-size': false,
  'metadata-file': false,
  'status-updates-file': false,
  quiet: true,
  notes: false,
  'user-metadata': false,
  config: false,
  'max-sendto-failures': false,
  'min-hitrate': false,
  'sender-threads': false,
  cores: false,
  'ignore-blocklist-errors': true,
};

// zmap options that the module sets itself, or that do not apply to a library.
const RESERVED = new Set([
  'output-module', 'output-file', 'output-args', 'no-header-row', 'log-file', 'log-directory',
  'list-probe-modules', 'list-output-modules', 'list-output-fields', 'help', 'version',
  'fast-dryrun', 'disable-syslog', 'netmap-wait-ping', 'verbosity',
]);

// Short names. The first group is the option names of libzmap 0.x (see the README).
const ALIASES = {
  iface: 'interface',
  ipaddr: 'source-ip',
  mac: 'gateway-mac',
  threads: 'sender-threads',
  probemodule: 'probe-module',
  blacklist: 'blocklist-file',
  whitelist: 'allowlist-file',
  shardtotal: 'shards',
  blocklist: 'blocklist-file',
  allowlist: 'allowlist-file',
  ports: 'target-ports',
  module: 'probe-module',
  fields: 'output-fields',
  filter: 'output-filter',
  cooldown: 'cooldown-time',
};

// Module options, not zmap options.
const OWN = new Set(['range', 'verbose', 'collect']);

// zmap's own default output: hosts that answer, once each.
const DEFAULT_FILTER = 'success = 1 && repeat = 0';

function kebab(key) {
  return key.replace(/[A-Z]/g, (c) => `-${c.toLowerCase()}`);
}

function toValue(key, value) {
  const values = Array.isArray(value) ? value : [value];
  for (const item of values) {
    if (item === null || !['string', 'number', 'bigint'].includes(typeof item)) {
      throw new TypeError(`option ${key} must be a string, number or array of those`);
    }
  }
  return values.map(String).join(',');
}

// Returns a Map of zmap option name to { key, value }.
function toSettings(options) {
  const settings = new Map();
  for (const [key, value] of Object.entries(options)) {
    if (OWN.has(key) || value === undefined || value === null) continue;
    let name = ALIASES[key] || kebab(key);
    // libzmap 0.x: { shards: N, shardtotal: M } means shard N of M.
    if (key === 'shards' && options.shardtotal !== undefined) name = 'shard';
    if (RESERVED.has(name)) {
      throw new TypeError(`option ${key} is not available: the module sets ${name} itself`);
    }
    if (!(name in OPTIONS)) {
      throw new TypeError(`unknown option: ${key}`);
    }
    if (settings.has(name)) {
      throw new TypeError(`option ${key} is given more than one time (as ${name})`);
    }
    settings.set(name, { key, value });
  }
  return settings;
}

function toTargets(range = [], settings) {
  const targets = Array.isArray(range) ? range : [range];
  for (const target of targets) {
    if (typeof target !== 'string' || target === '' || target.startsWith('-')) {
      throw new TypeError('range must be a string or an array of strings (addresses or CIDR blocks)');
    }
  }
  // With no targets, zmap scans the whole IPv4 Internet. Ask for that explicitly: range: '0.0.0.0/0'.
  if (targets.length === 0 && !settings.has('allowlist-file') && !settings.has('list-of-ips-file')) {
    throw new TypeError('give the targets: range, allowlist (whitelist) or listOfIpsFile');
  }
  return targets;
}

function toArgs(options) {
  if (options === null || typeof options !== 'object' || Array.isArray(options)) {
    throw new TypeError('options must be an object');
  }
  const settings = toSettings(options);

  const args = ['--output-module=node'];
  for (const [name, { key, value }] of settings) {
    if (OPTIONS[name]) {
      if (typeof value !== 'boolean') throw new TypeError(`option ${key} must be true or false`);
      if (value) args.push(`--${name}`);
    } else if (value !== false) {
      args.push(`--${name}=${toValue(key, value)}`);
    }
  }

  // Without ports, zmap's default probe (tcp_synscan) cannot run. libzmap 0.x used ICMP echo.
  if (!settings.has('probe-module') && !settings.has('target-ports')) {
    args.push('--probe-module=icmp_echoscan');
  }
  if (!settings.has('output-fields')) args.push('--output-fields=*');
  if (!settings.has('output-filter')) args.push(`--output-filter=${DEFAULT_FILTER}`);

  return [...args, ...toTargets(options.range, settings)];
}

function toVerbosity(verbose = 3) {
  const level = Number(verbose);
  if (!Number.isInteger(level) || level < 0 || level > 5) {
    throw new TypeError('verbose must be an integer from 0 to 5');
  }
  return level;
}

module.exports = { toArgs, toVerbosity, OPTIONS, RESERVED, ALIASES };
