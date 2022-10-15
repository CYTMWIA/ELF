# 事件循环框架

现有两个版本，`elfc`和`elfcpp`，功能基本相同  
`elfc`由C语言实现，符合C89标准  
`elfcpp`由C++实现，符合C++11标准  
由于某些原因，我尚未在实际的嵌入式项目中使用本框架  

先写着玩玩，万一哪天真用上了呢，毕竟不是所有芯片都有操作系统移植（逃  

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
以上代码将会输出：  
```
interval1: 1001
event1: 1001
interval1 after wait: 1501
interval1: 2001
event1: 2001
interval1 after wait: 2501
...
```

## 编译例子（测试用）

### `elfc`
```bash
clang test.c elfc/elf.c -g -o test.out -std=c89 -Wall -pedantic
```

### `elfcpp`
```bash
clang++ test.cc elfcpp/elf.cc -g -o test.out -std=c++11 -Wall -pedantic
```