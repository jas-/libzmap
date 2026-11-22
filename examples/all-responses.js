/*!
 * libzmap
 * Copyright(c) 2014 Jason Gerfen <jason.gerfen@gmail.com>
 * License: Apache-2.0
 */

'use strict'

// Every response: also RST, ICMP unreachable and duplicate answers. An empty
// filter turns off zmap's filter "success = 1 && repeat = 0".

const zmap = require('../');
const opts = {
  ports: '22,80',
  range: ['10.0.2.0/24'],
  outputFilter: '',
  dedupMethod: 'none' // keep duplicate answers too
};


zmap.zmap(opts, (err, report) => {
  if (err) throw err;

  const byClass = {};
  for (const r of report.results) byClass[r.classification] = (byClass[r.classification] ?? 0) + 1;
  console.log(JSON.stringify({ byClass, results: report.results }, null, 2));
});
