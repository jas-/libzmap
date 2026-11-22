/*!
 * libzmap
 * Copyright(c) 2014 Jason Gerfen <jason.gerfen@gmail.com>
 * License: Apache-2.0
 */
'use strict';

// for await (const result of scan): yields each result as it arrives. The loop ends when the
// scan ends, and throws if the scan fails. A break, return or throw stops the scan at once.
// zmap does not wait for a slow loop, so results that are not read yet are kept in memory.
function iterate(scan) {
  const queue = [];
  let head = 0;
  let waiting = null;
  let finished = false;
  let failure = null;

  const onResult = (result) => {
    if (waiting) {
      const { resolve } = waiting;
      waiting = null;
      resolve({ value: result, done: false });
    } else {
      queue.push(result);
    }
  };
  scan.on('result', onResult);

  // The loop gets the scan error, so scan.done does not also report it as unhandled.
  const settle = (err) => {
    if (finished) return;
    finished = true;
    failure = err;
    scan.off('result', onResult);
    if (waiting) {
      const { resolve, reject } = waiting;
      waiting = null;
      failure = null;
      if (err) reject(err);
      else resolve({ value: undefined, done: true });
    }
  };
  scan.done.then(() => settle(null), settle);

  return {
    next() {
      if (head < queue.length) {
        const value = queue[head];
        queue[head++] = undefined;
        if (head > 1024 && head * 2 > queue.length) {
          queue.splice(0, head);
          head = 0;
        }
        return Promise.resolve({ value, done: false });
      }
      if (finished) {
        const err = failure;
        failure = null;
        return err ? Promise.reject(err) : Promise.resolve({ value: undefined, done: true });
      }
      return new Promise((resolve, reject) => { waiting = { resolve, reject }; });
    },
    async return() {
      const wasRunning = !finished;
      finished = true;
      failure = null;
      queue.length = 0;
      head = 0;
      scan.off('result', onResult);
      if (wasRunning) await scan.stop({ force: true });
      return { value: undefined, done: true };
    },
    [Symbol.asyncIterator]() { return this; },
  };
}

module.exports = { iterate };
