#include <cstdlib> //atoi
#include "RemdDirs.h"
#include "CheckRuns.h"
#include "Submit.h"
#include "Messages.h"
#include "FileRoutines.h"
#include "StringRoutines.h"

static const char* VERSION = "0.96b";

static void CmdLineHelp() {
  Msg("Command line options:\n"
      "  -i <file>     : Creation input options file (remd.opts).\n"
      "  -q <file>     : Job queue submission options file (qsub.opts).\n"
      "  -b <start run>: Run # to start at.\n"
      "  -e <stop run> : Run # to end at.\n"
      "  -c <dir>      : Start coords directory (run creation only).\n"
      "  -s            : Allow job submission in addition to input creation.\n"
      "  -O            : Overwrite.\n"
      "  -t            : Test only; do not submit.\n"
      "  -h | --help   : Print this help.\n"
      "  --full-help   : Print extended help.\n"
      "  --create-help : Print run creation help.\n"
      "  --submit-help : Print run submission help.\n"
      "  --nomdin      : No extra MD input needed. Ignored if MDIN_FILE specified in input options file.\n"
      "  --analyze     : Enable analysis input creation/submit.\n"
      "  --archive     : Enable archiving input creation/submit.\n"
      "  --runs        : Enable run input creation/submit (default if nothing else specified).\n"
      "  --submit      : Submit jobs to queue only.\n"
      "  --check       : Check specified jobs only (requires NetCDF compilation).\n"
      "  --nocheck     : Do not check jobs before creating analyze/archive input.\n"
      "  --checkall    : When multiple replicas present, check all (default only first).\n\n");
}

