#ifndef INC_SUBMITTER_H
#define INC_SUBMITTER_H
#include <string>
#include "Queue.h"
#include "OptArray.h"
class TextFile;
/// Used to submit jobs for runs in a particular system
class Submitter {
  public:
    /// Type describing how to handle dependencies. Sync with DependTypeStr_
    enum DependType { BATCH = 0, SUBMIT, NO_DEPENDS };
    /// CONSTRUCTOR
    Submitter();

    /// Print help to stdout
    static void OptHelp();

    /// Write options to a file
    int WriteOptions(TextFile&) const;
    /// Parse a single option
    int ParseFileOption(OptArray::OptPair const&);
    /// Read options from a file
    int ReadOptions(std::string const&);
    /// Set default user if needed
    void SetDefaultUser();
    /// Check that options are valid
    int CheckSubmitter() const;
    /// Set debug level
    void SetDebug(int);

    /// Print info to stdout
    void Info() const;
    /// Submit job, set job id
    int SubmitJob(std::string&, std::string const&, bool, int, std::string const&, bool) const;

    /// \return Program name
    std::string const& Program() const { return program_; }
    /// \return MPI run command
    std::string const& MpiRun() const { return mpirun_; }
  private:
    /// KEEP IN SYNC WITH DependType
    static const char* DependTypeStr_[];
    /// Write queue header
    int writeHeader(TextFile&, int, std::string const&, int) const;
    /// Perform the job submission
    int DoSubmit(std::string&, std::string const&) const;

    int debug_;             ///< Debug level
    std::string job_name_;  ///< Job name
    int nodes_;             ///< Number of nodes required
    int procs_;             ///< Number of processors required
    std::string walltime_;  ///< Wallclock time required
    std::string email_;     ///< User email address
    std::string account_;   ///< Account for running jobs
    std::string user_;      ///< User name
    std::string program_;   ///< Executable name
    std::string mpirun_;    ///< MPI run command if needed
    DependType dependType_; ///< Describes how to handle job dependencies
    Queue localQueue_;      ///< Hold options for queue for a single System
};
#endif
