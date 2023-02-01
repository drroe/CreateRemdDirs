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

/** Print help to stdout */
void Queue::OptHelp() {
  Msg("  PPN <#>                : Processors per node needed.\n"
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
  if (OPT == "PPN") {
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
