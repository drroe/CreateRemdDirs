#include "Submitter.h"
#include "Messages.h"
#include "FileRoutines.h"
#include "TextFile.h"
#include "StringRoutines.h"
#include "CommonOptions.h"

using namespace Messages;
using namespace FileRoutines;
using namespace StringRoutines;

/** CONSTRUCTOR */
Submitter::Submitter() :
  debug_(0),
  nodes_(0),
  procs_(0),
  dependType_(NO_DEPENDS)
{}

/** KEEP IN SYNC WITH DependType */
const char* Submitter::DependTypeStr_[] = {
  "BATCH", "SUBMIT", "NONE"
};

/** Set debug level */
void Submitter::SetDebug(int debugIn) {
  debug_ = debugIn;
}

/** Print help to stdout. */
void Submitter::OptHelp() {
  Msg("Queue job submission input file variables:\n"
      "  INPUT_FILE <file>          : Read additional options from <file>.\n"
      "  JOBNAME <name>             : Job name in queueing system (required).\n"
      "  NODES <#>                  : Number of nodes needed.\n"
      "  PROCS <#>                  : Number of processes needed.\n"
      "                               Determined from NODES * PPN if not specified.\n"
      "  PROGRAM <name>             : Name of binary to run (required).\n"
      "  MPIRUN <command>           : Command used to execute parallel run. Can use\n"
      "                               $NODES, $THREADS, $PPN (will be set by script).\n"
      "  ACCOUNT <name>             : Account name\n"
      "  EMAIL <email>              : User email address\n"
      "  DEPEND {BATCH|SUBMIT|NONE} : Job dependencies. BATCH=Use batch system (default),\n"
      "                               SUBMIT=Execute next script at end of previous, or NONE.\n"
     );
  Queue::OptHelp();
  Msg("\n");
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
  } else if (OPT == "JOBNAME") {
    job_name_ = VAR;
  } else if (OPT == "NODES") {
    nodes_ = convertToInteger( VAR );
  } else if (OPT == "PROCS") {
    // NOTE: Previously was threads
    procs_ = convertToInteger( VAR );
  } else if (OPT == "PROGRAM") {
    program_ = VAR;
  } else if (OPT == "WALLTIME") {
    walltime_ = VAR;
  } else if (OPT == "ACCOUNT") {
    account_ = VAR;
  } else if (OPT == "EMAIL") {
    email_ = VAR;
  } else if (OPT == "MPIRUN") {
    mpirun_ = VAR;
  } else if (OPT == "DEPEND") {
    if (VAR == "BATCH")
      dependType_ = BATCH;
    else if (VAR == "SUBMIT")
      dependType_ = SUBMIT;
    else if (VAR == "NONE")
      dependType_ = NO_DEPENDS;
    else {
      ErrorMsg("Unrecognized variable %s for option %s\n", VAR.c_str(), OPT.c_str());
      return -1;
    }
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
        // See if this is a local queue option
        ret = localQueue_.ParseOption(opair->first, opair->second);
        if (ret == -1) {
          ErrorMsg("Could not parse queue option '%s' = '%s'\n", opair->first.c_str(), opair->second.c_str());

          return 1;
        } else if (ret == 0) {
          Msg("Warning: Ignoring unrecognized Submit option '%s' = '%s'\n", opair->first.c_str(), opair->second.c_str());
        }
      }
    }
  } // END loop over file options

  if (setupSubmitter()) {
    Msg("Warning: Invalid of missing options in file '%s'\n", input_file.c_str());
  }
  return 0;
}

/** Check that submitter is valid. */
int Submitter::setupSubmitter() {
  if (!localQueue_.IsValid()) {
    ErrorMsg("Invalid queue.\n");
    return 1;
  }
  // Set user if needed
  if (user_.empty())
    user_ = NoTrailingWhitespace( UserName() );
  return 0;
}

/** Print options to stdout. */
void Submitter::Info() const {
  Msg(  "  JOBNAME   : %s\n", job_name_.c_str());
  Msg(  "  USER      : %s\n", user_.c_str());
  Msg(  "  DEPEND    : %s\n", DependTypeStr_[dependType_]);
  Msg(  "  PROGRAM   : %s\n", program_.c_str());
  if (!mpirun_.empty())
    Msg("  MPIRUN    : %s\n", mpirun_.empty());
  if (nodes_ > 0)
    Msg("  NODES     : %i\n", nodes_);
  if (procs_ > 0)
    Msg("  PROCS     : %i\n", procs_);
  if (!walltime_.empty())
    Msg("  WALLTIME  : %s\n", walltime_.c_str());
  if (!email_.empty())
    Msg("  EMAIL     : %s\n", email_.c_str());
  if (!account_.empty())
    Msg("  ACCOUNT   : %s\n", account_.c_str());
  localQueue_.Info();
}

/** Submit job, set job id */
int Submitter::SubmitJob(std::string& jobid, std::string const& prev_jobidIn) const {
  // Ensure the run script exists
  std::string runScriptName = CommonOptions::Opt_RunScriptName().Val();
  if (!fileExists( runScriptName )) {
    ErrorMsg("Run script %s not found.\n");
    return 1;
  }
  // Create submit script name
  std::string submitScript( localQueue_.SubmitCmd() + ".sh" );
  Msg("DEBUG: submit script: %s\n", submitScript.c_str());
  return 0;
}
