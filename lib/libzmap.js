/*!
 * libzmap
 * Copyright(c) 2014 Jason Gerfen <jason.gerfen@gmail.com>
 * License: Apache-2.0
 */
'use strict';

const binding = require('./binding.js');
const { Scan } = require('./scan.js');

// One scan per process. A second call while a scan runs throws ERR_ZMAP_BUSY.
function scan(options = {}) {
  return new Scan(options);
}

// The callback form of libzmap 0.x: zmap(options, (err, report) => {}).
function zmap(options, callback) {
  if (typeof options === 'function') {
    callback = options;
    options = {};
  }
  if (typeof callback !== 'function') {
    throw new TypeError('zmap(options, callback) needs a callback');
  }
  const running = scan(options);
  running.done.then((report) => callback(null, report), (err) => callback(err, err.report ?? null));
  return running;
}

function isRunning() {
  return binding.isRunning();
}

module.exports = { scan, zmap, isRunning, Scan };
