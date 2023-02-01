#include "Queue.h"
#include "Messages.h"
#include "StringRoutines.h"

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
const char* Queue::HeaderFlag() const {
  if (queueType_ == PBS) return "PBS";
  else if (queueType_ == SLURM) return "SBATCH";
  return 0;
}

/** \return Queue submit command. */
const char* Queue::SubmitCmd() const {
  if (queueType_ == PBS) return "qsub";
  else if (queueType_ == SLURM) return "sbatch";
  return 0;
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
  if (name_.empty()) {
    ErrorMsg("Queue is missing NAME.\n");
    return false;
  }
  if (ppn_ < 1) {
    ErrorMsg("Queue is missing PPN.\n");
    return false;
  }
  return true;
}

/** Print info to stdout. */
void Queue::Info() const {
  if (queueType_ == NO_QUEUE) return;
  if (!key_.empty())
    Msg("  KEY       : %s\n", key_.c_str());
  if (!name_.empty())
    Msg("  NAME      : %s\n", name_.c_str());
  if (ppn_ > 0)
    Msg("  PPN       : %i\n", ppn_);
  Msg(  "  TYPE      : %s\n", TypeStr_[queueType_]);
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
