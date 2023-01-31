#include "QueueOpts.h"
#include "Messages.h"
#include "FileRoutines.h"
#include "TextFile.h"
#include "StringRoutines.h"
#include <cstdlib> // atoi

using namespace Messages;
using namespace FileRoutines;
using namespace StringRoutines;

QueueOpts::QueueOpts() :
  nodes_(0),
  ppn_(0),
  threads_(0),
  queueType_(PBS),
  isSerial_(false),
  dependType_(BATCH)
{}

const char* QueueOpts::QueueTypeStr[] = {
  "PBS", "SBATCH"
};

const char* QueueOpts::DependTypeStr[] = {
  "BATCH", "SUBMIT", "NONE"
};

const char* QueueOpts::SubmitCmdStr[] = {
  "qsub", "sbatch"
};

static inline int RetrieveOpt(const char** Str, int end, std::string const& VAR) {
  for (int i = 0; i != end; i++)
    if ( VAR.compare( Str[i] )==0 )
      return i;
  return end;
}

int QueueOpts::ProcessOption(std::string const& OPT, std::string const& VAR) {
  //Msg("Processing '%s' '%s'\n", OPT.c_str(), VAR.c_str()); // DEBUG

  if      (OPT == "JOBNAME") job_name_ = VAR;
  else if (OPT == "NODES"  ) nodes_ = atoi( VAR.c_str() );
  else if (OPT == "PPN"    ) ppn_ = atoi( VAR.c_str() );
  else if (OPT == "THREADS") threads_ = atoi( VAR.c_str() );
  else if (OPT == "RUNTYPE") {
    ErrorMsg("RUNTYPE is obsolete. Please remove.\n");
    return 1;
  }
  else if (OPT == "AMBERHOME") {
    if ( CheckExists("AMBERHOME", VAR) ) return 1;
    amberhome_ = tildeExpansion( VAR );
  }
  else if (OPT == "PROGRAM"  ) program_ = VAR;
  else if (OPT == "QSUB"     ) {
    queueType_ = (QUEUETYPE) RetrieveOpt(QueueTypeStr, NO_QUEUE, VAR);
    if (queueType_ == NO_QUEUE) {
      ErrorMsg("Unrecognized QSUB: %s\n", VAR.c_str());
      return 1;
    }
  }
  else if (OPT == "WALLTIME"   ) walltime_ = VAR;
  else if (OPT == "NODEARGS"   ) nodeargs_ = VAR;
  else if (OPT == "MPIRUN"     ) mpirun_ = VAR;
  else if (OPT == "MODULEFILE" ) modfileName_ = VAR;
  else if (OPT == "COMMANDFILE") {
    if ( CheckExists( "Additional commands file", VAR ) ) return 1;
    Msg("  Reading commands from file %s\n", VAR.c_str());
    TextFile cmdfile;
    if (cmdfile.OpenRead( tildeExpansion(VAR) )) return 1;
    const char* ptr = cmdfile.Gets();
    while (ptr != 0) {
      additionalCommands_.append( std::string(ptr) );
      ptr = cmdfile.Gets();
    }
    cmdfile.Close();
  }
  else if (OPT == "COMMAND") {
    additionalCommands_.append( VAR );
    additionalCommands_.append("\n");
  }
  else if (OPT == "ACCOUNT") account_ = VAR;
  else if (OPT == "EMAIL"  ) email_ = VAR;
  else if (OPT == "QUEUE"  ) queueName_ = VAR;
  else if (OPT == "SERIAL" ) isSerial_ = (bool)atoi( VAR.c_str() );
  else if (OPT == "DEPEND" ) {
    dependType_ = (DEPENDTYPE) RetrieveOpt(DependTypeStr, NO_DEP, VAR);
    if (dependType_ == NO_DEP) {
      ErrorMsg("Unrecognized DEPEND: %s\n", VAR.c_str());
      return 1;
    }
  }
/*  else if (OPT == "CHAIN"  ) {
    int ival = atoi( VAR.c_str() );
    if (ival == 1) dependType_ = SUBMIT;
  }
  else if (OPT == "NO_DEPEND") {
    if (atoi( VAR.c_str()) == 1)
      dependType_ = NO_DEPEND;
  }*/ 
  else if (OPT == "FLAG"     ) Flags_.push_back( VAR );
  else {
    ErrorMsg("Unrecognized option '%s' in input file.\n", OPT.c_str());
    return 1;
  }
  return 0;
}

