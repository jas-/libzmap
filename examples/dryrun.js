/*!
 * libzmap
 * Copyright(c) 2014 Jason Gerfen <jason.gerfen@gmail.com>
 * License: Apache-2.0
 */

'use strict'

// A dry run sends no packets, so it needs no root. It checks the options and
// shows what the scan would do. The report has zmap's metadata.
// With verbose: 4 the log events show each probe.

const zmap = require('../');
const opts = {
  dryrun: true,
  ports: '80,443',
  range: ['192.0.2.0/28'],
  gatewayMac: '52:54:00:12:34:56', // no ARP lookup in a dry run
  sourceMac: '02:00:00:00:00:01'
};


zmap.zmap(opts, (err, report) => {
  if (err) throw err;

  const { packets_sent, targets_scanned, iface, source_ips, probe_module } = report.metadata;
  console.log(JSON.stringify({ packets_sent, targets_scanned, iface, source_ips, probe_module }, null, 2));
});
