/* Upstream lib/blocklist.c. Its blocklist_init() runs once per process; this one frees the last scan's state first. */
#include "shim/libzmap-shim.h"

#define blocklist_init lz_orig_blocklist_init
#include "blocklist.c"
#undef blocklist_init

static void
lz_free_cidrs(bl_ll_t *list)
{
    bl_cidr_node_t *node, *next;

    if (list == NULL)
        return;
    for (node = list->first; node; node = next) {
        next = node->next;
        free(node);
    }
    free(list);
}

int
blocklist_init(char *allowlist_filename, char *blocklist_filename,
               char **allowlist_entries, size_t allowlist_entries_len,
               char **blocklist_entries, size_t blocklist_entries_len,
               int ignore_invalid_hosts)
{
    if (constraint) {
        constraint_free(constraint);
        constraint = NULL;
    }
    lz_free_cidrs(blocklisted_cidrs);
    lz_free_cidrs(allowlisted_cidrs);
    blocklisted_cidrs = NULL;
    allowlisted_cidrs = NULL;
    return lz_orig_blocklist_init(allowlist_filename, blocklist_filename,
                                  allowlist_entries, allowlist_entries_len,
                                  blocklist_entries, blocklist_entries_len,
                                  ignore_invalid_hosts);
}