int QueueOpts::Check() const {
  if (job_name_.empty()) {
    ErrorMsg("No job name\n");
    return 1;
  }
  if (program_.empty()) { // TODO check AMBERHOME ?
    ErrorMsg("PROGRAM not specified.\n");
    return 1;
  }
  if (!isSerial_ && mpirun_.empty()) {
    ErrorMsg("MPI run command MPIRUN not set.\n");
    return 1;
  }
  if (queueType_ == PBS) {
    if (nodes_ < 1) {
      ErrorMsg("Less than 1 node specified (use NODES option).\n");
      return 1;
    }
  } else if (queueType_ == SLURM) {
    if (nodes_ < 1 && threads_ < 1) {
      ErrorMsg("Must specify either NODES or THREADS.\n");
      return 1;
    }
  }
  return 0;
}

void QueueOpts::Info() const {
  Msg("\n---=== Job Submission ===---\n");
  Msg("  JOBNAME   : %s\n", job_name_.c_str());
  if (nodes_ > 0  ) Msg("  NODES     : %i\n", nodes_);
  if (ppn_ > 0    ) Msg("  PPN       : %i\n", ppn_);
  if (threads_ > 0) Msg("  THREADS   : %i\n", threads_);
  if (!amberhome_.empty()) Msg("  AMBERHOME : %s\n", amberhome_.c_str());
  Msg("  PROGRAM   : %s\n", program_.c_str());
  Msg("  QSUB      : %s\n", QueueTypeStr[queueType_]);
  if (!walltime_.empty())    Msg("  WALLTIME  : %s\n", walltime_.c_str());
  if (!mpirun_.empty())      Msg("  MPIRUN    : %s\n", mpirun_.c_str());
  if (!nodeargs_.empty())    Msg("  NODEARGS  : %s\n", nodeargs_.c_str());
  if (!account_.empty())     Msg("  ACCOUNT   : %s\n", account_.c_str());
  if (!email_.empty())       Msg("  EMAIL     : %s\n", email_.c_str());
  if (!queueName_.empty())   Msg("  QUEUE     : %s\n", queueName_.c_str());
  if (!modfileName_.empty()) Msg("  MODULEFILE: %s\n", modfileName_.c_str());
  Msg("  DEPEND    : %s\n", DependTypeStr[dependType_]);
}

void QueueOpts::CalcThreads() {
  if (threads_ < 1) {
    if (nodes_ > 0 || ppn_ > 0) {
      int N = 1, P = 1;
      if (nodes_ > 0) N = nodes_;
      if (ppn_ > 0) P = ppn_;
      threads_ = N * P;
    }
  }
  if (threads_ < 1)
    Msg("Warning: Less than 1 thread specified.\n");
}

void QueueOpts::AdditionalFlags(TextFile& qout) const {
  for (Sarray::const_iterator flag = Flags_.begin(); flag != Flags_.end(); ++flag)
    qout.Printf("#%s %s\n", QueueTypeStr[queueType_], flag->c_str());
}

