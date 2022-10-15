#ifndef __ELFC_ELF_H__
#define __ELFC_ELF_H__

/* ELF */
#include "elf_settings.h"

/* Basic Types */
struct ELF;
typedef struct ELF ELF;
typedef void (*ELF_TaskFunc)(ELF *);
typedef ELF_Tick (*ELF_TickProviderFunc)(void);

/*
 * Alias Typedefs
 */

typedef ELF_TaskFunc ELF_CallbackFunc;
typedef ELF_Tick ELF_Duration;

/*
 * Export Functions
 */

ELF elf_new(ELF_TickProviderFunc);
void elf_join(ELF *);
void elf_exit(ELF *);

void elf_set_interval(ELF *, ELF_CallbackFunc, ELF_Duration);
void elf_set_timeout(ELF *, ELF_CallbackFunc, ELF_Duration);

void elf_emit(ELF *, ELF_Event);
void elf_listen(ELF *, ELF_Event, ELF_CallbackFunc);

void elf_wait(ELF *, ELF_Duration);

/*
 * Macros
 */

#define ELF_DECLARE_CALLBACK_FUNC(name) void name(ELF *pelf)
#define ELF_DECLARE_TICK_PROVIDER_FUNC(name) ELF_Tick name(void)

/*
 * Structs
 * THEY ARE PRIVATE!!!
 */

struct ELF_Timer {
  ELF_CallbackFunc callback;
  ELF_Tick timeout_tick;
  ELF_Tick delay;
  ELF_u8 deleted : 1, overflow : 1, once : 1;
};

struct ELF_TimersControl {
  struct ELF_Timer timers[ELF_MAX_TIMERS];
  ELF_size count;
  ELF_Tick last_tick;
};

struct ELF_Listener {
  ELF_Event event;
  ELF_CallbackFunc callback;
  ELF_u8 deleted : 1;
};

struct ELF_ListenersControl {
  struct ELF_Listener listeners[ELF_MAX_LISTENERS];
  ELF_size count;
};

struct ELF_TasksControl {
  ELF_TaskFunc tasks[ELF_MAX_TASKS];
  ELF_size begin, end;
};

struct ELF {
  struct ELF_TasksControl tasks_control;
  struct ELF_TimersControl timers_control;
  struct ELF_ListenersControl listeners_control;

  ELF_TickProviderFunc tick_provider_func;
  ELF_u8 exit_flag : 1;
};

#endif
