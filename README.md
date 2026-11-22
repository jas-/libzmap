# libzmap

[Node.js](http://nodejs.org) bindings for the [zmap](https://github.com/zmap/zmap) network scanner. zmap runs inside the Node.js process, and the scan results come back as JSON.

[![npm](https://img.shields.io/npm/v/libzmap.svg)](https://npmjs.com/package/libzmap) [![CI](https://github.com/jas-/libzmap/actions/workflows/ci.yml/badge.svg)](https://github.com/jas-/libzmap/actions/workflows/ci.yml)

The module compiles zmap v4.4.0 from the upstream sources. It does not start the `zmap` program.

## Requirements

- Linux or macOS, with Node.js 18 or later.
- `make`, a C compiler and `python3`. The install compiles zmap with `node-gyp`.
- The libraries of zmap: `libpcap`, `json-c`, `gmp`, `Judy` and `libunistring`.
- From git only: `gengetopt`, `flex` and `byacc` (or `bison`). The npm package has the generated files.
- Root, or `CAP_NET_RAW` on Linux, to send packets. See [Run without root](#run-without-root).

<details>
<summary>Debian, Ubuntu (apt)</summary>

```sh
sudo apt-get install build-essential python3 pkg-config nodejs npm \
  libpcap-dev libjson-c-dev libgmp-dev libjudy-dev libunistring-dev \
  gengetopt flex byacc
```
</details>

<details>
<summary>Fedora, RHEL, Rocky, AlmaLinux (dnf)</summary>

```sh
sudo dnf install gcc gcc-c++ make python3 tar nodejs npm \
  libpcap-devel json-c-devel gmp-devel Judy-devel libunistring-devel \
  gengetopt flex byacc
```
</details>

<details>
<summary>Arch Linux (pacman)</summary>

```sh
sudo pacman -S --needed base-devel python nodejs npm \
  libpcap json-c gmp judy libunistring gengetopt flex byacc
```
</details>

<details>
<summary>Alpine (apk)</summary>

```sh
sudo apk add build-base linux-headers python3 pkgconf nodejs npm \
  libpcap-dev json-c-dev gmp-dev judy-dev libunistring-dev gengetopt flex byacc
```
</details>

<details>
<summary>macOS (Homebrew)</summary>

```sh
xcode-select --install
brew install node json-c gmp judy libunistring gengetopt byacc pkgconf
```

macOS includes `libpcap` and `flex`.
</details>

## Install

From npm:

```sh
npm install libzmap
```

From git:

```sh
git clone https://github.com/jas-/libzmap.git
cd libzmap
npm install
```

The install gets the zmap sources (git submodule, or the pinned release), generates the option parser and the filter parser, and compiles the module.

## Run without root

zmap sends raw packets, so a scan needs root: `sudo node your-script.js`.

On Linux, you can give the Node.js binary the raw-socket capabilities one time:

```sh
sudo setcap cap_net_raw,cap_net_admin+ep "$(readlink -f "$(command -v node)")"
```

The capability applies to every script that this `node` binary runs. Run the command again after a Node.js update. macOS has no such capability, so use `sudo` there.

A dry run (`dryrun: true`) sends no packets and needs no root.

## Example use

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

zmap.zmap(opts, (err, report) => {
  if (err) throw err;
  console.log(JSON.stringify(report, null, 2));
});
```

The same code is in `examples/default.js`. These are the option names of libzmap 0.x:

| Option | zmap option | Value |
| --- | --- | --- |
| `iface` | `--interface` | The network interface. |
| `ipaddr` | `--source-ip` | The source addresses: one, a range (`a-b`) or a list. The host must own them, or the answers do not arrive. |
| `mac` | `--gateway-mac` | The MAC address of the gateway. |
| `shards`, `shardtotal` | `--shard`, `--shards` | This scan is shard `shards` (from 0) of `shardtotal`. |
| `seed` | `--seed` | The seed of the address order. A sharded scan must have one, the same on each shard. |
| `threads` | `--sender-threads` | The number of send threads. |
| `range` | targets | An address, a CIDR block, or an array of those. |
| `blacklist`, `whitelist` | `--blocklist-file`, `--allowlist-file` | Files with one CIDR block on each line. |

Every other zmap option is also available, in camelCase: `ports` (or `targetPorts`), `probeModule`, `probeArgs`, `rate`, `bandwidth`, `maxTargets`, `maxRuntime`, `maxResults`, `cooldown` (or `cooldownTime`), `outputFields`, `outputFilter`, `dryrun`, `sourceMac`, `sourcePort`, `probeTtl`, `dedupMethod` and the rest of `zmap --help`. Options that take a list also take an array.

The module chooses some settings itself:

- The output always goes to Node.js as JSON. `outputModule`, `outputFile` and the log file options are not available.
- Without `ports` and `probeModule`, the probe module is `icmp_echoscan`, as in libzmap 0.x.
- Each result has all fields (`outputFields: '*'`), and the filter is zmap's own default, `success = 1 && repeat = 0`. Use `outputFilter: ''` for every answer.
- A scan must have targets: `range`, `whitelist` or `listOfIpsFile`. Use `range: '0.0.0.0/0'` for the whole IPv4 Internet.
- `verbose` (0 to 5) sets the log level. `collect: false` keeps the results out of the final report.

A process runs one scan at a time, because zmap keeps global state. Scan only networks that you own or have permission to scan.

## The report

A TCP SYN scan of two hosts with ports 80 and 8080 open (the first result only):

```json
{
  "startedAt": "2026-11-22T23:28:41.480Z",
  "finishedAt": "2026-11-22T23:28:42.514Z",
  "durationMs": 1034,
  "counts": { "results": 4, "success": 4, "hosts": 2 },
  "hosts": [
    { "ip": "10.78.0.2", "ports": [80, 8080] },
    { "ip": "10.78.0.3", "ports": [80, 8080] }
  ],
  "results": [
    {
      "saddr": "10.78.0.2",
      "daddr": "10.78.0.1",
      "sport": 80,
      "dport": 51889,
      "classification": "synack",
      "success": true,
      "repeat": false,
      "cooldown": true,
      "ttl": 64,
      "timestamp_str": "2026-11-22T23:28:41.496+0000"
    }
  ],
  "metadata": { "probe_module": "tcp_synscan", "packets_sent": 4 }
}
```

- `results` has one object for each answer, with the fields of zmap's JSON output.
- `hosts` has one entry for each address that answered, with the ports that answered.
- `metadata` is zmap's scan metadata: the configuration and the statistics.

When a scan fails, the error has `code: 'ERR_ZMAP_SCAN'`, the zmap log lines in `err.log`, and the report so far in `err.report`.

## Promise, events and streams

```javascript
const scan = zmap.scan(opts);

// One report at the end
scan.done.then((report) => console.log(JSON.stringify(report)));

// Or each result as it arrives
for await (const result of scan) {
  console.log(JSON.stringify(result));
}

// Or events
scan.on('result', (result) => console.log(JSON.stringify(result)));
scan.on('progress', (progress) => console.log(JSON.stringify(progress)));
scan.on('log', (log) => console.log(JSON.stringify(log)));
scan.on('done', (report) => console.log(JSON.stringify(report.counts)));
scan.on('error', (err) => console.error(err.message));

// Stop: the first call waits the cooldown for late answers, force stops at once
scan.stop({ force: true });
```

## Examples

The `examples` folder has a script for each use. Run them as root, and change the ranges to networks that you may scan.

| Example | Shows |
| --- | --- |
| `default.js` | The example above. |
| `promise.js` | `scan(opts).done`, with the interface and addresses found by zmap. |
| `icmp.js` | A ping sweep of the network of each interface. |
| `tcp-syn.js` | A TCP SYN scan of some ports. |
| `udp.js` | A UDP scan with a payload. |
| `dns.js` | The DNS probe module. |
| `sharding.js` | One shard of a scan that several machines share. |
| `lists.js` | Allowlist and blocklist files. |
| `limits.js` | Rate, target, time and result limits. |
| `fields.js` | Selected fields and a filter. |
| `all-responses.js` | Every answer, also RST and duplicates. |
| `stream.js` | Results as JSON lines, with `for await`. |
| `events.js` | Every event as JSON. |
| `stop.js` | A scan that stops early. |
| `dryrun.js` | A dry run, which needs no root. |
| `report2file.js` | The report, metadata and status in files. |
| `queue.js` | Several scans, one after the other. |

## Tests

```sh
npm test
```

The tests use dry runs, so they send no packets and need no root.

## Credits

[zmap](https://github.com/zmap/zmap) is the work of the ZMap team and its contributors.

## License

This software is licensed under the [Apache License 2.0](LICENSE).

Copyright Jason Gerfen, 2014.
