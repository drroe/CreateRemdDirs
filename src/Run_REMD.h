#ifndef INC_RUN_REMD_H
#define INC_RUN_REMD_H
#include "Run.h"
/// <Enter description of Run_REMD here>
class Run_REMD : public Run {
  public:
    /// CONSTRUCTOR
    Run_REMD();

    static Run* Alloc() { return (Run*)new Run_REMD(); }
    /// COPY CONSTRUCTOR
    Run_REMD(Run_REMD const&);
    /// ASSIGNMENT
    Run_REMD& operator=(Run_REMD const&);

    /// \return Copy of this run
    Run*  Copy() const { return (Run*)new Run_REMD( *this ); }
    /// Print run info to stdout
    void RunInfo() const;
    /// Create run directory
    int CreateRunDir(Creator const&, int, int, std::string const&) const;
  private:
    int InternalSetup(FileRoutines::StrArray const&);

};
#endif
