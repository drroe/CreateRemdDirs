#include "Submitter.h"
#include "Messages.h"
#include "FileRoutines.h"
#include "TextFile.h"
#include "OptArray.h"

using namespace Messages;
using namespace FileRoutines;

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

/** Read queue/job options from a file. */
int Submitter::ReadOptions(std::string const& input_file) {
  // TODO clear previous options?
  // Read options from input file
  if (CheckExists("Submit options file", input_file)) return 1;
  std::string fname = tildeExpansion( input_file );
  Msg("Reading Submit options from file: %s\n", fname.c_str());
  TextFile infile;
  OptArray Options = infile.GetOptionsArray(fname, debug_);
  if (Options.empty()) return 1;

  return 0;
}
