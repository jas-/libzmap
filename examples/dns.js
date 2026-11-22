/*!
 * libzmap
 * Copyright(c) 2014 Jason Gerfen <jason.gerfen@gmail.com>
 * License: Apache-2.0
 */

'use strict'

// Finds open DNS resolvers. The dns probe module sends one question to UDP
// port 53 of each host. probeArgs is "type,name".

const zmap = require('../');
const opts = {
  probeModule: 'dns',
  ports: '53',
  probeArgs: 'A,example.com',
  range: ['10.0.2.0/24']
};


zmap.zmap(opts, (err, report) => {
  if (err) throw err;

  console.log(JSON.stringify(report.results, null, 2));
});
