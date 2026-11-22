/*!
 * libzmap
 * Copyright(c) 2014 Jason Gerfen <jason.gerfen@gmail.com>
 * License: Apache-2.0
 */
'use strict';

const { EventEmitter } = require('node:events');
const binding = require('./binding.js');
const { toArgs, toVerbosity } = require('./options.js');
const { parseJson } = require('./json.js');
const { Report } = require('./report.js');
const { iterate } = require('./iterator.js');

// One running scan. Events: 'result', 'progress', 'log', 'done' and 'error'.
class Scan extends EventEmitter {
  constructor(options = {}) {
    const args = toArgs(options);
    const verbosity = toVerbosity(options.verbose);
    super();
    this.running = true;
    this._builder = new Report({ collect: options.collect !== false });
    // The report so far. scan.done gives it complete.
    this.report = this._builder.json;
    this.done = new Promise((resolve, reject) => {
      this._resolve = resolve;
      this._reject = reject;
    });

    binding.start(args, verbosity, (events) => this._dispatch(events));
  }

  _dispatch(events) {
    const thrown = [];
    for (const { type, ...data } of events) {
      let payload = data;
      if (type === 'result') {
        payload = parseJson(data.json) ?? {};
        this._builder.add(payload);
      }

      // A throwing listener must not drop the rest of the batch, or the scan never settles.
      try {
        if (type === 'done') this._finish(data);
        else this.emit(type, payload);
      } catch (err) {
        thrown.push(err);
      }
    }
    // Node-API hides exceptions thrown in this callback, so rethrow them outside it.
    for (const err of thrown) process.nextTick(() => { throw err; });
  }

  _finish({ error, exitCode, metadata }) {
    this.running = false;
    const report = this._builder.finish(parseJson(metadata));

    if (error) {
      const err = new Error(error);
      err.code = 'ERR_ZMAP_SCAN';
      err.exitCode = exitCode;
      err.log = error.split('\n');
      err.report = report;
      // No 'error' listener: the rejection is unhandled unless the caller uses scan.done.
      const listened = this.listenerCount('error') > 0;
      if (listened) this.done.catch(() => {});
      this._reject(err);
      if (listened) this.emit('error', err);
      return;
    }
    this._resolve(report);
    this.emit('done', report);
  }

  // First call: stop sending, then wait the cooldown for late responses. force or a second call: stop now.
  stop({ force = false } = {}) {
    if (this.running) binding.stop(Boolean(force));
    return this.done.catch(() => {});
  }

  // for await (const result of scan): each result as it arrives.
  [Symbol.asyncIterator]() {
    return iterate(this);
  }
}

module.exports = { Scan };
