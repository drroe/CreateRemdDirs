#ifndef INC_RUN_SINGLE_H
#define INC_RUN_SINGLE_H
#include "Run.h"
#include "Simulation.h"
/// Hold info for a single run.
class Run_Single : public Run {
  public:
    Run_Single();

    static Run* Alloc() { return (Run*)new Run_Single(); }
    //Run* Copy() const;
  private:
    int InternalSetup(FileRoutines::StrArray const&);

    Simulation sim_;
};
#endif
