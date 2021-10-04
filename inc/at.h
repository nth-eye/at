#ifndef AT_H
#define AT_H

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    AT_RSP_OK,
    AT_RSP_CONNECT,
    AT_RSP_RING,
    AT_RSP_NO_CARRIER,
    AT_RSP_ERROR,
    AT_RSP_RESERVED,
    AT_RSP_NO_DIALTONE,
    AT_RSP_BUSY,
    AT_RSP_NO_ANSWER,
    AT_RSP_RDY,
    // NOTE: Add manufacturer-specific responses here and to AT_RESPONSES in at.c.
    // NOTE: Leave AT_RSP_num at the end.
    AT_RSP_num
} AT_Rsp;

typedef enum {
    AT_ST_IDLE,
    AT_ST_START,
    AT_ST_URC,
    AT_ST_PARAM_START,
    AT_ST_PARAM,
    AT_ST_TXT,
    AT_ST_END,
    // NOTE: Leave AT_ST_size at the end.
    AT_ST_num,
} AT_State;

typedef enum {
    AT_EV_CR,
    AT_EV_LF,
    AT_EV_MARK,
    AT_EV_SEMI,
    AT_EV_SPACE,
    AT_EV_OTHER,
    // NOTE: Leave AT_EV_size at the end.
    AT_EV_num,
} AT_Event;

typedef struct AT AT;
typedef void (*AT_Callback)(AT*);   // Callback typedef
typedef AT_State (*AT_Trans)(AT*);  // Transition function typedef

typedef struct {
    const char  *cmd;
    AT_Callback cb;
} AT_Cmd;

typedef struct {
    AT_Cmd  *cbks;
    char    *buf;
    size_t  cbks_size;
    size_t  buf_size;
} AT_Cfg;

struct AT {
    AT_Cfg      cfg;
    AT_State    st;
    AT_Event    ev;
    AT_Event    prev_ev;
    AT_Rsp      rsp;
    size_t      pos;
    char        c;
    char        *param;
};

bool at_send(const char *cmd, ...);
void at_init(AT *at, const AT_Cfg *cfg);
void at_clear(AT *at);
void at_process(AT *at, char c);

#ifdef __cplusplus
}
#endif

#endif