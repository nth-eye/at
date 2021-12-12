# at

Parser of AT command responses which can handle callbacks.

## How to use

Create array of callbacks, provide working buffer, initialize parser and start feeding incoming data.

```c
AT_Cmd callbacks[] = { 
    { "+TEST",  test    },
};
char buf[128];

const AT_Cfg cfg = {
    .cbks       = callbacks,
    .buf        = buf, 
    .cbks_size  = 1,
    .buf_size   = sizeof(buf),
};

AT at;

at_init(&at, &cfg);

while (1) {

    char c = getc(...); // get byte from UART for example

    at_process(&at, c);
}
```

## TODO

**_Nothing_**
