#ifndef AT_H
#define AT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

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

typedef enum {
    AT_ST_IDLE,
    AT_ST_START,
    AT_ST_RSP,
    AT_ST_ARGS_START,
    AT_ST_ARGS,
    AT_ST_CODE,
    AT_ST_TEXT,
    AT_ST_END,
    // NOTE: Leave AT_ST_num at the end.
    AT_ST_num,
} AT_State;

typedef enum {
    AT_EV_CR,
    AT_EV_LF,
    AT_EV_MARK,
    AT_EV_SEMI,
    AT_EV_SPACE,
    AT_EV_DIGIT,
    AT_EV_LETTER,
    AT_EV_OTHER,
    // NOTE: Leave AT_EV_num at the end.
    AT_EV_num,
} AT_Event;

typedef struct AT AT;
typedef void (*AT_Callback)(AT*);   // Callback typedef
typedef AT_State (*AT_Trans)(AT*);  // Transition function typedef

typedef struct {
    const char  *cmd;
    AT_Callback cb;
    AT_Callback *res_cbks;
} AT_Cmd;

typedef struct {
    AT_Cmd  *cbks;
    char    *buf;
    size_t  cbks_size;
    size_t  buf_size;
} AT_Cfg;

struct AT {
    AT_Status   status;
    AT_State    st;
    AT_State    parse_st;
    AT_Event    ev;
    AT_Event    prev_ev;
    AT_Code     code;
    AT_Cmd      *cbks;
    char        *buf;
    char        *args;
    char        c;
    size_t      cbks_size;
    size_t      buf_size;
    size_t      pos;
    size_t      cb_idx;
};

AT_Status at_tx(char c);
AT_Status at_rx(char *c);
AT_Status at_send(const char *cmd, ...);

void at_init(AT *at, const AT_Cfg *cfg);
void at_process(AT *at);
void at_clear(AT *at);

#ifdef __cplusplus
}
#endif

#endif // AT_H