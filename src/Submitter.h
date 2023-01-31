#ifndef INC_SUBMITTER_H
#define INC_SUBMITTER_H
#include <string>
/// Used to submit jobs for runs in a particular system
class Submitter {
  public:
    /// Type describing how to handle dependencies
    enum DependType { BATCH = 0, SUBMIT, NONE, NO_DEPENDS };
    /// CONSTRUCTOR
    Submitter();
    /// Read options from a file
    int ReadOptions(std::string const&);
    /// Set debug level
    void SetDebug(int);
  private:
    int debug_;             ///< Debug level
    std::string job_name_;  ///< Job name
    int nodes_;             ///< Number of nodes required
    int procs_;             ///< Number of processors required
    std::string walltime_;  ///< Wallclock time required
    std::string email_;     ///< User email address
    std::string account_;   ///< Account for running jobs
    std::string program_;   ///< Executable name
    std::string mpirun_;    ///< MPI run command if neededi
    DependType dependType_; ///< Describes how to handle job dependncies
};
#endif
