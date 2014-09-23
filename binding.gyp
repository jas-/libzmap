{
	"targets":
	[{
		"target_name": "zmap",
		"type": "shared_library",
    "variables": {
      'cwd': '<!(pwd)',
      'path': 'src/zmap-1.2.1',
      'ld': '<!(export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:<(cwd)/build/Release)',
      'lexer': '<!(flex -o"<(path)/src/lexer.c" --header-file="<(path)/src/lexer.h" "<(path)/src/lexer.l")',
      'parser': '<!(byacc -d -o "<(path)/src/parser.c" "<(path)/src/parser.y")',
      'modules1': "<!(sed -i 's/extern output_module_t module_csv_file;/extern output_module_t module_node_json;/' <(path)/src/output_modules/output_modules.c)",
      'modules2': "<!(sed -i 's/\&module_csv_file,/\&module_node_json,/' <(path)/src/output_modules/output_modules.c)",
    },
    "include_dirs": [
      "<(path)/lib",
      "<(path)/src",
      "<(path)/src/output_modules",
    ],
    "conditions": [
      ['OS=="linux"', {
        "cflags": [
          "-Wall",
          "-Wformat=2",
          "-Wno-format-nonliteral",
          "-pedantic",
          "-fno-strict-aliasing",
          "-Wextra",
          "-Wfloat-equal",
          "-Wundef",
          "-Wwrite-strings",
          "-Wredundant-decls",
          "-Wnested-externs",
          "-Wbad-function-cast",
          "-Winit-self",
          "-Wmissing-noreturn",
          "-Wstack-protector",
          "-std=gnu99",
          "-U_FORTIFY_SOURCE",
          "-D_FORTIFY_SOURCE=2",
          "-fstack-protector-all",
          "-fwrapv",
          "-fPIC",
          "--param ssp-buffer-size=1",
          "-O2",
        ],
        "link_settings": {
          "libraries": [
            "-pthread",
            "-lpcap",
            "-lgmp",
            "-lfl",
            "-lm"
          ]
        },
      }]
    ],
    "sources": [
      "<(path)/lib/blacklist.c",
      "<(path)/lib/constraint.c",
      "<(path)/lib/logger.c",
      "<(path)/lib/pbm.c",
      "<(path)/lib/random.c",
      "<(path)/lib/rijndael-alg-fst.c",
      "<(path)/lib/xalloc.c",
      "<(path)/src/aesrand.c",
      "<(path)/src/cyclic.c",
      "<(path)/src/expression.c",
      "<(path)/src/fieldset.c",
      "<(path)/src/filter.c",
      "<(path)/src/get_gateway.c",
      "<(path)/src/iterator.c",
      "<(path)/src/lexer.c",
      "<(path)/src/monitor.c",
      "<(path)/src/parser.c",
      "<(path)/src/send.c",
      "<(path)/src/shard.c",
      "<(path)/src/state.c",
      "<(path)/src/recv.c",
      "<(path)/src/validate.c",
      "<(path)/src/zopt.c",
      "<(path)/src/zmap.c",
      "<(path)/src/output_modules/output_modules.c",
      "<(path)/src/probe_modules/module_icmp_echo.c",
      "<(path)/src/probe_modules/module_tcp_synscan.c",
      "<(path)/src/probe_modules/module_udp.c",
      "<(path)/src/probe_modules/packet.c",
      "<(path)/src/probe_modules/probe_modules.c",
      "src/module.c",
    ]
  },
  {
		"target_name": "libzmap",
		"type": "loadable_module",
    "variables": {
      'cwd': '<!(pwd)',
      'path': 'src/zmap-1.2.1',
      'ldpath': 'build/Release',
    },
		"include_dirs": [
			"<(path)/lib",
			"<(path)/src",
	  ],
		"dependencies": [
			"zmap",
		],
		"sources": [
			"src/config.cc",
			"src/output.cc",
			"src/async.cc",
			"src/libzmap.cc",
		],
    "libraries":[
      "<(cwd)/<(ldpath)/zmap.so"
    ],
		"conditions": [
      ['OS=="linux"', {
        "cflags": [
          "-fPIC",
          "-O2",
				],
				"ldflags": [
          "-pthread",
        ]
			}]
		],
		"include_dirs" : [
			"<!(node -e \"require('nan')\")"
		],
	}],
}
