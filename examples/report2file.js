/*!
 * libzmap
 * Copyright(c) 2014 Jason Gerfen <jason.gerfen@gmail.com>
 * License: Apache-2.0
 */

'use strict'

// Writes the report of one scan to a file, and zmap's metadata and status to
// files of their own.

const fs = require('fs');
const zmap = require('../');
const opts = {
  range: ['10.0.2.0/24'],
  metadataFile: 'metadata.json', // zmap's scan metadata (also in report.metadata)
  statusUpdatesFile: 'status.csv' // one line of progress each second
};


zmap.zmap(opts, (err, report) => {
  if (err) throw err;

  fs.writeFileSync('report.json', JSON.stringify(report, null, 2));
});
