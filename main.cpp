#include <cstdio>
#include <cctype>
#include <cstdint>
#include <cstddef>
#include "at.h"

static inline void log_hex(std::string_view str)
{
    if (str.empty())
        return;
    auto bin_to_char = [](uint8_t bin) {
        return "0123456789abcdef"[bin & 0xf];
    };
    auto p = str.data();

    for (size_t i = 0; i < str.size(); ++i) {

        if (!(i & 15)) {
            putchar('|');
            putchar(' ');
        }
        putchar(bin_to_char(p[i] >> 4));
        putchar(bin_to_char(p[i] & 0xF));
        putchar(' ');
        
        if ((i & 7) == 7)
            putchar(' ');

        if ((i & 15) == 15) {
            putchar('|');
            for (int j = 15; j >= 0; --j) {
                char c = p[i - j];
                putchar(isprint(c) ? c : '.');
            }
            putchar('|');
            putchar('\n');
        }
    }
    int rem = str.size() - ((str.size() >> 4) << 4);
    if (rem) {
        for (int j = (16 - rem) * 3 + ((~rem & 8) >> 3); j >= 0; --j)
            putchar(' ');
        putchar('|');
        for (int j = rem; j; --j) {
            char c = p[str.size() - j];
            putchar(isprint(c) ? c : '.');
        }
        for (int j = 0; j < 16 - rem; ++j)
            putchar('.');
        putchar('|');
        putchar('\n');
    }
}

int main() 
{
    at::parser<64> parser;
    
    parser.setup("+TEST");
    parser.clear();

    while (1) {

        parser.process(getc(stdin));

        if (parser.full()       ||
            parser.acquired()   ||
            parser.response()   != at::invalid) 
        {
            puts("+-------RAW_-------+");
            log_hex(parser.get_raw());
            puts("+-------LINE-------+");
            log_hex(parser.get_line());
            puts("+-------ARGS-------+");
            log_hex(parser.get_args());
            puts("+------------------+");
            parser.clear();
        }
    }
}
