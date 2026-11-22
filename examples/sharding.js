/*!
 * libzmap
 * Copyright(c) 2014 Jason Gerfen <jason.gerfen@gmail.com>
 * License: Apache-2.0
 */

'use strict'

// Splits one scan over several processes or machines. Each runs one shard,
// with the same seed, shard total and range. Together the shards probe each
// target one time. Run with the shard number: node examples/sharding.js 0

const zmap = require('../');
const shard = Number(process.argv[2] ?? 0);
const opts = {
  shard, // this shard, from 0
  shards: 4, // total number of shards
  seed: 1234, // the same on each shard
  range: ['10.0.0.0/16'],
  rate: 1000
};


zmap.zmap(opts, (err, report) => {
  if (err) throw err;

  console.log(JSON.stringify({ shard, hosts: report.hosts }, null, 2));
});
