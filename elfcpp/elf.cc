#include "elf.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <queue>
#include <vector>

namespace elf {
using TasksContainer = std::queue<TaskFunc>;

class TimersContainer {
private:
  struct Timer {
    // will be push to task queue after timer was triggered
    CallbackFunc callback;

    /* normally, when (now>=timeout_tick) timer will be
     * triggered
     */
    Tick timeout_tick;

    // this value would take effect if (once==0)
    Tick delay;

    uint_least8_t
        // timer can be covered if deleted
        deleted : 1,

        // if timer can only trigger once
        once : 1,

        /* timeout_tick overflow, timer won't be triggered in this
         * tick cycle
         */
        overflow : 1;
  };

  Tick last_tick_;

  std::vector<Timer> timers_;
  using TimerIndex = size_t;

public:
  TimersContainer() : last_tick_(0), timers_() {}

  TimerIndex push(Tick now, Tick delay, bool once, const CallbackFunc &callback,
                  TimerIndex start_insert = 0) {
    Tick timeout = now + delay;
    Timer timer{callback, timeout, delay, 0, once, now > timeout};
    TimerIndex idx;
    for (idx = start_insert; idx < timers_.size(); idx++) {
      if (timers_[idx].deleted) {
        timers_[idx] = timer;
        break;
      }
    }
    if (idx == timers_.size())
      timers_.push_back(std::move(timer));
    return idx;
  }

  void pop(TimerIndex idx) {
    if (idx < timers_.size())
      timers_[idx].deleted = true;
  }

  void check(Tick now, TasksContainer &tasks) {
    bool new_cycle = now < last_tick_;
    for (TimerIndex i = 0; i < timers_.size(); i++) {
      auto &timer = timers_[i];
      // overflow new_cycle -> overflow(new)
      //        0         0                0
      //        0         1                0
      //        1         0                1
      //        1         1                0
      timer.overflow &= timer.overflow ^ new_cycle;
      // When should a timer be triggered?
      // normally: timer.timeout_tick <= now
      // and, if (new_cycle==1): last_tick_ <= timer.timeout_tick
      // timeout<=now | new_cycle | last_tick<=timeout --> trigger
      //            0           0                    0           0
      //            0           0                    1           0
      //            0           1                    0           0
      //            0           1                    1           1
      //            1           ?                    ?           1
      if (timer.deleted || timer.overflow)
        continue;
      if (!(timer.timeout_tick <= now || (new_cycle && last_tick_ <= timer.timeout_tick)))
        continue;
      // timer triggered
      tasks.push(timer.callback);
      timer.deleted = 1;
      if (!timer.once) // will cover current timer
        push(now, timer.delay, false, timer.callback, i);
    }
    last_tick_ = now;
  }
};

class ListenersContainer {
private:
  struct Listener {
    EventCode event;
    CallbackFunc callback;
    uint_least8_t deleted : 1;
  };
  std::vector<Listener> listeners_;
  using ListenerIndex = size_t;

public:
  void emit(EventCode event, TasksContainer &tasks) {
    for (const auto &listener : listeners_) {
      if (!listener.deleted && listener.event == event)
        tasks.push(listener.callback);
    }
  }
  void add_listener(EventCode event, const CallbackFunc &callback) {
    Listener listener{event, callback, 0};
    listeners_.push_back(std::move(listener));
  }
};

struct ELF::Impl {
  int loop_count_;
  bool exit_loop_flag_;
  TickProviderFunc tick_provider_;
  TimersContainer timers_container_;
  ListenersContainer listeners_container_;
  TasksContainer tasks_container_;

  Impl(const TickProviderFunc &tick_provider)
      : loop_count_(0), exit_loop_flag_(false), tick_provider_(tick_provider),
        timers_container_(), tasks_container_() {}

  void push_timer(Tick delay, bool once, const CallbackFunc &callback) {
    timers_container_.push(tick_provider_(), delay, once, callback);
  }

  void check_timers() {
    timers_container_.check(tick_provider_(), tasks_container_);
  }

  void emit(EventCode event) {
    listeners_container_.emit(event, tasks_container_);
  }

  void listen(EventCode event, const CallbackFunc &callback) {
    listeners_container_.add_listener(event, callback);
  }

  void join_loop(ELF &self) {
    loop_count_++;

    while (!exit_loop_flag_) {
      check_timers();

      while (!exit_loop_flag_ && !tasks_container_.empty()) {
        auto task = tasks_container_.front();
        tasks_container_.pop();
        task(self);
      }
    }

    loop_count_--;
    exit_loop_flag_ = false;
  }

  void exit_loop(int least_loop) {
    if (loop_count_ > least_loop) {
      exit_loop_flag_ = true;
    }
  }
};
ELF::~ELF() = default;
ELF::ELF(const TickProviderFunc &tick_provider)
    : pimpl_(new Impl(tick_provider)) {}

void ELF::join_loop() { pimpl_->join_loop(*this); }

void ELF::exit_loop(int least_loop) { pimpl_->exit_loop(least_loop); }

void ELF::set_interval(Tick delay, const CallbackFunc &callback) {
  pimpl_->push_timer(delay, false, callback);
}

void ELF::set_timeout(Tick delay, const CallbackFunc &callback) {
  pimpl_->push_timer(delay, true, callback);
}

void ELF::emit(EventCode event) { pimpl_->emit(event); }
void ELF::listen(EventCode event, const CallbackFunc &callback) {
  pimpl_->listen(event, callback);
}

void ELF::wait(Tick delay) {
  set_timeout(delay, [](ELF &self) { self.exit_loop(); });
  join_loop();
}

} // namespace elf