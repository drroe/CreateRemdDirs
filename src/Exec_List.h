#ifndef INC_EXEC_LIST_H
#define INC_EXEC_LIST_H
#include "Exec.h"
class Exec_List : public Exec {
  public:
    Exec_List();
    std::string Help() const { return "List systems."; }
    RetType Execute(Manager&, Cols&) const;
};
#endif
