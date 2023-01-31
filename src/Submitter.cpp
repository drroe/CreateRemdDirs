#include "Submitter.h"

/** CONSTRUCTOR */
Submitter::Submitter() :
  debug_(0),
  nodes_(0),
  procs_(0),
  dependType_(NO_DEPENDS)
{}

/** Set debug level */
void Submitter::SetDebug(int debugIn) {
  debug_ = debugIn;
}
