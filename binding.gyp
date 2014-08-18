{
	"targets": [
	{
		"target_name": "zmap",
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
			"src/zmap-1.2.1/src/monitor.c",
			"src/zmap-1.2.1/src/resv.c",
			"src/zmap-1.2.1/src/send.c",
			"src/zmap-1.2.1/src/shard.c",
			"src/zmap-1.2.1/src/state.c",
			"src/zmap-1.2.1/src/summary.c",
			"src/zmap-1.2.1/src/validate.c",
			"src/zmap-1.2.1/src/zmap.c"
		]
	}],
	"include_dirs": [
		"src/zmap-1.2.1/lib",
  ]
}