int QueueOpts::QsubHeader(TextFile& qout, int run_num, std::string const& jobID,
                                  std::string const& namePrefix)
{
  std::string job_title, previous_job;
  if (run_num > -1)
    job_title = namePrefix + job_name_ + "." + integerToString(run_num);
  else
    job_title = namePrefix + job_name_;
  if (dependType_ == BATCH )
    previous_job = jobID;
  // Queue specific options.
  // ----- PBS -----------------------------------
  if (queueType_ == PBS) {
    std::string resources("nodes=" + integerToString(nodes_));
    if (ppn_ > 0)
      resources.append(":ppn=" + integerToString(ppn_));
    resources.append(nodeargs_);
    qout.Printf("#PBS -S /bin/bash\n#PBS -l walltime=%s,%s\n#PBS -N %s\n#PBS -j oe\n",
                walltime_.c_str(), resources.c_str(), job_title.c_str());
    if (!email_.empty()) qout.Printf("#PBS -m abe\n#PBS -M %s\n", email_.c_str()); 
    if (!account_.empty()) qout.Printf("#PBS -A %s\n", account_.c_str());
    if (!previous_job.empty()) qout.Printf("#PBS -W depend=afterok:%s\n", previous_job.c_str());  
    if (!queueName_.empty()) qout.Printf("#PBS -q %s\n", queueName_.c_str());
    AdditionalFlags( qout );
    qout.Printf("\ncd $PBS_O_WORKDIR\n\n");
  }
  // ----- SLURM ---------------------------------
  else if (queueType_ == SLURM) {
    qout.Printf("#!/bin/bash\n#SBATCH -J %s\n", job_title.c_str());
    if (nodes_ > 0) qout.Printf("#SBATCH -N %i\n", nodes_);
    qout.Printf("#SBATCH -t %s\n", walltime_.c_str());
    if (threads_ > 0) qout.Printf("#SBATCH -n %i\n", threads_);
    if (!email_.empty())
      qout.Printf("#SBATCH --mail-user=%s\n#SBATCH --mail-type=all\n", email_.c_str());
    if (!account_.empty())
      qout.Printf("#SBATCH -A %s\n", account_.c_str());
    if (!previous_job.empty()) qout.Printf("#SBATCH -d afterok:%s\n", previous_job.c_str());
    if (!queueName_.empty()) qout.Printf("#SBATCH -p %s\n", queueName_.c_str());
    AdditionalFlags( qout );
    qout.Printf("\necho \"JobID: $SLURM_JOB_ID\"\necho \"NodeList: $SLURM_NODELIST\"\n"
                "cd $SLURM_SUBMIT_DIR\n\n");
  }
  // Set thread info
  if (ppn_ > 0) qout.Printf("PPN=%i\n", ppn_);
  if (nodes_ > 0) qout.Printf("NODES=%i\n", nodes_);
  if (threads_ > 0) qout.Printf("THREADS=%i\n", threads_);
  // If AMBERHOME is set, set the EXE path
  if (!amberhome_.empty()) {
    qout.Printf("export AMBERHOME=%s\n", amberhome_.c_str());
    if (fileExists(amberhome_ + "/amber.sh"))
      qout.Printf("source $AMBERHOME/amber.sh\n");
    else
      qout.Printf("export PATH=$AMBERHOME/bin:$PATH\n"); // TODO LD_LIBRARY_PATH?
    // Check if program exists
    std::string exepath = amberhome_ + "/bin/" + program_;
    if (CheckExists("Full program path", exepath)) return 1;
    qout.Printf("export EXEPATH=%s\nls -l $EXEPATH\n", exepath.c_str());
  }
  // Add any module file commands
  if (!modfileName_.empty()) {
    Msg("  Reading module file %s\n", modfileName_.c_str());
    TextFile modFile;
    if (modFile.OpenRead( tildeExpansion(modfileName_) )) return 1;
    const char* ptr = modFile.Gets();
    while (ptr != 0) {
      qout.Printf("%s", ptr);
      ptr = modFile.Gets();
    }
    modFile.Close();
  }
  // Add any additional input
  if (!additionalCommands_.empty())
    qout.Printf("\n%s\n\n", additionalCommands_.c_str());
  qout.Printf("export MPIRUN=\"%s\"\n", mpirun_.c_str()); // TODO Combine with EXEPATH
  // Set EXE path here if AMBERHOME not set
  if (amberhome_.empty())
    qout.Printf("export EXEPATH=`which %s`\nls -l $EXEPATH\n", program_.c_str());

  return 0;
}
