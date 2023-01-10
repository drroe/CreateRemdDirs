#ifndef INC_EXEC_DEBUG_H
#define INC_EXEC_DEBUG_H
#include "Exec.h"
class Exec_Debug : public Exec {
  public:
    Exec_Debug();
    void Help() const;
    RetType Execute(Manager&, Cols&) const;
};
#endif
