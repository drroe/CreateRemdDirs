#include <cstdlib> //atoi
#include "Messages.h"
#include "FileRoutines.h"
#include "StringRoutines.h"
#include "Manager.h"
#include "Commands.h"

using namespace Messages;
using namespace FileRoutines;

static const char* VERSION = "2.00a";

static void CmdLineHelp() {
  Msg("Command line options:\n"
      "  -i <file>     : Creation input options file (remd.opts).\n"
      "  -q <file>     : Job queue submission options file (qsub.opts).\n"
      "  -b <start run>: Run # to start at.\n"
      "  -e <stop run> : Run # to end at.\n"
      "  -c <dir>      : Start coords directory (run creation only).\n"
      "  -j <id>       : Have first run depend on given job id (run submission only).\n"
      "  -s            : Allow job submission in addition to input creation.\n"
      "  -O            : Overwrite.\n"
      "  -t            : Test only; do not submit.\n"
      "  -h | --help   : Print this help.\n"
      "  --full-help   : Print extended help.\n"
      "  --create-help : Print run creation help.\n"
      "  --submit-help : Print run submission help.\n"
      "  --submit      : Submit jobs to queue only.\n"
     );
}

static void Help(bool extended) {
  CmdLineHelp();
  if (extended) {
    Creator::OptHelp();
    Submitter::OptHelp();
  }
}

static void Defines() {
  Msg("Defines:");
# ifdef HAS_NETCDF
  Msg(" -DHAS_NETCDF");
# endif
  Msg("\n");
}

// =============================================================================
/** There are three types of modes available:
  * 1) Creation: Input is created for MD runs, analysis, and/or archiving.
  * 2) Submission: Jobs are submitted for MD runs, analysis, and/or archiving.
  * 3) Check: MD runs that have already run are checked. A check is also 
  *    performed when input is created for analysis or archiving unless
  *     disabled.
  * For now make all modes mutually exclusive.
  */
int main(int argc, char** argv) {
  Msg("\nMD Director: MD run input creation/job submission/job checking.\n");
  Msg("Version: %s\n", VERSION);
  Msg("Daniel R. Roe, 2023\n");
  // Command line options
  std::string input_file;
  int debug = 0;
  int start_run = -1;
  int stop_run = -1;
  std::string crd_dir;
  std::string previous_jobid;
  bool overwrite = false;
  bool testOnly = false;
  std::string qfile = "qsub.opts";
  std::string systems_file = "systems.opts";
  // Get command line options
  for (int iarg = 1; iarg < argc; iarg++) {
    std::string Arg( argv[iarg] );
    if (Arg == "-i" && iarg+1 != argc)            // Input file for CREATE 
      input_file.assign( argv[++iarg] );
    else if (Arg == "-b" && iarg+1 != argc)       // Begin run
      start_run = atoi(argv[++iarg]);
    else if (Arg == "-e" && iarg+1 != argc)       // End run
      stop_run = atoi(argv[++iarg]);
    else if (Arg == "-c" && iarg+1 != argc)       // Run start coordinates
      crd_dir.assign( argv[++iarg] );
    else if (Arg == "-d" && iarg+1 != argc)       // Debug level
      debug = atoi(argv[++iarg]);
    else if (Arg == "-j" && iarg+1 != argc)       // Previous job id
      previous_jobid.assign(argv[++iarg]);
    else if (Arg == "-h" || Arg == "--help") {    // Print help and exit
      Help(false);
      return 0;
    } else if (Arg == "--full-help") {            // Print extended help and exit
      Help(true);
      return 0;
    } else if (Arg == "--create-help") {
      Creator::OptHelp();
      return 0;
    } else if (Arg == "--submit-help") {
      Submitter::OptHelp();
      return 0;
    } else if (Arg == "-t" || Arg == "--test")    // Test, do not submit
      testOnly = true;
    else if (Arg == "-O")                         // Overwrite existing files
      overwrite = true;
//    else if (Arg == "--nomdin")                   // Run does not need MDIN
//      needsMdin = false;
//    else if (Arg == "--nocheck")                  // Do not check for Analyze/Archive create
//      runCheck = false;
//    else if (Arg == "--runs")                     // Enable RUNS input
//      InputEnabled[RUNS] = true;
//    else if (Arg == "--analyze")                  // Enable ANALYZE input 
//      InputEnabled[ANALYZE] = true;
//    else if (Arg == "--archive")                  // Enable ARCHIVE input
//      InputEnabled[ARCHIVE] = true; 
//    else if (Arg == "--check") {                  // Enable CHECK mode only
//      ModeEnabled[CHECK] = true;
//      ModeEnabled[CREATE] = false;
//      ModeEnabled[SUBMIT] = false;
//    } else if (Arg == "--checkall")               // Check all replicas, not just first.
//      checkFirst = false;
    else if (Arg == "-q" && iarg+1 != argc) {       // SUBMIT input file
      qfile.assign( argv[++iarg] );
//    } else if (Arg == "--submit") {                 // Enable SUBMIT mode only
//      ModeEnabled[SUBMIT] = true;
//      ModeEnabled[CHECK] = false;
//      ModeEnabled[CREATE] = false;
//    } else if (Arg == "-s") {                     // Enable SUBMIT mode in addition to creation.
//      ModeEnabled[CREATE] = true;
//      ModeEnabled[SUBMIT] = true;
    } else if (Arg == "--defines") {
      Defines();
      return 0;
    } else if (Arg == "-v" || Arg == "--version") {
      return 0;
    } else {
      ErrorMsg("Unrecognized CMD line opt: %s\n", argv[iarg]);
      CmdLineHelp();
      return 1;
    }
  }

  // Get current directory
  std::string TopDir = GetWorkingDir();
  if (TopDir.empty()) {
    ErrorMsg("Could not get current directory name.\n");
    return 1;
  }

  // Start Manager mode.
  Commands::InitCommands();
  Manager manager;
  manager.SetDebug( debug );
  if (manager.InitManager( TopDir, systems_file )) return 1;
  if (!input_file.empty()) {
    Exec::RetType ret = Commands::ReadInput(input_file, manager);
    if (ret == Exec::QUIT) return 0;
    if (ret == Exec::ERR)
      Msg("Warning: Errors encountered while reading input file '%s'\n", input_file.c_str());
  }
  int nerr = Commands::Prompt(manager);

  Msg("\n");
  return nerr;
}
