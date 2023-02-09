#include "Queue.h"
#include "Messages.h"
#include "StringRoutines.h"
#include "TextFile.h"

using namespace Messages;
using namespace StringRoutines;

/** CONSTRUCTOR */
Queue::Queue() :
  ppn_(0),
  queueType_(NO_QUEUE)
{}

/** KEEP IN SYNC WITH Type */
const char* Queue::TypeStr_[] = {
  "PBS", "SLURM", "NONE"
};

/** \return Queue flag header text. */
std::string Queue::HeaderFlag() const {
  if (queueType_ == PBS) return "PBS";
  else if (queueType_ == SLURM) return "SBATCH";
  return "";
}

/** \return Queue submit command.
  * This is also used to name the job submit script.
  * NOTE: If additional names are added here, update CommonOptions::IsSubmitScript()
  */
std::string Queue::SubmitCmd() const {
  if (queueType_ == PBS) return "qsub";
  else if (queueType_ == SLURM) return "sbatch";
  return "run";
}

/** \return status string for given job id. */
Queue::JobStatType Queue::JobStatus(std::string const& jobid) const {
  JobStatType jstat = JOB_UNKNOWN;
  if (queueType_ == NO_QUEUE) return jstat;
  std::string cmd;
  if (queueType_ == SLURM)
    cmd.assign("squeue -h -o %T -j " + jobid);
  else {
    ErrorMsg("Internal Error: Queue::JobStatus not yet set up for PBS.\n");
    return ERROR;
  }
  TextFile shell;
  if (shell.OpenPipe( cmd )) return ERROR;
  std::string statString = shell.GetString();
  shell.Close();
  if (statString.empty())
    Msg("Warning: Empty status string from command '%s'\n", cmd.c_str());

  if (queueType_ == SLURM) {
    // states are PENDING, RUNNING, SUSPENDED, COMPLETING, and COMPLETED
    if (statString == "PENDING")
      jstat = QUEUED;
    else if (statString == "RUNNING")
      jstat = RUNNING;
    else if (!statString.empty())
      jstat = OTHER;
  }
  return jstat;
}

/** Print help to stdout */
void Queue::OptHelp() {
  Msg("  QUEUE <name>           : Queue/partition name.\n"
      "  PPN <#>                : Processors per node needed.\n"
      "  QTYPE {PBS|SLURM|NONE} : Batch queue system type.\n"
      "  FLAG <flag>            : Queue-specific flag(s).\n"
      "  COMMAND <cmd>          : Additional command(s) to run before execution.\n"
     );
}

/** Write queue options to file. */
int Queue::WriteQueueOpts(TextFile& outfile) const {
  if (!name_.empty())
    outfile.Printf("QUEUE %s\n", name_.c_str());
  if (ppn_ > 0)
    outfile.Printf("PPN %i\n", ppn_);
  if (queueType_ != NO_QUEUE)
    outfile.Printf("QTYPE %s\n", TypeStr_[queueType_]);
  for (Sarray::const_iterator it = Flags_.begin(); it != Flags_.end(); ++it)
    outfile.Printf("FLAG %s\n", it->c_str());
  for (Sarray::const_iterator it = additionalCommands_.begin();
                              it != additionalCommands_.end(); ++it)
    outfile.Printf("COMMAND %s\n", it->c_str());
  return 0;
}

/** Parse queue-specific option.
  * \return 1 if processed, 0 if ignored, -1 if error.
  */
int Queue::ParseOption(std::string const& OPT, std::string const& VAR)
{
  if (OPT == "QUEUE") {
    name_ = VAR;
  } else if (OPT == "PPN") {
    ppn_ = convertToInteger(VAR);
  } else if (OPT == "QSUB" || OPT == "QTYPE") {
    // QSUB for backwards compat.
    if (VAR == "PBS")
      queueType_ = PBS;
    else if (VAR == "SBATCH" || VAR == "SLURM")
      queueType_ = SLURM;
    else if (VAR == "NONE" || VAR == "none")
      queueType_ = NO_QUEUE;
    else {
      ErrorMsg("Unrecognized variable %s for option %s\n", VAR.c_str(), OPT.c_str());
      return -1;
    }
  } else if (OPT == "NODEARGS" || OPT == "FLAG") {
    Flags_.push_back( VAR );
  } else if (OPT == "COMMAND") {
    additionalCommands_.push_back( VAR );
  } else {
    return 0;
  }
  return 1;
}

/** \return True if queue has enough info for job submission. */
bool Queue::IsValid() const {
  // No queue is automatically valid
  if (queueType_ == NO_QUEUE) return true;
  // Need at least name and PPN
  int errcount = 0;
  if (name_.empty()) {
    ErrorMsg("Queue is missing name (QUEUE).\n");
    errcount++;
  }
  if (ppn_ < 1) {
    ErrorMsg("Queue is missing PPN.\n");
    errcount++;
  }
  return (errcount == 0);
}

/** Print info to stdout. */
void Queue::Info() const {
  if (queueType_ == NO_QUEUE) return;
  if (!key_.empty())
    Msg(  "  KEY       : %s\n", key_.c_str());
  Msg(    "  QTYPE     : %s\n", TypeStr_[queueType_]);
  if (queueType_ != NO_QUEUE) {
      Msg("  QUEUE     : %s\n", name_.c_str());
      Msg("  PPN       : %i\n", ppn_);
  }
  if (!additionalCommands_.empty()) {
    Msg("  Additional commands:\n");
    for (Sarray::const_iterator it = additionalCommands_.begin(); it != additionalCommands_.end(); ++it)
      Msg("\t%s\n", it->c_str());
  }
  if (!Flags_.empty()) {
    Msg("  Queue flags:\n");
    for (Sarray::const_iterator it = Flags_.begin(); it != Flags_.end(); ++it)
      Msg("\t%s\n", it->c_str());
  }
}

/** Write additional queue flags to file. */
int Queue::AdditionalFlags( TextFile& qout ) const {
  std::string Flag = HeaderFlag();
  for (Sarray::const_iterator it = Flags_.begin(); it != Flags_.end(); ++it)
    qout.Printf("#%s %s\n", Flag.c_str(), it->c_str());
  return 0;
}
