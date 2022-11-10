# at

Parser of AT modem responses as finite-state machine.

## Guide

## Examples

```cpp
at::parser<64> parser;

parser.setup("+TEST");
parser.clear();

while (1) {

    parser.process(getc()); // from UART for example

    if (parser.full()       ||
        parser.acquired()   ||
        parser.response()   != at::invalid) 
    {
        puts("+-------RAW_-------+");
        log_hex(parser.raw().data(), parser.raw().size());
        puts("+-------LINE-------+");
        log_hex(parser.line().data(), parser.line().size());
        puts("+-------ARGS-------+");
        log_hex(parser.args().data(), parser.args().size());
        puts("+------------------+");
        break;
    }
}
```

## TODO

- [x] source
- [ ] tests
- [ ] readme
    - [ ] intro
    - [ ] guide
    - [x] examples
