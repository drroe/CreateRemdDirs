#include "Submitter.h"
#include "Messages.h"

using namespace Messages;

/** CONSTRUCTOR */
Submitter::Submitter() :
  debug_(0),
  nodes_(0),
  procs_(0),
  dependType_(NO_DEPENDS)
{}

/** Print help to stdout. */
void Submitter::OptHelp() {
  Msg("\n");
}

/** Set debug level */
void Submitter::SetDebug(int debugIn) {
  debug_ = debugIn;
}
