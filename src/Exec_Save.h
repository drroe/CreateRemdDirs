#ifndef INC_EXEC_SAVE_H
#define INC_EXEC_SAVE_H
#include "Exec.h"
/// <Enter description of Exec_Save here>
class Exec_Save : public Exec {
  public:
    /// CONSTRUCTOR
    Exec_Save();
    void Help() const;
    RetType Execute(Manager&, Cols&) const;
};
#endif
