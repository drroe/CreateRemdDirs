#ifndef INC_RUN_H
#define INC_RUN_H
#include <string>
#include <vector>
#include "RunStatus.h"
class Creator;
class MdPackage;
class Submitter;
class Queue;
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
    void RunSummary() const;
    /// \return Run status object
    RunStatus const& Stat() const { return runStat_; }
    /// \return run dir name
    std::string const& RunDirName() const { return rundir_; }
    /// \return run job id
    std::string const& JobId() const { return jobid_; }
    /// Setup existing directory - run dir name
    int SetupExisting(std::string const&, MdPackage*, Queue const&);
    /// Refresh run status
    int Refresh(MdPackage*, Queue const&);
    /// Create new directory - rundir, creator, mdpackage, start #, run #, prev run dir
    int CreateNew(std::string const&, Creator const&, Submitter const&, MdPackage*, int, int, std::string const&);
    /// Submit run
    int SubmitRun(Submitter const&, bool, std::string const&, std::string const&, bool);
  private:
    /// Get job id from file if present in array
    int getJobIdFromFile(std::vector<std::string> const&, Queue const&);
    /// Update time last modified from files in array
    int updateTimeLastModified(std::vector<std::string> const&);

    std::string rundir_;   ///< Run directory. May be relative.
    std::string setupDir_; ///< Directory in which SetupRun gets invoked. Should be absolute run dir
    RunStatus runStat_;    ///< Current run status
    std::string jobid_;    ///< Current job id
    int idx_;              ///< Run index, based on directory name extension
    int debug_;            ///< Debug level
    long int t_last_mod_;  ///< Time most recent file in run was modified.
};
#endif
