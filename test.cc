#include "elfcpp/elf.hpp"

#include <ctime>
#include <iostream>

using namespace std;

elf::Tick elf_now() {
  uintmax_t ms = clock() * (1000.0 / CLOCKS_PER_SEC);
  return ms % 65536;
}

int main() {
    elf::ELF elf1(elf_now);

    elf1.set_interval(1000, [](elf::ELF& self){
        cout << "interval-1 start wait: " << elf_now() << endl;
        self.emit(1);
        // self.wait(2000);
        // cout << "interval-1 end wait: " << elf_now() << endl;
    });

    elf1.listen(1, [](elf::ELF& self){
        cout << "listen(1)-1 triggered: " << elf_now() << endl;
    });
    elf1.listen(1, [](elf::ELF& self){
        cout << "listen(1)-2 triggered: " << elf_now() << endl;
    });

    elf1.join_loop();
}