/*!
 * libzmap
 * Copyright(c) 2014 Jason Gerfen <jason.gerfen@gmail.com>
 * License: Apache-2.0
 */

'use strict'

// Prints each result as one JSON line when it arrives, instead of one report
// at the end. Ctrl+C stops the scan at once.

const zmap = require('../');
const scan = zmap.scan({
  ports: '80,443',
  range: ['10.0.0.0/16'],
  rate: 2000,
  collect: false // keep the results out of the final report
});

process.once('SIGINT', () => scan.stop({ force: true }));

(async () => {
  for await (const result of scan) {
    console.log(JSON.stringify(result));
  }
  const report = await scan.done;
  console.log(JSON.stringify({ counts: report.counts, durationMs: report.durationMs }));
})().catch((err) => {
  console.error(err.message);
  process.exitCode = 1;
});
