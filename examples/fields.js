/*!
 * libzmap
 * Copyright(c) 2014 Jason Gerfen <jason.gerfen@gmail.com>
 * License: Apache-2.0
 */

'use strict'

// Selects the fields of each result and filters the results. By default each
// result has every field, and the filter keeps the first success of each host.

const zmap = require('../');
const opts = {
  ports: '22,80,443',
  range: ['10.0.2.0/24'],
  outputFields: ['saddr', 'sport', 'classification', 'ttl', 'timestamp_str'],
  outputFilter: 'success = 1 && repeat = 0 && ttl > 32'
};


zmap.zmap(opts, (err, report) => {
  if (err) throw err;

  console.log(JSON.stringify(report.results, null, 2));
});
