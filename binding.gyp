{
	"targets":
	[{
		"target_name": "zmap",
		"type": "static_library",
		"variables": {
			'path': 'src/zmap-1.2.1',
			'lexer': '<!(flex -o "src/zmap-1.2.1/src/lexer.c" --header-file="src/zmap-1.2.1/src/lexer.h" "src/zmap-1.2.1/src/lexer.l")',
			'parser': '<!(byacc -d -o "src/zmap-1.2.1/src/parser.c" "src/zmap-1.2.1/src/parser.y")',
		},
		"include_dirs": [
			"src/zmap-1.2.1/lib",
			"src/zmap-1.2.1/src",
			"src/zmap-1.2.1/src/output_modules",
	  ],
		"conditions": [
			['OS=="linux"', {
				"cflags": [
					"-Wall -Wformat=2 -Wno-format-nonliteral",
					"-pedantic -fno-strict-aliasing",
					"-Wextra",
					"-Wfloat-equal -Wundef -Wwrite-strings -Wredundant-decls",
					"-Wnested-externs -Wbad-function-cast -Winit-self",
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
						"-lm"
	        ]
				},
			}]
		],
		"sources": [
			"src/zmap-1.2.1/lib/blacklist.c",
			"src/zmap-1.2.1/lib/constraint.c",
			"src/zmap-1.2.1/lib/logger.c",
			"src/zmap-1.2.1/lib/pbm.c",
			"src/zmap-1.2.1/lib/random.c",
			"src/zmap-1.2.1/lib/rijndael-alg-fst.c",
			"src/zmap-1.2.1/lib/xalloc.c",
			"src/zmap-1.2.1/src/aesrand.c",
			"src/zmap-1.2.1/src/cyclic.c",
			"src/zmap-1.2.1/src/expression.c",
			"src/zmap-1.2.1/src/fieldset.c",
			"src/zmap-1.2.1/src/filter.c",
			"src/zmap-1.2.1/src/get_gateway.c",
			"src/zmap-1.2.1/src/iterator.c",
			"src/zmap-1.2.1/src/lexer.c",
			"src/zmap-1.2.1/src/monitor.c",
			"src/zmap-1.2.1/src/send.c",
			"src/zmap-1.2.1/src/shard.c",
			"src/zmap-1.2.1/src/state.c",
			"src/zmap-1.2.1/src/recv.c",
			"src/zmap-1.2.1/src/validate.c",
			"src/zmap-1.2.1/src/zopt.c",
			"src/zmap-1.2.1/src/zmap.c",
			"src/zmap-1.2.1/src/output_modules/module_csv.c",
			"src/zmap-1.2.1/src/output_modules/output_modules.c",
			"src/zmap-1.2.1/src/probe_modules/module_icmp_echo.c",
			"src/zmap-1.2.1/src/probe_modules/module_tcp_synscan.c",
			"src/zmap-1.2.1/src/probe_modules/module_udp.c",
			"src/zmap-1.2.1/src/probe_modules/packet.c",
			"src/zmap-1.2.1/src/probe_modules/probe_modules.c"
		]
	},
	{
		"target_name": "libzmap",
		"type": "executable",
		"dependencies": [
			"zmap",
		],
		"include_dirs" : [
			"<!(node -e \"require('nan')\")",
			"src/zmap-1.2.1/lib",
			"src/zmap-1.2.1/src",
			"src/zmap-1.2.1/src/output_modules",
		],
		"sources": [
			"src/libzmap.c",
		],
		"conditions": [
			['OS=="linux"', {
				"cflags": [
					"-Wall -Wformat=2 -Wno-format-nonliteral",
					"-pedantic -fno-strict-aliasing",
					"-Wextra",
					"-Wfloat-equal -Wundef -Wwrite-strings -Wredundant-decls",
					"-Wnested-externs -Wbad-function-cast -Winit-self",
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
						"-lm"
	        ]
				},
			}]
		],
	}],
}
