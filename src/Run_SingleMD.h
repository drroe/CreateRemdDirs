#ifndef INC_RUN_SINGLEMD_H
#define INC_RUN_SINGLEMD_H
#include "Run.h"
#include "Simulation.h"
/// Hold info for a single MD run.
class Run_SingleMD : public Run {
  public:
    Run_SingleMD();

    static Run* Alloc() { return (Run*)new Run_SingleMD(); }
    //Run* Copy() const;
  private:
    int InternalSetup(FileRoutines::StrArray const&);

    Simulation sim_;
};
#endif
