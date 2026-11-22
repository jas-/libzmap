/*!
 * libzmap
 * Copyright(c) 2014 Jason Gerfen <jason.gerfen@gmail.com>
 * License: Apache-2.0
 */

'use strict'

// Allowlist and blocklist files. zmap scans the union of the allowlist and
// the range, without the blocklist. Each line has a CIDR block or an address.

const path = require('path');
const zmap = require('../');
const opts = {
  allowlist: path.join(__dirname, 'whitelist.conf'),
  blocklist: path.join(__dirname, 'blacklist.conf'),
  ignoreBlocklistErrors: true // skip lines that are not valid, do not stop
};


zmap.zmap(opts, (err, report) => {
  if (err) throw err;

  const { blocklist_total_allowed, blocklist_total_not_allowed } = report.metadata;
  console.log(JSON.stringify({ blocklist_total_allowed, blocklist_total_not_allowed, hosts: report.hosts }, null, 2));
});
