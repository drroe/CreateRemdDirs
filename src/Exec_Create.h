#ifndef INC_EXEC_CREATE_H
#define INC_EXEC_CREATE_H
#include "Exec.h"
#include "FileRoutines.h"
class Exec_Create : public Exec {
  public:
    Exec_Create();
    void Help() const;
    RetType Execute(Manager&, Cols&) const;
  private:
    static FileRoutines::StrArray createRunDirs(int, int, int);
};
#endif
