#ifndef INC_SUBMIT_H
#define INC_SUBMIT_H
#include <string>
#include <vector>
/// Class used to submit jobs via a queuing system.
class Submit {
  public:
    Submit() : Run_(0), Analyze_(0), Archive_(0), n_input_read_(0) {}
   ~Submit();

   int ReadOptions(std::string const&);
  private:
    class QueueOpts;
    int ReadOptions(std::string const&, QueueOpts&);

    enum RunType { MD=0, TREMD, HREMD, MREMD, ANALYSIS, ARCHIVE };
    enum QueueType { PBS = 0, SLURM };
    enum DependType { BATCH = 0, SUBMIT };
    typedef std::vector<std::string> Sarray;

    QueueOpts *Run_; ///< Run queue options
    QueueOpts *Analyze_; ///< Analysis queue options
    QueueOpts *Archive_; ///< Archive queue options
    int n_input_read_; ///< # of times ReadOptions has been called.
};

class Submit::QueueOpts {
  public:
    QueueOpts();

    int ProcessOption(std::string const&, std::string const&);
  private:
    std::string job_name_; ///< Unique job name
    int nodes_; ///< Number of nodes
    int ng_; ///< Number of groups (-ng command line flag).
    int ppn_; ///< Processors per node
    int threads_; ///< Total number of threads required.
    RunType runType_; ///< Run type
    bool overWrite_; ///< If true overwrite existing scripts.
    bool testing_; ///< If true do not actually submit scripts.
    std::string walltime_; ///< Wallclock time for queuing system
    std::string email_; ///< User email address
    std::string account_; ///< Account for running jobs
    std::string amberhome_; ///< Location of amber
    std::string program_; ///< Program name
    QueueType queueType_; ///< PBS or SBATCH
    std::string mpirun_; ///< MPI run command
    std::string nodeargs_; ///< Any additional node arguments
    std::string additionalCommands_; ///< Any additional script commands.
    std::string queueName_; ///< Name of queue to submit to.
    bool isSerial_; ///< If true MPI run command not required.
    DependType dependType_; ///< 0: Use batch. 1: Chain via submit.
    bool setupDepend_; ///< If true set up job dependencies.
    Sarray Flags_; ///< Additional queue flags.
};
#endif
