#ifndef INC_SUBMIT_H
#define INC_SUBMIT_H
#include "FileRoutines.h"
#include "TextFile.h"
/// Class used to submit jobs via a queuing system.
class Submit {
  public:
    Submit() : Run_(0), Analyze_(0), Archive_(0), n_input_read_(0), testing_(false) {}
   ~Submit();

   static void OptHelp();
   int ReadOptions(std::string const&);
   int CheckOptions();
   int SubmitRuns(std::string const&, StrArray const&, int) const;
   int SubmitAnalysis(std::string const&, int, int, bool) const;
   int SubmitArchive(std::string const&, int, int, bool) const;
   void SetTesting(bool t) { testing_ = t; }
  private:
    class QueueOpts;
    int ReadOptions(std::string const&, QueueOpts&);

    enum QUEUETYPE { PBS = 0, SLURM, NO_QUEUE };
    enum DEPENDTYPE { BATCH = 0, SUBMIT, NONE, NO_DEP };
    typedef std::vector<std::string> Sarray;

    QueueOpts *Run_;     ///< Run queue options
    QueueOpts *Analyze_; ///< Analysis queue options
    QueueOpts *Archive_; ///< Archive queue options
    int n_input_read_;   ///< # of times ReadOptions has been called.
    bool testing_;       ///< If true do not actually submit scripts.
};

class Submit::QueueOpts {
  public:
    QueueOpts();

    int ProcessOption(std::string const&, std::string const&);
    int Check() const;
    void Info() const;
    void CalcThreads();
    int QsubHeader(TextFile&, int, std::string const&, std::string const&);

    DEPENDTYPE DependType() const { return dependType_; }
    QUEUETYPE QueueType()   const { return queueType_; }
    bool OverWrite()        const { return overWrite_; }
    const char* SubmitCmd() const { return SubmitCmdStr[queueType_]; }
  private:
    void AdditionalFlags(TextFile&) const;

    static const char* QueueTypeStr[];
    static const char* DependTypeStr[];
    static const char* SubmitCmdStr[];
    // TODO reorganize
    std::string job_name_;           ///< Unique job name
    int nodes_;                      ///< Number of nodes
    int ppn_;                        ///< Processors per node
    int threads_;                    ///< Total number of threads required.
    bool overWrite_;                 ///< If true overwrite existing scripts.
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
