# libzmap

Build & leverage [zmap](https://github.com/zmap/zmap) as a node.js module, no need to install a `zmap` binary.

[![npm](https://img.shields.io/npm/v/libzmap.svg)](https://npmjs.com/package/libzmap) [![CI](https://github.com/jas-/libzmap/actions/workflows/ci.yml/badge.svg)](https://github.com/jas-/libzmap/actions/workflows/ci.yml) ![Downloads](https://img.shields.io/npm/dm/libzmap.svg)

## install ##

![npm install libzmap](docs/install.gif)

To install `npm install libzmap`

The install compiles zmap v4.4.0 and needs Linux or macOS (x86_64 or arm64), node.js 22 or later, and the packages in `build tools`.

Some package managers (pnpm, yarn, bun, and npm with `ignore-scripts`) do not
run install scripts unless you allow them. If `require('libzmap')` gives the
error "Cannot find module './build/Release/zmap.node'", allow the install script
of `libzmap` and run `npm rebuild libzmap`.

### build tools ###

Example package requirements. The last line of each list is only necessary for an install from git.

<details>
<summary>Debian, Ubuntu (apt)</summary>

```sh
sudo apt-get install build-essential python3 pkg-config \
  libpcap-dev libjson-c-dev libgmp-dev libjudy-dev libunistring-dev \
  gengetopt flex byacc
```

Ubuntu 24.04 and Debian 12 package node.js 18; install node.js 22 or later from [nodejs.org](https://nodejs.org/en/download).
</details>

<details>
<summary>Fedora, RHEL, CentOS, Rocky, AlmaLinux (dnf)</summary>

```sh
sudo dnf install gcc gcc-c++ make python3 tar nodejs npm \
  libpcap-devel json-c-devel gmp-devel Judy-devel libunistring-devel \
  gengetopt flex byacc
```

On RHEL and its rebuilds, `Judy-devel` and `gengetopt` are in EPEL.
</details>

<details>
<summary>openSUSE (zypper)</summary>

```sh
sudo zypper install gcc gcc-c++ make python3 nodejs22 npm22 \
  libpcap-devel libjson-c-devel gmp-devel judy-devel libunistring-devel \
  gengetopt flex byacc
```
</details>

<details>
<summary>Arch Linux, Manjaro (pacman)</summary>

```sh
sudo pacman -S --needed base-devel python nodejs npm \
  libpcap json-c gmp judy libunistring \
  gengetopt flex byacc
```
</details>

<details>
<summary>Gentoo (emerge)</summary>

```sh
sudo emerge --ask net-libs/nodejs net-libs/libpcap dev-libs/json-c dev-libs/gmp \
  dev-libs/judy dev-libs/libunistring \
  dev-util/gengetopt sys-devel/flex dev-util/byacc
```
</details>

<details>
<summary>Alpine (apk)</summary>

```sh
sudo apk add build-base linux-headers python3 pkgconf nodejs npm \
  libpcap-dev json-c-dev gmp-dev judy-dev libunistring-dev \
  gengetopt flex byacc
```
</details>

<details>
<summary>macOS</summary>

```sh
xcode-select --install
brew install node json-c gmp judy libunistring pkgconf \
  gengetopt byacc
```

The Xcode command line tools supply the compiler, `make`, `python3` and `flex`. macOS includes `libpcap`.
</details>

### permissions ###

zmap sends raw packets, so a scan needs root (`sudo node script.js`). On Linux
you can instead give node.js the capabilities one time:

```sh
sudo setcap cap_net_raw,cap_net_admin+ep "$(readlink -f "$(command -v node)")"
```

## methods ##

* `zmap`        Runs a scan, `zmap(opts, callback)`
* `scan`        Runs a scan and returns it: `.done` (promise), events, `for await` and `.stop()`
* `isRunning`   True while a scan runs (one scan at a time in a process)

## options ##

* `range`       {String|Array}   Addresses or CIDR blocks to scan
* `ports`       {String}         Ports to scan; without ports the scan is ICMP echo
* `iface`       {String}         Network interface (default: the interface of the default route)
* `ipaddr`      {String}         Source address(es), e.g. `10.0.2.15-10.0.2.18`
* `mac`         {String}         Gateway MAC address
* `whitelist`   {String}         File of networks to scan
* `blacklist`   {String}         File of networks to skip
* `shards`      {Number}         With `shardtotal`, this scan is shard N of M (needs `seed`)
* `seed`        {Number}         Seed of the address order, the same on each shard
* `threads`     {Number}         Send threads
* `rate`        {Number}         Packets per second (default `10000`)
* `probeModule` {String}         `icmp_echoscan`, `tcp_synscan`, `udp`, `dns`, ...
* `dryrun`      {Boolean}        Send no packets (needs no root)

Any other zmap option works in camelCase, e.g. `maxRuntime`, `cooldownTime`, `outputFilter`.

## examples ##

More examples are in the `examples/` folder.

![JSON report](docs/default.gif)

```javascript
const zmap = require('libzmap');
const opts = {
  iface: 'eth0',
  ipaddr: '10.0.2.15-10.0.2.18',
  mac: '52:54:00:12:34:56',
  shards: 2,
  shardtotal: 4,
  seed: 1234,
  threads: 10,
  range: '10.0.2.0/24',
  blacklist: '/path/to/blacklist.conf',
  whitelist: '/path/to/whitelist.conf'
};

zmap.zmap(opts, function(err, report) {
  if (err) throw err;

  console.log(JSON.stringify(report, null, 2));
});
```

The report has `hosts` (each address that answered, with its ports), `results`
(each answer, with zmap's fields) and `metadata` (zmap's scan settings and statistics).

Each result as it arrives:

```javascript
for await (const result of zmap.scan(opts)) {
  console.log(JSON.stringify(result));
}
```

## tests ##

To test `npm test`

## license ##

This software is licensed under the [Apache License 2.0](LICENSE), the same license as zmap.

Copyright Jason Gerfen, 2014.
