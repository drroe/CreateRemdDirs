#ifndef INC_EXEC_INFO_H
#define INC_EXEC_INFO_H
#include "Exec.h"
/// <Enter description of Exec_Info here>
class Exec_Info : public Exec {
  public:
    /// CONSTRUCTOR
    Exec_Info();
    void Help() const;
    RetType Execute(Manager&, Cols&) const;
};
#endif
