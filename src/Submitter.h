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

    /// Parse a single option
    int ParseFileOption(OptArray::OptPair const&);
    /// Read options from a file
    int ReadOptions(std::string const&);
    /// Set debug level
    void SetDebug(int);
    /// Ensure current options are valid
    int RefreshSubmitter();

    /// Print info to stdout
    void Info() const;
    /// Submit job, set job id
    int SubmitJob(std::string&, std::string const&, int, std::string const&) const;
  private:
    /// KEEP IN SYNC WITH DependType
    static const char* DependTypeStr_[];
    /// Write queue header
    int writeHeader(TextFile&, int, std::string const&, int) const;

    /// Check that the submitter is valid
    int setupSubmitter();

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
    DependType dependType_; ///< Describes how to handle job dependncies
    Queue localQueue_;      ///< Hold options for queue for a single System
};
#endif
