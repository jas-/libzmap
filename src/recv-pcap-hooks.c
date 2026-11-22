/* Upstream recv-pcap.c. lz_recv_close() closes the capture handle that a failed scan leaves open. */
#include "shim/libzmap-shim.h"
#include "libzmap.h"

#include "recv-pcap.c"

void
lz_recv_close(void)
{
    if (pc != NULL) {
        pcap_close(pc);
        pc = NULL;
    }
}
