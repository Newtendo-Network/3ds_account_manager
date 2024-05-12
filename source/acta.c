#include <string.h>
#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/synchronization.h>
#include <3ds/ipc.h>
#include <3ds/result.h>
#include <3ds/srv.h>

#include "acta.h"

static Handle actaHandle;
static int actaRefCount;

Result actaInit(void)
{
    Result ret = 0;

    if (AtomicPostIncrement(&actaRefCount))
        return 0;

    ret = srvGetServiceHandle(&actaHandle, "act:a");
    if (R_FAILED(ret))
        AtomicDecrement(&actaRefCount);

    return ret;
}

void actaExit(void)
{
    if (AtomicDecrement(&actaRefCount))
        return;
    svcCloseHandle(actaHandle);
}

Handle *actaGetSessionHandle(void)
{
    return &actaHandle;
}
