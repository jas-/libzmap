#define __STDC_FORMAT_MACROS 1

#include <assert.h>
#include <inttypes.h>
#include "zmap-1.2.1/src/fieldset.h"
#include "zmap-1.2.1/lib/logger.h"
#include "zmap-1.2.1/src/output_modules/output_modules.h"

#include "module.h"

int node_json_init(struct state_conf *conf, char **fields, int fieldlens)
{
	assert(conf);

	if (fieldlens > 1 && file) {
		for (int i=0; i < fieldlens; i++) {
			if (i) {
				fprintf(file, ", ");
			}
			fprintf(file, "%s", fields[i]);
		}
    fprintf(file, "\n");
	}

	return EXIT_SUCCESS;
}

int node_json_close(__attribute__((unused)) struct state_conf* c,
	__attribute__((unused)) struct state_send* s,
	__attribute__((unused)) struct state_recv* r)
{
	return EXIT_SUCCESS;
}

int node_json_process(fieldset_t *fs)
{
	obj = fs;

	log_error("fs->len", "%d", fs->len);
	for (int i=0; i < fs->len; i++) {
		field_t *f = &(fs->fields[i]);
		if (i) {
			fprintf(file, ",");
		}
		if (f->type == FS_STRING) {
			if (strchr((char*) f->value.ptr, ',')) {
				fprintf(file, "\"%s\"", (char*) f->value.ptr);
			} else {
				fprintf(file, "%s", (char*) f->value.ptr);
			}
		} else if (f->type == FS_UINT64) {
			fprintf(file, "%" PRIu64, (uint64_t) f->value.num);
		} else if (f->type == FS_BINARY) {
			hex_encode(file, (unsigned char*) f->value.ptr, f->len);
		} else if (f->type == FS_NULL) {
			// do nothing
		}
	}

	log_error("obj->len", "%d", fs->len);
	for (int i=0; i < obj->len; i++) {
		field_t *f = &(obj->fields[i]);
		if (i) {
			fprintf(file, ",");
		}
		if (f->type == FS_STRING) {
			if (strchr((char*) f->value.ptr, ',')) {
				fprintf(file, "\"%s\"", (char*) f->value.ptr);
			} else {
				fprintf(file, "%s", (char*) f->value.ptr);
			}
		} else if (f->type == FS_UINT64) {
			fprintf(file, "%" PRIu64, (uint64_t) f->value.num);
		} else if (f->type == FS_BINARY) {
			hex_encode(file, (unsigned char*) f->value.ptr, f->len);
		} else if (f->type == FS_NULL) {
			// do nothing
		}
	}

	return EXIT_SUCCESS;
}

output_module_t module_node_json = {
  .name = "node-json",
  .init = &node_json_init,
  .start = NULL,
  .update = NULL,
  .update_interval = 0,
  .close = &node_json_close,
  .process_ip = &node_json_process,
  .helptext = NULL
};
