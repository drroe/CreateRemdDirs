#ifndef INC_EXEC_QUIT_H
#define INC_EXEC_QUIT_H
#include "Exec.h"
class Exec_Quit : public Exec {
  public:
    Exec_Quit() {}
    std::string Help() const { return "Exits the program."; }
    RetType Execute(Manager&, Cols&) const { return QUIT; }
};
#endif
