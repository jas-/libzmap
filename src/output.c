/* The "node" output module: each result becomes one JSON event. Replaces upstream output_modules.c. */
#include "shim/libzmap-shim.h"
#include "libzmap.h"

#include <stdlib.h>
#include <string.h>

#include <json.h>

#include "output_modules.h"
#include "summary.h"

/* Upstream module_json.c. */
json_object *fs_to_jsonobj(fieldset_t *fs);

extern output_module_t module_csv_file;
extern output_module_t module_json_file;

/* Scan metadata from the last scan, as JSON (summary.c). runner.c takes it. */
char *volatile lz_scan_meta = NULL;

static int
node_init(struct state_conf *conf, const char **fields, int fieldlens)
{
    (void)conf;
    (void)fields;
    (void)fieldlens;
    return EXIT_SUCCESS;
}

static int
node_process(fieldset_t *fs)
{
    struct lz_event ev;
    json_object *record = fs_to_jsonobj(fs);
    const char *text;

    if (record == NULL)
        return EXIT_SUCCESS;
    text = json_object_to_json_string_ext(record, JSON_C_TO_STRING_PLAIN);
    memset(&ev, 0, sizeof(ev));
    ev.type = LZ_EV_RESULT;
    ev.text = text ? strdup(text) : NULL;
    ev.text_len = ev.text ? strlen(ev.text) : 0;
    json_object_put(record);
    if (ev.text)
        lz_push(lz_current, &ev);
    return EXIT_SUCCESS;
}

/* Called after the scan threads end, while the settings are still valid. */
static int
node_close(struct state_conf *conf, struct state_send *send,
           struct state_recv *recv)
{
    char *buffer = NULL;
    size_t size = 0;
    FILE *mem;
    (void)conf;
    (void)send;
    (void)recv;

    mem = open_memstream(&buffer, &size);
    if (mem == NULL)
        return EXIT_SUCCESS;
    json_metadata(mem);
    fclose(mem);
    free(lz_scan_meta);
    lz_scan_meta = buffer;
    return EXIT_SUCCESS;
}

output_module_t module_node = {
    .name = "node",
    .supports_dynamic_output = DYNAMIC_SUPPORT,
    .update_interval = 0,
    .init = &node_init,
    .start = NULL,
    .update = NULL,
    .close = &node_close,
    .process_ip = &node_process,
    .helptext = "Sends each result to Node.js as one JSON object.",
};

static output_module_t *output_modules[] = {
    &module_node, &module_csv_file, &module_json_file,
};

output_module_t *
get_output_module_by_name(const char *name)
{
    size_t i;

    for (i = 0; i < sizeof(output_modules) / sizeof(output_modules[0]); i++) {
        if (!strcmp(output_modules[i]->name, name))
            return output_modules[i];
    }
    return NULL;
}

void
print_output_modules(void)
{
    size_t i;

    for (i = 0; i < sizeof(output_modules) / sizeof(output_modules[0]); i++)
        printf("%s\n", output_modules[i]->name);
}
