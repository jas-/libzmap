{
  "variables": {
    "shim": "<(module_root_dir)/src/shim/libzmap-shim.h",
    "zmap_include": [
      "src",
      "src/gen",
      "src/zmap/lib",
      "src/zmap/src",
      "src/zmap/src/output_modules",
      "<!@(node scripts/deps.js include)"
    ]
  },
  "target_defaults": {
    "xcode_settings": { "MACOSX_DEPLOYMENT_TARGET": "11.0" }
  },
  "targets": [
    {
      # Unmodified upstream zmap with the shim force-included, and the files that adapt it.
      "target_name": "zmap_core",
      "type": "static_library",
      "sources": [
        "<!@(node scripts/sources.js)",
        "src/runner.c",
        "src/blocklist-hooks.c",
        "src/recv-pcap-hooks.c",
        "src/get-gateway-hooks.c",
        "src/monitor-hooks.c",
        "src/output.c"
      ],
      # shim/include first: its assert.h replaces the system one.
      "include_dirs": [ "src/shim/include", "<@(zmap_include)" ],
      "cflags": [ "-fPIC", "-fvisibility=hidden", "-std=gnu11", "-O2", "-w", "-include", "<(shim)" ],
      "xcode_settings": {
        "OTHER_CFLAGS": [ "-std=gnu11", "-O2", "-w", "-include", "<(shim)" ],
        "GCC_SYMBOLS_PRIVATE_EXTERN": "YES",
        "GCC_WARN_INHIBIT_ALL_WARNINGS": "YES"
      }
    },
    {
      "target_name": "zmap",
      "dependencies": [ "zmap_core" ],
      "sources": [ "src/addon.c", "src/hooks.c" ],
      "include_dirs": [ "<@(zmap_include)" ],
      # Hidden symbols stop the host process from replacing zmap functions on Linux.
      "cflags": [ "-fvisibility=hidden", "-std=gnu11", "-O2" ],
      "xcode_settings": {
        "OTHER_CFLAGS": [ "-std=gnu11", "-O2" ],
        "GCC_SYMBOLS_PRIVATE_EXTERN": "YES"
      },
      "libraries": [ "<!@(node scripts/deps.js libs)" ],
      "conditions": [
        [ "OS=='linux'", {
          "libraries": [ "-lm", "-ldl", "-lpthread" ]
        }],
        [ "OS=='mac'", {
          "libraries": [ "-lm" ]
        }]
      ]
    }
  ]
}
