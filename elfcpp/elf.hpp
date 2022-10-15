#ifndef __ELF_ELF_HPP__
#define __ELF_ELF_HPP__

#include "elf_settings.hpp"

#include <functional>
#include <memory>

namespace elf {

class ELF;
using Tick = elf_settings::Tick;
using EventCode = elf_settings::EventCode;
using TaskFunc = std::function<void(ELF &)>;
using CallbackFunc = TaskFunc;
using TickProviderFunc = std::function<Tick(void)>;

class ELF {
private:
  struct Impl;
  std::unique_ptr<Impl> pimpl_;

public:
  ELF(const TickProviderFunc &);
  ~ELF();

  void set_interval(Tick, const CallbackFunc &);
  void set_timeout(Tick, const CallbackFunc &);

  void emit(EventCode);
  void listen(EventCode, const CallbackFunc &);

  void join_loop();
  void exit_loop(int least_loop = 1);

  void wait(Tick);
};
} // namespace elf

#endif