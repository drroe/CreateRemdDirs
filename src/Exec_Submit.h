#ifndef INC_EXEC_SUBMIT_H
#define INC_EXEC_SUBMIT_H
#include "Exec.h"
class Exec_Submit : public Exec {
  public:
    Exec_Submit();
    void Help() const;
    RetType Execute(Manager&, Cols&) const;
};
#endif
