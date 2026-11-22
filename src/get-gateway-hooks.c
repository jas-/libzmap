/*
 * Upstream get_gateway.c. On Linux, get_default_gw() and get_hw_addr()
 * (get_gateway-linux.h) never close their netlink socket. This records each
 * one, and the scan closes it when it ends.
 *
 * Upstream closes its other sockets in this file, so only netlink sockets are
 * recorded. send_nl_req() closes one only on an integer overflow, which the
 * fixed sizes of its callers rule out.
 */
#include "shim/libzmap-shim.h"
#include "libzmap.h"

#include <sys/socket.h>

static int
lz_gateway_socket(int domain, int type, int protocol)
{
    int fd = socket(domain, type, protocol);

#ifdef PF_NETLINK
    if (domain == PF_NETLINK)
        lz_record_fd(fd);
#endif
    return fd;
}

#define socket lz_gateway_socket
#include "get_gateway.c"
#undef socket
