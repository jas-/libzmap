/*!
 * libzmap
 * Copyright(c) 2014 Jason Gerfen <jason.gerfen@gmail.com>
 * License: Apache-2.0
 */

'use strict'

// The same scan with a promise. zmap finds the interface, the source
// address and the gateway MAC address itself when the options leave them out.

const zmap = require('../');
const opts = {
  range: ['10.0.2.0/24']
};


zmap.scan(opts).done
  .then((report) => console.log(JSON.stringify(report, null, 2)))
  .catch((err) => console.error(err.message));
