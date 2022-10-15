#include "elfc/elf.h"

#include <stdio.h>
#include <time.h>

ELF_DECLARE_TICK_PROVIDER_FUNC(tick)
{
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
