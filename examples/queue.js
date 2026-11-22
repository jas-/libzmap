/*!
 * libzmap
 * Copyright(c) 2014 Jason Gerfen <jason.gerfen@gmail.com>
 * License: Apache-2.0
 */

'use strict'

// A process runs one scan at a time, because zmap keeps global state. This
// example runs a list of scans one after the other and prints one JSON object.

const zmap = require('../');
const jobs = [
  { name: 'ping', range: ['10.0.2.0/24'] },
  { name: 'web', range: ['10.0.2.0/24'], ports: '80,443' },
  { name: 'ssh', range: ['10.0.2.0/24'], ports: '22' }
];

(async () => {
  const out = {};
  for (const { name, ...opts } of jobs) {
    try {
      out[name] = (await zmap.scan({ ...opts, cooldown: 2 }).done).hosts;
    } catch (err) {
      out[name] = { error: err.message };
    }
  }
  console.log(JSON.stringify(out, null, 2));
})();
