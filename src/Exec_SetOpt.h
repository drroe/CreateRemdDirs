#ifndef INC_EXEC_SETOPT_H
#define INC_EXEC_SETOPT_H
#include "Exec.h"
/// Used to set create/submit options 
class Exec_SetOpt : public Exec {
  public:
    /// CONSTRUCTOR
    Exec_SetOpt();
    void Help() const;
    void Help(Cols&) const;
    RetType Execute(Manager&, Cols&) const;
};
#endif
