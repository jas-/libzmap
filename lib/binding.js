/*!
 * libzmap
 * Copyright(c) 2014 Jason Gerfen <jason.gerfen@gmail.com>
 * License: Apache-2.0
 */
'use strict';

// The compiled zmap module: start(argv, verbosity, onEvents), stop(force), isRunning().
module.exports = require('../build/Release/zmap.node');
