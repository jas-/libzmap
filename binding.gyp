{
	"targets":
	[{
		"target_name": "zmap",
		"type": "static_library",
		"include_dirs": [
			"src/zmap-1.2.1/lib",
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
				"ldflags": [
          "-pthread",
        ]
			}]
		],
		"sources": [
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
		]
	},
	{
		"target_name": "libzmap",
		"type": "executable",
		"dependencies": [
			"zmap",
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
