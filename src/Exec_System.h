#ifndef INC_EXEC_SYSTEM_H
#define INC_EXEC_SYSTEM_H
#include "Exec.h"
/// <Enter description of Exec_System here>
class Exec_System : public Exec {
  public:
    /// CONSTRUCTOR
    Exec_System();
    void Help() const;
    RetType Execute(Manager&, Cols&) const;
};
#endif
