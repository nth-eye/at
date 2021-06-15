#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "at_hal.h"

// NOTE: Add manufacturer-specific response codes at the end.
const char *AT_RESULTS[AT_CODE_size] = { 
    "OK", "CONNECT", "RING", "NO CARRIER", "ERROR", 
    "_", "NO DIALTONE", "BUSY", "NO ANSWER" 
};

AT_Status AT_Tx(char c)
{
    return putc(c, stdout) == EOF ? AT_STATUS_FAIL : AT_STATUS_OK;
}

AT_Status AT_Rx(char *c)
{
    *c = getc(stdin);

    return *c == EOF ? AT_STATUS_FAIL : AT_STATUS_OK;
}

AT_Status AT_Send(const char *cmd, ...)
{
    char buf[256];

    va_list args;
    va_start(args, cmd);

    int len = vsnprintf(buf, sizeof(buf) - 2, cmd, args);
    if (len < 0 || len > (int) sizeof(buf) - 3)
        return AT_STATUS_FAIL;

    va_end(args);

    buf[len++] = '\r';
    buf[len++] = '\n';
    buf[len]   = 0;

    for (int i = 0; i < len; ++i) {
        switch (AT_Tx(buf[i])) {
            case AT_STATUS_OK:      break;
            case AT_STATUS_FAIL:    return AT_STATUS_FAIL;
            case AT_STATUS_TIMEOUT: return AT_STATUS_TIMEOUT;
        }
    }
    return AT_STATUS_OK;
}