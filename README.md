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