static void Help(bool extended) {
  CmdLineHelp();
  if (extended) {
    RemdDirs::OptHelp();
    Submit::OptHelp();
  }
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
  Msg("\nCreateRemdDir: Amber run input creation/job submission/job check.\n");
  Msg("Version: %s\n", VERSION);
  Msg("Daniel R. Roe, 2017\n");
  enum ModeType { CREATE = 0, SUBMIT, CHECK };
  enum InputType { RUNS = 0, ANALYZE, ARCHIVE };
  std::vector<bool> ModeEnabled( 3, false );
  std::vector<bool> InputEnabled( 3, false );
  // Command line option defaults.
  std::string input_file = "remd.opts";
  int debug = 0;
  int start_run = -1;
  int stop_run = -1;
  std::string crd_dir;
  bool needsMdin = true;
  bool overwrite = false;
  bool checkFirst = true;
  bool runCheck = true;
  bool testOnly = false;
  std::string qfile = "qsub.opts";
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
    else if (Arg == "-h" || Arg == "--help") {    // Print help and exit
      Help(false);
      return 0;
    } else if (Arg == "--full-help") {            // Print extended help and exit
      Help(true);
      return 0;
    } else if (Arg == "--create-help") {
      RemdDirs::OptHelp();
      return 0;
    } else if (Arg == "--submit-help") {
      Submit::OptHelp();
      return 0;
    } else if (Arg == "-t" || Arg == "--test")    // Test, do not submit
      testOnly = true;
    else if (Arg == "-O")                         // Overwrite existing files
      overwrite = true;
    else if (Arg == "--nomdin")                   // Run does not need MDIN
      needsMdin = false;
    else if (Arg == "--nocheck")                  // Do not check for Analyze/Archive create
      runCheck = false;
    else if (Arg == "--runs")                     // Enable RUNS input
      InputEnabled[RUNS] = true;
    else if (Arg == "--analyze")                  // Enable ANALYZE input 
      InputEnabled[ANALYZE] = true;
    else if (Arg == "--archive")                  // Enable ARCHIVE input
      InputEnabled[ARCHIVE] = true; 
    else if (Arg == "--check") {                  // Enable CHECK mode only
      ModeEnabled[CHECK] = true;
      ModeEnabled[CREATE] = false;
      ModeEnabled[SUBMIT] = false;
    } else if (Arg == "--checkall")               // Check all replicas, not just first.
      checkFirst = false;
    else if (Arg == "-q" && iarg+1 != argc)       // SUBMIT input file
      qfile.assign( argv[++iarg] );
    else if (Arg == "--submit") {                 // Enable SUBMIT mode only
      ModeEnabled[SUBMIT] = true;
      ModeEnabled[CHECK] = false;
      ModeEnabled[CREATE] = false;
    } else if (Arg == "-s") {                     // Enable SUBMIT mode in addition to creation.
      ModeEnabled[CREATE] = true;
      ModeEnabled[SUBMIT] = true;
    } else {
      ErrorMsg("Unrecognized CMD line opt: %s\n", argv[iarg]);
      CmdLineHelp();
      return 1;
    }
  }
  if (stop_run == -1)
    stop_run = start_run;
  // By default enable CREATE Mode and RUNS Input
  if (!ModeEnabled[CREATE] && !ModeEnabled[SUBMIT] && !ModeEnabled[CHECK])
    ModeEnabled[CREATE] = true;
  if (!InputEnabled[RUNS] && !InputEnabled[ANALYZE] && !InputEnabled[ARCHIVE])
    InputEnabled[RUNS] = true;

  // Write options
  Msg("  START            : %i\n", start_run);
  Msg("  STOP             : %i\n", stop_run);
  // Check options
  if (start_run < 0 ) {
    ErrorMsg("Negative value for START_RUN\n");
    return 1;
  }
  if (stop_run < start_run) {
    ErrorMsg("STOP_RUN < START_RUN\n");
    return 1;
  }
  std::string TopDir = GetWorkingDir();
  if (TopDir.empty()) return 1;
  Msg("Working Dir: %s\n", TopDir.c_str());
  // Create array of run directories
  int runWidth = std::max( DigitWidth(stop_run), 3 );
  StrArray RunDirs;
  for (int run = start_run; run <= stop_run; ++run)
    RunDirs.push_back( "run." + integerToString(run, runWidth) );

  // ----- Input Creation ------------------------
  if (ModeEnabled[CREATE]) {
    // Read RUN options from input file
    RemdDirs create;
    create.SetDebug(debug);
    if (create.ReadOptions( input_file, start_run )) return 1;
    // Setup run
    if (create.Setup( crd_dir, needsMdin )) return 1;
    create.Info();
    // Input for Runs
    if (InputEnabled[RUNS]) {
      Msg("Creating %i runs from %i to %i\n", stop_run - start_run + 1, start_run, stop_run);
      if (create.CreateRuns(TopDir, RunDirs, start_run, overwrite)) return 1;
    }
    // If analysis or archive input requested, run check unless explicitly told not to.
    if (InputEnabled[ANALYZE] || InputEnabled[ARCHIVE]) {
      if (runCheck) {
        if (CheckRuns( TopDir, RunDirs, checkFirst )) return 1;
      } else
        Msg("Warning: Not running check on run directories.\n");
      create.CreateAnalyzeArchive(TopDir, RunDirs, start_run, stop_run, overwrite, runCheck,
                                  InputEnabled[ANALYZE], InputEnabled[ARCHIVE]);
    }
  }
  // ----- Run Check -----------------------------
  if (ModeEnabled[CHECK]) {
    if (CheckRuns( TopDir, RunDirs, checkFirst )) return 1;
  }
  // ----- Job submission ------------------------
  if (ModeEnabled[SUBMIT]) {
    ChangeDir( TopDir );
    Submit submit;
    submit.SetDebug(debug);
    submit.SetTesting( testOnly );
    std::string defaultName("~/default.qsub.opts");
    if (fileExists(defaultName)) {
      if (submit.ReadOptions(defaultName)) return 1;
    }
    if (submit.ReadOptions( qfile )) return 1;
    if (submit.CheckOptions()) return 1;
    if (InputEnabled[RUNS]) {
      if (submit.SubmitRuns(TopDir, RunDirs, start_run, overwrite)) return 1;
    }
    if (InputEnabled[ANALYZE]) {
      if (submit.SubmitAnalysis(TopDir, start_run, stop_run, overwrite)) return 1;
    }
    if (InputEnabled[ARCHIVE]) {
      if (submit.SubmitArchive(TopDir, start_run, stop_run, overwrite)) return 1;
    }
  }

  Msg("\n");
  return 0;
}
