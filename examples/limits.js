/*!
 * libzmap
 * Copyright(c) 2014 Jason Gerfen <jason.gerfen@gmail.com>
 * License: Apache-2.0
 */

'use strict'

// Limits for a scan: the send rate, the number of targets, the time and the
// number of results. The scan stops at the first limit that it reaches.

const zmap = require('../');
const opts = {
  ports: '80',
  range: ['10.0.0.0/8'],
  rate: 500, // packets each second (or bandwidth: '10M')
  maxTargets: '1%', // a number, or a percentage of the range
  maxRuntime: 30, // seconds of sending
  maxResults: 100, // stop after 100 results
  cooldown: 3 // seconds to wait for answers after the last probe
};


zmap.zmap(opts, (err, report) => {
  if (err) throw err;

  console.log(JSON.stringify({ counts: report.counts, hosts: report.hosts }, null, 2));
});
