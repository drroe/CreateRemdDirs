#ifndef INC_QUEUEOPTS_H
#define INC_QUEUEOPTS_H
#include <string>
#include <vector>
class TextFile;
/// Hold options for queuing system
class QueueOpts {
  public:
    QueueOpts();

    enum QUEUETYPE { PBS = 0, SLURM, NO_QUEUE };
    enum DEPENDTYPE { BATCH = 0, SUBMIT, NONE, NO_DEP };

    int ProcessOption(std::string const&, std::string const&);
    int Check() const;
    void Info() const;
    void CalcThreads();
    int QsubHeader(TextFile&, int, std::string const&, std::string const&);

    DEPENDTYPE DependType() const { return dependType_; }
    QUEUETYPE QueueType()   const { return queueType_; }
    const char* SubmitCmd() const { return SubmitCmdStr[queueType_]; }
  private:
    typedef std::vector<std::string> Sarray;

    void AdditionalFlags(TextFile&) const;

    static const char* QueueTypeStr[];
    static const char* DependTypeStr[];
    static const char* SubmitCmdStr[];
    // TODO reorganize
    std::string job_name_;           ///< Unique job name
    int nodes_;                      ///< Number of nodes
    int ppn_;                        ///< Processors per node
    int threads_;                    ///< Total number of threads required.
    std::string walltime_;           ///< Wallclock time for queuing system
    std::string email_;              ///< User email address
    std::string account_;            ///< Account for running jobs
    std::string amberhome_;          ///< Location of amber
    std::string program_;            ///< Program name
    QUEUETYPE queueType_;            ///< PBS or SBATCH
    std::string mpirun_;             ///< MPI run command
    std::string nodeargs_;           ///< Any additional node arguments
    std::string additionalCommands_; ///< Any additional script commands.
    std::string modfileName_;        ///< Module file name
    std::string queueName_;          ///< Name of queue to submit to.
    bool isSerial_;                  ///< If true MPI run command not required.
    DEPENDTYPE dependType_;          ///< How to handle dependencies 
    Sarray Flags_;                   ///< Additional queue flags.
};
#endif
