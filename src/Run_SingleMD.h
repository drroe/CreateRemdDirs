#ifndef INC_RUN_SINGLEMD_H
#define INC_RUN_SINGLEMD_H
#include "Run.h"
#include "Simulation.h"
/// Hold info for a single MD run.
class Run_SingleMD : public Run {
  public:
    Run_SingleMD();

    static Run* Alloc() { return (Run*)new Run_SingleMD(); }
    /// COPY CONSTRUCTOR
    Run_SingleMD(Run_SingleMD const& rhs) : Run(rhs), sim_(rhs.sim_) {}
    /// ASSIGNMENT
    Run_SingleMD& operator=(Run_SingleMD const& rhs) {
      if (&rhs == this) return *this;
      Run::operator=(rhs);
      sim_ = rhs.sim_;
      return *this;
    }
    /// \return Copy of this run
    Run* Copy() const { return (Run*)new Run_SingleMD( *this ); }
    /// Print run info to stdout
    void RunInfo() const;
    /// Create run directory
    int CreateRunDir(Creator const&, int, int, std::string const&) const;
  private:
    int InternalSetup(FileRoutines::StrArray const&);

    Simulation sim_;
};
#endif
