#ifndef INC_RUN_SINGLE_H
#define INC_RUN_SINGLE_H
#include "Run.h"
/// Hold info for a single run.
class Run_Single : public Run {
  public:
    Run_Single() : Run(SINGLE_MD) {}

    static Run* Alloc() { return (Run*)new Run_Single(); }
    //Run* Copy() const;
  private:
};
#endif
