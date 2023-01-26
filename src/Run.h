#ifndef INC_RUN_H
#define INC_RUN_H
#include <string>
#include "RunStatus.h"
class Creator;
class MdPackage;
/// Hold information for a single run
class Run {
  public:
    /// CONSTRUCTOR
    Run();
    /// \return Current debug level
    int Debug() const { return debug_; }
    /// \return Run index
    int RunIndex() const { return idx_; }
    /// Print run info to stdout
    void RunInfo() const;
    /// \return Run status object
    RunStatus const& Stat() const { return runStat_; }
    /// \return run dir name
    std::string const& RunDirName() const { return rundir_; }
    /// Setup existing directory - run dir name
    int SetupExisting(std::string const&, MdPackage*);
    /// Create new directory - rundir, creator, mdpackage, start #, run #, prev run dir
    int CreateNew(std::string const&, Creator const&, MdPackage*, int, int, std::string const&);
    
  private:
    std::string rundir_;   ///< Run directory. May be relative.
    std::string setupDir_; ///< Directory in which SetupRun gets invoked. Should be absolute run dir
    RunStatus runStat_;    ///< Current run status
    int idx_;              ///< Run index, based on directory name extension
    int debug_;            ///< Debug level
};
#endif
