#ifndef AT_PLATFORM_H
#define AT_PLATFORM_H

#include <stdio.h>

#define AT_PRINT printf

typedef enum {
    AT_STATUS_OK,
    AT_STATUS_FAIL,
    AT_STATUS_TIMEOUT,
} AT_Status;

typedef enum {
    AT_CODE_OK,
    AT_CODE_CONNECT,
    AT_CODE_RING,
    AT_CODE_NO_CARRIER,
    AT_CODE_ERROR,
    AT_CODE_RESERVED,
    AT_CODE_NO_DIALTONE,
    AT_CODE_BUSY,
    AT_CODE_NO_ANSWER,
    // NOTE: Add manufacturer-specific response codes here.
    // NOTE: Leave AT_CODE_size at the end.
    AT_CODE_size
} AT_Code;

AT_Status AT_Tx(char c);
AT_Status AT_Rx(char *c);
AT_Status AT_Send(const char *cmd, ...);

#endif // AT_PLATFORM_H