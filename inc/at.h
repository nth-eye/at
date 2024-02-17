#ifndef AT_H
#define AT_H

#include <string_view>

namespace at {

enum answer {
    ok,
    connect,
    ring,
    no_carrier,
    error,
    no_dialtone,
    busy,
    no_answer,
    rdy,
    invalid,
};

struct result {
    std::string_view raw;
    std::string_view arg;
    answer rsp;
    bool found;
};

struct fsm {
    fsm() = delete;
    fsm(char* buf, size_t len) : buf{buf}, max{len} {}
public:
    auto get_result() const { return result{{buf, pos}, arg, rsp, found}; }
    auto get_raw() const    { return std::string_view{buf, pos}; }
    auto get_line() const   { return txt; }
    auto get_args() const   { return arg; }
    auto response() const   { return rsp; }
    bool acquired() const   { return found; }
    bool full() const       { return pos >= max; }
    void setup(std::string_view target)   
    { 
        trg = target; 
    }
    void clear()
    {
        pos = 0;
        st  = st_idle;
        arg = {};
        reset();
    }
    void reset()
    {
        rsp     = invalid;
        txt     = {};
        found   = false;
        is_arg  = false;
    }
    void process(char c)
    {
        static constexpr state (fsm::*table[st_][ev_])() = 
        {   // ev_cr,           ev_lf,              ev_mark,            ev_semi,            ev_space,           ev_other
            { &fsm::on_new_cr,  &fsm::err,          &fsm::err,          &fsm::err,          &fsm::err,          &fsm::err         }, // st_idle
            { &fsm::on_new_cr,  &fsm::on_new_lf,    &fsm::err,          &fsm::err,          &fsm::err,          &fsm::err         }, // st_new_cr
            { &fsm::on_new_cr,  &fsm::err,          &fsm::on_new_urc,   &fsm::on_new_txt,   &fsm::on_new_txt,   &fsm::on_new_txt  }, // st_new_lf
            { &fsm::err,        &fsm::err,          &fsm::on_arg,       &fsm::on_arg,       &fsm::on_space,     &fsm::on_arg      }, // st_arg_start
            { &fsm::on_end_cr,  &fsm::err,          &fsm::on_arg,       &fsm::on_arg,       &fsm::on_arg,       &fsm::on_arg      }, // st_arg
            { &fsm::on_end_cr,  &fsm::err,          &fsm::on_urc,       &fsm::on_new_arg,   &fsm::on_urc,       &fsm::on_urc      }, // st_urc
            { &fsm::on_end_cr,  &fsm::on_txt,       &fsm::on_txt,       &fsm::on_txt,       &fsm::on_txt,       &fsm::on_txt      }, // st_txt
            { &fsm::on_end_cr,  &fsm::on_end_lf,    &fsm::on_txt,       &fsm::on_txt,       &fsm::on_txt,       &fsm::on_txt      }, // st_end
        };
        if (!full()) {
            st = (this->*table[st][next_ev(c)])();
            buf[pos++] = c;
        }
    }
protected:
    enum state {
        st_idle,
        st_new_cr,
        st_new_lf,
        st_arg_start,
        st_arg,
        st_urc,
        st_txt,
        st_end,
        st_,
    };
    enum event {
        ev_cr,
        ev_lf,
        ev_mark,
        ev_semi,
        ev_space,
        ev_other,
        ev_,
    };
    static constexpr event next_ev(char c)
    {
        switch (c) {
            case '\r':  return ev_cr;
            case '\n':  return ev_lf;
            case '+':  
            case '#': 
            case '&': 
            case '$':
            case '%':   return ev_mark;
            case ':':   return ev_semi;
            case ' ':   return ev_space;
        }
        return ev_other;
    }
    auto ptr() const
    { 
        return buf + pos; 
    }
    size_t diff(const char* p) const    
    { 
        return ptr() - p; 
    }
    state handle_arg()
    {
        found = trg.empty() || trg == txt;
        return st_idle;
    }
    state handle_txt()
    {
        static constexpr std::string_view responses[invalid] = { 
            "OK", "CONNECT", "RING", "NO CARRIER", "ERROR", 
            "NO DIALTONE", "BUSY", "NO ANSWER", "RDY",
        };
        for (int i = 0; i < invalid; ++i) {
            if (txt == responses[i]) {
                rsp = answer(i);
                break;
            }
        }
        return handle_arg();
    }
    state err()
    {
        reset();
        return st_idle;
    }
    state on_end_lf()   { return is_arg ? handle_arg() : handle_txt(); }
    state on_new_lf()   { return st_new_lf; }
    state on_arg()      { return st_arg; }
    state on_urc()      { return st_urc; }
    state on_txt()      { return st_txt; }
    state on_space()
    {
        is_arg = true;
        arg = {ptr() + 1, 0};
        return st_arg;
    }
    state on_new_arg()
    {
        txt = {txt.begin(), diff(txt.begin())};
        return st_arg_start;
    }
    state on_new_urc()
    {
        txt = {ptr(), 0};
        return st_urc;
    }
    state on_new_txt()
    {
        txt = {ptr(), 0};
        return st_txt;
    }
    state on_new_cr()
    {
        reset();
        return st_new_cr;
    }
    state on_end_cr()
    {
        if (is_arg)
            arg = {arg.begin(), diff(arg.begin())};
        else
            txt = {txt.begin(), diff(txt.begin())};
        return st_end;
    }
protected:
    std::string_view trg; // Target string to match
    std::string_view arg; // Parsed string with arguments (parameters)
    std::string_view txt; // Parsed string with last text line
    char* buf;
    size_t max;
    size_t pos  = 0;
    state st    = st_idle;
    answer rsp  = invalid;
    bool found  = false;
    bool is_arg = false;
};

template<size_t N>
struct parser : fsm {
    parser() : fsm{buf, N} {}
private:
    char buf[N];
};

}

#endif
