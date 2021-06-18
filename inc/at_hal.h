#ifndef AT_HAL_H
#define AT_HAL_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

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
    // NOTE: Leave AT_CODE_num at the end.
    AT_CODE_num
} AT_Code;

AT_Status at_tx(char c);
AT_Status at_rx(char *c);
AT_Status at_send(const char *cmd, ...);

#ifdef __cplusplus
}
#endif

#endif // AT_HAL_H