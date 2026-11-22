/*!
 * libzmap
 * Copyright(c) 2014 Jason Gerfen <jason.gerfen@gmail.com>
 * License: Apache-2.0
 */

'use strict'

// The events of a scan: each result, the progress each second, the log
// lines of zmap, and the report at the end. Each event is printed as JSON.

const zmap = require('../');
const scan = zmap.scan({
  range: ['10.0.2.0/24'],
  verbose: 3 // log level: 0 fatal, 1 error, 2 warn, 3 info, 4 debug, 5 trace
});

scan.on('result', (result) => console.log(JSON.stringify({ result })));
scan.on('progress', (progress) => console.log(JSON.stringify({ progress })));
scan.on('log', (log) => console.log(JSON.stringify({ log })));
scan.on('done', (report) => console.log(JSON.stringify({ counts: report.counts })));
scan.on('error', (err) => console.error(JSON.stringify({ error: err.message })));
