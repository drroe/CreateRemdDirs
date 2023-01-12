#ifndef INC_EXEC_ACTIVE_H
#define INC_EXEC_ACTIVE_H
#include "Exec.h"
/// <Enter description of Exec_Active here>
class Exec_Active : public Exec {
  public:
    /// CONSTRUCTOR
    Exec_Active();
    void Help() const;
    RetType Execute(Manager&, Cols&) const;
};
#endif
