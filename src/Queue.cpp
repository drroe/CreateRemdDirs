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
  Msg("  PPN <#>            : Processors per node needed.\n");
}

/** Parse queue-specific option.
  * \return 1 if processed, 0 if ignored, -1 if error.
  */
int Queue::ParseOption(std::string const& OPT, std::string const& VAR)
{
  if (OPT == "PPN") {
    ppn_ = convertToInteger(VAR);
  } else {
    return 0;
  }
  return 1;
}
