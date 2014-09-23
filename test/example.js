var lib = require('../build/Release/zmap.node')
  , opts = {
//      iface: 'eth0',
//      ipaddr: '10.0.2.15-10.0.2.18',
      mac: '52:54:00:12:34:56',
//      bandwidth: '256M',
//      shards: 2,
//      shardtotal: 4,
      threads: 10,
//      range: '155.97.0.0/17',
//      probemodule: 'tcp_synscan', //tcp_synscan, udp, icmp_echoscan (default)
//      blacklist: '/home/jas/projects/mine/libzmap/src/zmap-1.2.1/conf/blacklist.conf',
      whitelist: '/home/jas/projects/mine/libzmap/test/whitelist.conf'
    };

lib.zmap(opts, function(err, result) {
  console.log(err);
  console.log(result);
});
