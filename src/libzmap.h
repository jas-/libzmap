struct options {

	/* Common options */
	int port;
	char* output_file;
	char* blacklist_file;

	/* Scan options */
	int max_targets;
	int max_results;
	int max_runtime;
	int rate;
	int bandwidth;
	char* bps;
	int cooldown;
	int seed;
	int shards;
	int shard;
	int sender_threads;
	int probes;

	/* Network options */
	char* source_port;
	char* source_ip;
	char* gateway_ip;
	char* interface;

	/* Probe options */
	char* probe_module;
	char* probe_args;
	char* probe_fields;
	char* probe_filter;

	/* Output options */
	char* output_module;
	char* output_args;
	char* output_fields;
	char* output_filter;

	/* Additional options */
	char* config_file;
	int quiet;
	int summary;
	int verbosity;

} opts;
