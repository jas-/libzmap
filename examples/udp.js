/*!
 * libzmap
 * Copyright(c) 2014 Jason Gerfen <jason.gerfen@gmail.com>
 * License: Apache-2.0
 */

'use strict'

// A UDP scan. probeArgs sets the payload of each probe: text, hex or a file.
// A host that answers on the port counts as a success. An ICMP unreachable
// answer is in the results when the filter permits it.

const zmap = require('../');
const opts = {
  probeModule: 'udp',
  ports: '161',
  probeArgs: 'hex:302902010004067075626c6963a01c0204ffffffff020100020100300e300c06082b060102010101000500',
  outputFilter: '', // keep every answer, also the ICMP unreachables
  range: ['10.0.2.0/24']
};


zmap.zmap(opts, (err, report) => {
  if (err) throw err;

  console.log(JSON.stringify(report.results, null, 2));
});
