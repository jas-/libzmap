/*!
 * libzmap
 * Copyright(c) 2014 Jason Gerfen <jason.gerfen@gmail.com>
 * License: Apache-2.0
 */

'use strict'

// A TCP SYN scan of some ports. A host that answers with SYN-ACK has the
// port open. report.hosts lists the open ports of each host.

const zmap = require('../');
const opts = {
  probeModule: 'tcp_synscan',
  ports: '22,80,443,8000-8100',
  range: ['10.0.2.0/24']
};


zmap.zmap(opts, (err, report) => {
  if (err) throw err;

  console.log(JSON.stringify(report.hosts, null, 2));
});
