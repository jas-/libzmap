#define __STDC_FORMAT_MACROS 1

#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "zmap-1.2.1/lib/xalloc.h"
#include "zmap-1.2.1/lib/logger.h"
#include "zmap-1.2.1/src/fieldset.h"

fieldset_t *obj;
fieldset_t *return_object(void);

static FILE *file = NULL;

static void hex_encode(FILE *f, unsigned char* readbuf, size_t len)
{
	for(size_t i=0; i < len; i++) {
		fprintf(f, "%02x", readbuf[i]);
	}
}

fieldset_t *return_object(void) {
	fieldset_t *cobj = (fieldset_t*) xmalloc(sizeof(fieldset_t*));
	cobj = obj;

	log_error("cobj->len", "%d", cobj->len);
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

  return cobj;
}
