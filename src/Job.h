#ifndef INC_JOB_H
#define INC_JOB_H
/// Hold information on how to submit runs for a particular system
class Job {
  public:
    /// Type describing how to handle dependencies
    enum DependType { BATCH = 0, SUBMIT, NONE, NO_DEPENDS };

    Job();
  private:
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
