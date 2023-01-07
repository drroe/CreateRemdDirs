#ifndef INC_EXEC_LIST_H
#define INC_EXEC_LIST_H
#include "Exec.h"
class Exec_List : public Exec {
  public:
    Exec_List();
    void Help() const;
    RetType Execute(Manager&, Cols&) const;
};
#endif
