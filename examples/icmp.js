/*!
 * libzmap
 * Copyright(c) 2014 Jason Gerfen <jason.gerfen@gmail.com>
 * License: Apache-2.0
 */

'use strict'

// Finds the hosts that answer ICMP echo (ping) on the network of each
// interface. Without ports, the probe module is icmp_echoscan.

const os = require('os');
const zmap = require('../');

// The /24 network of each IPv4 address that is not loopback.
const range = Object.values(os.networkInterfaces())
  .flat()
  .filter((a) => a.family === 'IPv4' && !a.internal)
  .map((a) => `${a.address.split('.').slice(0, 3).join('.')}.0/24`);

const opts = {
  range: [...new Set(range)],
  cooldown: 2 // seconds to wait for late answers. zmap's default is 8.
};


zmap.zmap(opts, (err, report) => {
  if (err) throw err;

  console.log(JSON.stringify(report.hosts, null, 2));
});
