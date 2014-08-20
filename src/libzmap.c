#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sched.h>
#include <errno.h>
#include <pwd.h>
#include <time.h>

#include <pcap/pcap.h>

#include <pthread.h>

#include "zmap-1.2.1/lib/includes.h"
#include "zmap-1.2.1/lib/blacklist.h"
#include "zmap-1.2.1/lib/logger.h"
#include "zmap-1.2.1/lib/random.h"
#include "zmap-1.2.1/lib/xalloc.h"


#if defined(__APPLE__)
#include <mach/thread_act.h>
#endif

#ifdef JSON
#include <json.h>
#endif

#include "zmap-1.2.1/src/aesrand.h"
#include "zmap-1.2.1/src/zopt.h"
#include "zmap-1.2.1/src/send.h"
#include "zmap-1.2.1/src/recv.h"
#include "zmap-1.2.1/src/state.h"
#include "zmap-1.2.1/src/monitor.h"
#include "zmap-1.2.1/src/get_gateway.h"
#include "zmap-1.2.1/src/filter.h"

#include "zmap-1.2.1/src/output_modules/output_modules.h"
#include "zmap-1.2.1/src/probe_modules/probe_modules.h"

#define MAC_ADDR_LEN 6

int main(int argc, char *argv[])
{
	struct gengetopt_args_info args;
	struct cmdline_parser_params *params;
	params = cmdline_parser_params_create();
	params->initialize = 1;
	params->override = 0;
	params->check_required = 0;

	if (cmdline_parser_ext(argc, argv, &args, params) != 0) {
		exit(EXIT_SUCCESS);
	}
	if (args.config_given) {
		params->initialize = 0;
		params->override = 0;
		if (cmdline_parser_config_file(args.config_arg, &args, params) != 0) {
			exit(EXIT_FAILURE);
		}
	}

	return 0;
}
