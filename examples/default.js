/*!
 * libzmap
 * Copyright(c) 2014 Jason Gerfen <jason.gerfen@gmail.com>
 * License: Apache-2.0
 */

'use strict'

// The example from the README: the option names of libzmap 0.x, with the
// callback form. Change iface, ipaddr, mac and range for your network.
// zmap needs a seed for a sharded scan: every shard must use the same seed.
//
//   sudo node examples/default.js

const path = require('path');
const zmap = require('../');
const opts = {
  iface: 'eth0',
  ipaddr: '10.0.2.15-10.0.2.18', // source addresses
  mac: '52:54:00:12:34:56', // gateway MAC address
  shards: 2, // this scan is shard 2 ...
  shardtotal: 4, // ... of 4
  seed: 1234,
  threads: 10,
  range: '10.0.2.0/24',
  blacklist: path.join(__dirname, 'blacklist.conf'),
  whitelist: path.join(__dirname, 'whitelist.conf')
};


zmap.zmap(opts, (err, report) => {
  if (err) throw err;

  console.log(JSON.stringify(report, null, 2));
});
