{
	"make_global_settings": [
    ["CXX","/usr/bin/cmake"],
    ["LINK","/usr/bin/cmake"],
  ],
	"targets": [
	{
		"target_name": "zmap",
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
