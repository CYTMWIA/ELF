#include "elf.h"
#include "elf_settings.h"

/*
 * Internal Functions
 */

#define TASKS_INDEX_NEXT(var) (var) = ((var) + 1) % (ELF_MAX_TASKS)

static void tasks_push(struct ELF_TasksControl *ptasks, ELF_TaskFunc func) {
  ptasks->tasks[ptasks->end] = func;
  TASKS_INDEX_NEXT(ptasks->end);
}

static void timer_next(struct ELF_Timer *ptimer) {
  ELF_Tick before = ptimer->timeout_tick;
  ptimer->timeout_tick += ptimer->delay;
  ptimer->overflow = before > ptimer->timeout_tick;
}

static void timers_push(struct ELF_TimersControl *ptimers,
                        struct ELF_Timer *ptimer) {
  ELF_size i;
  for (i = 0; i < ptimers->count; i++) {
    if (ptimers->timers[i].deleted) {
      ptimers->timers[i] = *ptimer;
      return;
    }
  }
  if (i < (ELF_MAX_TIMERS)) {
    ptimers->timers[i] = *ptimer;
    ptimers->count++;
  }
}

static void timers_scan(ELF *pelf) {
  ELF_Tick now = pelf->tick_provider_func();
  struct ELF_TimersControl *ptimers = &pelf->timers_control;
  ELF_u8 new_cycle = now < ptimers->last_tick;
  ELF_size i;
  struct ELF_Timer *pt;
  ELF_u8 trigger;
  for (i = 0; i < ptimers->count; i++) {
    pt = &ptimers->timers[i];
    /* overflow new_cycle -> overflow(new)
    //        0         0                0
    //        0         1                0
    //        1         0                1
    //        1         1                0
    */
    pt->overflow &= pt->overflow ^ new_cycle;
    /* timeout<=now | new_cycle | last_tick<timeout --> trigger
    //            0           0                   0           0
    //            0           0                   1           0
    //            0           1                   0           0
    //            0           1                   1           1
    //            1           ?                   ?           1
    */
    trigger = (pt->timeout_tick <= now ||
               (new_cycle && ptimers->last_tick <= pt->timeout_tick));
    if (pt->deleted || pt->overflow || !trigger)
      continue;
    tasks_push(&pelf->tasks_control, pt->callback);
    if (pt->once)
      pt->deleted = 1;
    else
      timer_next(pt);
  }
  ptimers->last_tick = now;
}

/*
 * Export Functions
 */

ELF elf_new(ELF_TickProviderFunc func) {
  ELF elf = {0};
  elf.tick_provider_func = func;
  return elf;
}

void elf_join(ELF *pelf) {
  ELF_size i = 0;
  struct ELF_TasksControl *ptasks = &pelf->tasks_control;
  while (!pelf->exit_flag) {
    timers_scan(pelf);

    while (!pelf->exit_flag && ptasks->begin != ptasks->end) {
      i = ptasks->begin;
      TASKS_INDEX_NEXT(ptasks->begin);
      ptasks->tasks[i](pelf);
    }
  }
  pelf->exit_flag = 0;
}

void elf_exit(ELF *pelf) { pelf->exit_flag = 1; }

void elf_set_interval(ELF *pelf, ELF_CallbackFunc callback,
                      ELF_Duration delay) {
  ELF_Tick now = pelf->tick_provider_func();
  struct ELF_Timer tim = {0};
  tim.callback = callback;
  tim.timeout_tick = now + delay;
  tim.delay = delay;
  tim.deleted = 0;
  tim.overflow = now > tim.timeout_tick;
  tim.once = 0;
  timers_push(&pelf->timers_control, &tim);
}

void elf_set_timeout(ELF *pelf, ELF_CallbackFunc callback, ELF_Duration delay) {
  ELF_Tick now = pelf->tick_provider_func();
  struct ELF_Timer tim = {0};
  tim.callback = callback;
  tim.timeout_tick = now + delay;
  tim.delay = delay;
  tim.deleted = 0;
  tim.overflow = now > tim.timeout_tick;
  tim.once = 1;
  timers_push(&pelf->timers_control, &tim);
}

void elf_emit(ELF *pelf, ELF_Event ev) {
  struct ELF_ListenersControl *plisteners = &pelf->listeners_control;
  struct ELF_Listener *pl;
  ELF_size i;
  for (i = 0; i < plisteners->count; i++) {
    pl = &plisteners->listeners[i];
    if (!pl->deleted && pl->event == ev) {
      tasks_push(&pelf->tasks_control, pl->callback);
    }
  }
}

void elf_listen(ELF *pelf, ELF_Event ev, ELF_CallbackFunc callback) {
  struct ELF_ListenersControl *plisteners = &pelf->listeners_control;
  struct ELF_Listener *pl;
  ELF_size i;
  for (i = 0; i < plisteners->count; i++) {
    pl = &plisteners->listeners[i];
    if (pl->deleted) {
      pl->event = ev;
      pl->callback = callback;
      pl->deleted = 0;
      return;
    }
  }
  if (i == plisteners->count) {
    pl = &plisteners->listeners[i];
    pl->event = ev;
    pl->callback = callback;
    pl->deleted = 0;
    plisteners->count++;
  }
}

void elf_wait(ELF *pelf, ELF_Duration delay) {
  elf_set_timeout(pelf, elf_exit, delay);
  elf_join(pelf);
}
