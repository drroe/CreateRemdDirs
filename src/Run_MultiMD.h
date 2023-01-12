#ifndef INC_RUN_MULTIMD_H
#define INC_RUN_MULTIMD_H
#include "Run.h"
/// <Enter description of Run_MultiMD here>
class Run_MultiMD : public Run {
  public:
    /// CONSTRUCTOR
    Run_MultiMD();

    static Run* Alloc() { return (Run*)new Run_MultiMD(); }
    /// COPY CONSTRUCTOR
    Run_MultiMD(Run_MultiMD const&);
    /// ASSIGNMENT
    Run_MultiMD& operator=(Run_MultiMD const&);

    /// \return Copy of this run
    Run*  Copy() const { return (Run*)new Run_MultiMD( *this ); }
    /// Print run info to stdout
    void RunInfo() const;
    /// Create run directory
    int CreateRunDir(Creator const&, int, int, std::string const&) const;
  private:
    int InternalSetup(FileRoutines::StrArray const&);

};
#endif
