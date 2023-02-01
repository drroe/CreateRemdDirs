#include "Submitter.h"
#include "Messages.h"
#include "FileRoutines.h"
#include "TextFile.h"

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

/** Parse option from a file.
  * \return 1 if option was parsed, 0 if ignored, -1 if error.
  */
int Submitter::ParseFileOption( OptArray::OptPair const& opair ) {
  std::string const& OPT = opair.first;
  std::string const& VAR = opair.second;
  //if (debug_ > 0) // FIXME
    Msg("    Option: %s  Variable: %s\n", OPT.c_str(), VAR.c_str());
  if (OPT == "INPUT_FILE") {
    Msg("Warning: INPUT_FILE is only processed when read from a Submit options file.\n");
  } else {
    return 0;
  }
  return 1;
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
  for (OptArray::const_iterator opair = Options.begin(); opair != Options.end(); ++opair)
  {
    if (opair->first == "INPUT_FILE") {
      // Try to prevent recursion
      std::string fn = tildeExpansion( opair->second );
      if (fn == fname) {
        ErrorMsg("An input file may not read from itself (%s)\n", opair->second.c_str());
        return 1;
      }
      if (ReadOptions( fn )) return 1;
    } else {
      int ret = ParseFileOption( *opair );
      if (ret == -1) {
        ErrorMsg("Could not parse option '%s' = '%s'\n", opair->first.c_str(), opair->second.c_str());
        return 1;
      } else if (ret == 0) {
        Msg("Warning: Ignoring unrecognized Submit option '%s' = '%s'\n", opair->first.c_str(), opair->second.c_str());
      }
    }
  } // END loop over file options
  return 0;
}
