'use strict';
// Prints compiler or linker flags for the libraries zmap needs, for binding.gyp.
//
//   node scripts/deps.js include   include directories
//   node scripts/deps.js libs      linker flags
//
// pkg-config is used when it knows a library. Otherwise the usual prefixes are searched:
// Homebrew (/opt/homebrew, /usr/local), MacPorts (/opt/local) and /usr.

const fs = require('node:fs');
const path = require('node:path');
const { execFileSync } = require('node:child_process');

const PREFIXES = ['/opt/homebrew', '/usr/local', '/opt/local', '/usr'];

// [pkg-config name, header to find, library name]
const LIBS = [
  ['json-c', 'json-c/json.h', 'json-c'],
  ['libpcap', 'pcap/pcap.h', 'pcap'],
  ['gmp', 'gmp.h', 'gmp'],
  [null, 'Judy.h', 'Judy'],
  [null, 'unistr.h', 'unistring'],
];

function pkgConfig(args) {
  try {
    return execFileSync('pkg-config', args, { encoding: 'utf8', stdio: ['ignore', 'pipe', 'ignore'] }).trim();
  } catch {
    return null;
  }
}

function findHeader(header) {
  const multiarch = fs.existsSync('/usr/include')
    ? fs.readdirSync('/usr/include').filter((d) => /-linux-gnu|-linux-musl/.test(d)).map((d) => `/usr/include/${d}`)
    : [];
  for (const dir of [...PREFIXES.map((p) => `${p}/include`), ...multiarch]) {
    if (fs.existsSync(path.join(dir, header))) return dir;
  }
  return null;
}

function findLibDir(name) {
  const dirs = PREFIXES.flatMap((p) => [`${p}/lib`, `${p}/lib64`, `${p}/lib/${process.arch === 'arm64' ? 'aarch64' : 'x86_64'}-linux-gnu`]);
  for (const dir of dirs) {
    if (['so', 'dylib', 'a'].some((ext) => fs.existsSync(path.join(dir, `lib${name}.${ext}`)))) return dir;
  }
  return null;
}

const include = new Set();
const libs = [];
const missing = [];

for (const [pkg, header, lib] of LIBS) {
  const cflags = pkg && pkgConfig(['--cflags-only-I', pkg]);
  const ldflags = pkg && pkgConfig(['--libs', pkg]);
  if (cflags !== null && ldflags !== null && pkg) {
    for (const flag of cflags.split(/\s+/).filter(Boolean)) include.add(flag.replace(/^-I/, ''));
    // json-c headers are included as <json.h>.
    if (pkg === 'json-c') {
      const dir = findHeader(header);
      if (dir) include.add(path.join(dir, 'json-c'));
    }
    libs.push(...ldflags.split(/\s+/).filter(Boolean));
    continue;
  }
  const dir = findHeader(header);
  // macOS has libpcap in the SDK, which the compiler searches already.
  if (!dir && process.platform === 'darwin' && lib === 'pcap') {
    libs.push('-lpcap');
    continue;
  }
  if (!dir) missing.push(header);
  else {
    include.add(dir);
    if (pkg === 'json-c') include.add(path.join(dir, 'json-c'));
  }
  const libDir = findLibDir(lib);
  if (libDir && !['/usr/lib', '/usr/lib64'].includes(libDir)) libs.push(`-L${libDir}`);
  libs.push(`-l${lib}`);
}

if (missing.length > 0) {
  console.error(`libzmap: headers not found: ${missing.join(', ')}. Install the packages in the README.`);
  process.exit(1);
}

const mode = process.argv[2];
if (mode === 'include') console.log([...include].join(' '));
else if (mode === 'libs') console.log([...new Set(libs)].join(' '));
else {
  console.error('usage: node scripts/deps.js include|libs');
  process.exit(2);
}
