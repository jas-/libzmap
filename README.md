# libzmap
[node.js](http://nodejs.org) bindings for the [zmap](https://zmap.io/) network scanning utility.

# installation
Some libraries are necessary for zmap to compile correctly.
Review the [instructions](https://github.com/jas-/libzmap/wiki/Installation) for your OS.

Setup your LD_LIBRARY_PATH to accomidate the linking

```sh
%> export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:`pwd`/build/Release
```

Now install this module with `npm`

```sh
%> npm install libzmap
```

Example usage

```javascript
var lib = require('../build/Release/zmap.node')
  , opts = {
      "iface": 'eth0',
      "ipaddr": '10.0.2.15-10.0.2.18',
      "mac": '52:54:00:12:34:56',
      "shards": 2,
      "shardTotal": 4,
      "range": '10.0.2.0/24',
      "blacklist": '/path/to/blacklist.conf',
      "whitelist": '/path/to/whitelist.conf'
    };

lib.zmap(opts, function(err, result) {
  console.log(err);
  console.log(result);
});
```
