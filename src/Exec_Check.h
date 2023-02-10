#ifndef INC_EXEC_CHECK_H
#define INC_EXEC_CHECK_H
#include "Exec.h"
/// <Enter description of Exec_Check here>
class Exec_Check : public Exec {
  public:
    /// CONSTRUCTOR
    Exec_Check();
    void Help() const;
    RetType Execute(Manager&, Cols&) const;
};
#endif
