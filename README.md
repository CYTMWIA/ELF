# Event Loop Framework for Embedded System

[中文介绍](./README.zh-CN.md)

There are two version of this framework: `elfc` and `elfcpp`.  
`elfc` was written in C89.  
`elfcpp` was written in C++11.  
Both nearly have the same features.
For some reasons, I haven't test this framework in real embedded system yet.

## Examples (`elfc`)

```C
#include "elfc/elf.h"

#include <stdio.h>
#include <time.h>

ELF_DECLARE_TICK_PROVIDER_FUNC(tick)
{
    /* You can implement this function by using hardware timers.
     */
    clock_t ms = clock() * (1000.0 / CLOCKS_PER_SEC);
    return ms % 65536;
}

ELF_DECLARE_CALLBACK_FUNC(interval1)
{
    printf("interval1: %d\n", tick());
    elf_emit(pelf, 1);
    elf_wait(pelf, 500);
    printf("interval1 after wait: %d\n", tick());
}

ELF_DECLARE_CALLBACK_FUNC(event1)
{
    printf("event1: %d\n", tick());
}

int main()
{
    ELF elf = elf_new(tick);
    elf_set_interval(&elf, interval1, 1000);
    elf_listen(&elf, 1, event1);
    elf_join(&elf);
    return 0;
}
```
Above codes will output:  
```
interval1: 1001
event1: 1001
interval1 after wait: 1501
interval1: 2001
event1: 2001
interval1 after wait: 2501
...
```

## Complie Example (for test)

### `elfc`
```bash
clang test.c elfc/elf.c -g -o test.out -std=c89 -Wall -pedantic
```

### `elfcpp`
```bash
clang++ test.cc elfcpp/elf.cc -g -o test.out -std=c++11 -Wall -pedantic
```