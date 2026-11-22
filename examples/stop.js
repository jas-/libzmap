/*!
 * libzmap
 * Copyright(c) 2014 Jason Gerfen <jason.gerfen@gmail.com>
 * License: Apache-2.0
 */

'use strict'

// Stops a scan before it ends. The first stop() stops the probes and waits
// the cooldown for late answers. stop({ force: true }) stops at once.
// The report has the results until the stop.

const zmap = require('../');
const scan = zmap.scan({
  range: ['10.0.0.0/8'],
  rate: 1000,
  cooldown: 2
});

setTimeout(() => scan.stop(), 10 * 1000);

scan.done
  .then((report) => console.log(JSON.stringify({
    durationMs: report.durationMs,
    sent: report.metadata.packets_sent,
    hosts: report.hosts
  }, null, 2)))
  .catch((err) => console.error(err.message));
