#include <cstdlib> //atoi
#include "RemdDirs.h"
#include "Messages.h"
#include "FileRoutines.h"
#include "StringRoutines.h"
#include "CheckRuns.h"

static void CmdLineHelp() {
  Msg("Command line options:\n"
      "  -i <input file> (remd.opts)\n"
      "  -b <start run>\n"
      "  -e <stop run>\n"
      "  -c <start coords dir>\n"
      "  -O : Overwrite\n"
      "  --nomdin (no extra MD input, overridden by MDIN_FILE)\n"
      "  --analyze (no need for -c)\n"
      "  --archive (no need for -c)\n\n");
}

static void Help() {
  CmdLineHelp();
  RemdDirs::OptHelp();
}

// =============================================================================
int main(int argc, char** argv) {
  std::string input_file = "remd.opts";
  int debug = 0;
  int start_run = -1;
  int stop_run = -1;
  std::string crd_dir;
  bool hasMdin = true;
  bool overwrite = false;
  enum RunType { REPLICA=0, MD };
  enum ModeType { CREATE=0, ANALYZE_ARCHIVE, CHECK };
  bool analyzeEnabled = false;
  bool archiveEnabled = false;
  bool checkFirst = true;
  RunType runType = REPLICA;
  ModeType modeType = CREATE;
  // Get command line options
  for (int iarg = 1; iarg < argc; iarg++) {
    std::string Arg( argv[iarg] );
    if (Arg == "-i" && iarg+1 != argc)
      input_file.assign( argv[++iarg] );
    else if (Arg == "-b" && iarg+1 != argc)
      start_run = atoi(argv[++iarg]);
    else if (Arg == "-e" && iarg+1 != argc)
      stop_run = atoi(argv[++iarg]);
    else if (Arg == "-c" && iarg+1 != argc)
      crd_dir.assign( argv[++iarg] );
    else if (Arg == "-d" && iarg+1 != argc)
      debug = atoi(argv[++iarg]);
    else if (Arg == "-h" || Arg == "--help") {
      Help();
      return 0;
    } else if (Arg == "-O")
      overwrite = true;
    else if (Arg == "--nomdin")
      hasMdin = false;
    else if (Arg == "--analyze") {
      modeType = ANALYZE_ARCHIVE;
      analyzeEnabled = true;
    } else if (Arg == "--archive") {
      modeType = ANALYZE_ARCHIVE;
      archiveEnabled = true;
    } else if (Arg == "--check")
      modeType = CHECK;
    else if (Arg == "--checkall") {
      modeType = CHECK;
      checkFirst = false;
    } else {
      ErrorMsg("Unrecognized CMD line opt: %s\n", argv[iarg]);
      CmdLineHelp();
      return 1;
    }
  }
  if (stop_run == -1)
    stop_run = start_run;

  // Read options from input file
  RemdDirs REMD;
  REMD.SetDebug(debug);
  if (modeType != CHECK) {
    if (REMD.ReadOptions( input_file )) return 1;
    // If no dimensions defined assume normal MD run.
    if (REMD.Ndims() == 0) {
      Msg("  No dimensions defined: assuming MD run.\n");
      runType = MD;
      // If no input coords specified on input line, use coords from file.
      // First run only.
      if (start_run == 0 && crd_dir.empty())
        crd_dir = REMD.CrdFile();
    }
  }

  // Write options
  Msg("  START            : %i\n", start_run);
  Msg("  STOP             : %i\n", stop_run);
  if (modeType == CREATE) {
    Msg("  MDIN_FILE        : %s\n", REMD.mdin_file());
    Msg("  NSTLIM=%i, DT=%f\n",  REMD.Nstlim(), REMD.Dt());
    if (runType == REPLICA) {
      Msg("  NUMEXCHG=%i\n", REMD.Numexchg());
      Msg("  CRD_DIR          : %s\n", crd_dir.c_str());
    } else // MD
      Msg("  CRD              : %s\n", crd_dir.c_str());
  }
  // Load dimensions for REMD runs only.
  if (modeType != CHECK && runType != MD) {
    Msg("  %u dimensions :\n", REMD.Ndims());
    if (REMD.LoadDimensions()) return 1;
  }

  // Check options
  if (modeType == CREATE) {
    if (crd_dir.empty()) {
      ErrorMsg("No starting coords directory/file specified.\n");
      return 1;
    }
    // If the COORDS dir exists at this point assume it is an absolute path
    // and perform tildeExpansion.
    if (fileExists(crd_dir))
      crd_dir = tildeExpansion( crd_dir );
    if (REMD.Nstlim() < 1 || (runType == REPLICA && REMD.Numexchg() < 1)) {
      ErrorMsg("NSTLIM or NUMEXCHG < 1\n");
      return 1;
    }
    if (hasMdin && REMD.Mdin_File().empty()) {
      ErrorMsg("No MDIN_FILE specified and '--nomdin' not specified.\n");
      return 1;
    }
    if (REMD.UmbrellaFreq() > 0 && REMD.N_MD_Runs() < 2) {
      ErrorMsg("If UMBRELLA is specified MDRUNS must be > 1.\n");
      return 1;
    }
  }
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
  int runWidth = std::max( DigitWidth(stop_run), 3 );
  // Create array of run directories
  StrArray RunDirs;
  for (int run = start_run; run <= stop_run; ++run)
    RunDirs.push_back( "run." + integerToString(run, runWidth) );

  if (modeType == CHECK) {
    if (CheckRuns( TopDir, RunDirs, checkFirst )) return 1;
  } else if (modeType == CREATE) {
    Msg("Creating %i runs from %i to %i\n", stop_run - start_run + 1, start_run, stop_run);
    // Create runs
    REMD.SetCrdDir( crd_dir );
    for (int run = start_run; run <= stop_run; run++)
    {
      if (ChangeDir(TopDir)) return 1;
      // Determine run directory name, see if it is being overwritten.
      Msg("  RUNDIR: %s\n", RunDirs[run].c_str());
      if (fileExists(RunDirs[run]) && !overwrite) {
        ErrorMsg("Directory '%s' exists and '-O' not specified.\n", RunDirs[run].c_str());
        return 1;
      }
      // Create run input
      if (runType == REPLICA) {
        if (REMD.CreateRun(start_run, run, RunDirs[run])) return 1;
      } else { // MD
        if (REMD.CreateMD(start_run, run, RunDirs[run])) return 1;
      }
    }
  } else if (modeType == ANALYZE_ARCHIVE) {
    Msg("Creating input for");
    if (analyzeEnabled) Msg(" analysis");
    if (archiveEnabled) Msg(" archive");
    Msg("\n");
    ChangeDir( RunDirs.front() );
    StrArray TrajFiles;
    if (runType == REPLICA)
      TrajFiles = ExpandToFilenames("TRAJ/rem.crd.*");
    else // MD
      TrajFiles = ExpandToFilenames("md.nc.*");
    if (TrajFiles.empty()) {
      ErrorMsg("No trajectory files found.\n");
      return 1;
    }
    std::string traj_prefix("/" + TrajFiles.front());
    ChangeDir( TopDir );
    // Ensure traj 1 for all runs between start and stop exist.
    for (StrArray::const_iterator rdir = RunDirs.begin(); rdir != RunDirs.end(); ++rdir)
    {
      std::string TRAJ1(*rdir + traj_prefix);
      if (CheckExists("Trajectory", TRAJ1)) return 1;
    }
    if (CheckRuns( TopDir, RunDirs, checkFirst )) return 1;
    // -------------------------------------------
    if (analyzeEnabled) {
      // Set up input for analysis
      std::string CPPDIR = "Analyze." + integerToString(start_run) + "." + 
                                        integerToString(stop_run);
      if ( fileExists(CPPDIR) ) {
        if (!overwrite) {
          ErrorMsg("Directory '%s' exists and '-O' not specified.\n", CPPDIR.c_str());
          return 1;
        }
      } else
        Mkdir( CPPDIR );
      TextFile CPPIN;
      if (CPPIN.OpenWrite( CPPDIR + "/batch.cpptraj.in" )) return 1;
      std::string TRAJINARGS;
      // If HREMD, need nosort keyword
      if (REMD.RunType().empty() || REMD.RunType() == "HREMD")
        TRAJINARGS.assign("nosort");
      CPPIN.Printf("parm %s\n", REMD.Topology().c_str());
      for (int run = start_run; run <= stop_run; run++)
        CPPIN.Printf("ensemble ../run.%0*i%s %s\n", 
                     runWidth, run, traj_prefix.c_str(), TRAJINARGS.c_str());
      CPPIN.Printf("strip :WAT\nautoimage\n"
                   "trajout run%i-%i.nowat.nc netcdf remdtraj %s\n",
                   start_run, stop_run, REMD.TrajoutArgs().c_str());
      CPPIN.Close();
    }
    // -------------------------------------------
    if (archiveEnabled) {
      // Set up input for archiving. This will be done in 2 separate runs. 
      // The first sorts and saves fully solvated trajectories of interest
      // (FULLARCHIVE). The second saves all stripped trajs.
      if (REMD.FullArchive().empty()) {
        ErrorMsg("FULLARCHIVE must contain a comma-separated list of ensemble members to"
                 " save full coordinates, or NONE to skip this step.\n");
        return 1;
      }
      std::string ARDIR="Archive." + integerToString(start_run) + "." +
                                     integerToString(stop_run);
      if ( fileExists(ARDIR) ) {
        if (!overwrite) {
          ErrorMsg("Directory '%s' exists and '-O' not specified.\n", ARDIR.c_str());
          return 1;
        }
      } else
        Mkdir( ARDIR );
      std::string TRAJINARGS;
      // If HREMD, need nosort keyword
      if (REMD.RunType().empty() || REMD.RunType() == "HREMD")
        TRAJINARGS.assign("nosort");
      std::string TOP = REMD.Topology();
      if ( REMD.FullArchive() != "NONE") {
        // Create input for fully archiving selected members of each run 
        for (int run = start_run; run <= stop_run; run++) {
          TextFile ARIN;
          if (ARIN.OpenWrite(ARDIR + "/ar1." + integerToString(run) + ".cpptraj.in")) return 1;
          std::string RUNDIR = "../run." + integerToString(run, runWidth);
          ARIN.Printf("parm %s\nensemble %s%s %s\n"
                      "trajout %s/TRAJ/wat.nc netcdf remdtraj onlymembers %s\n",
                      TOP.c_str(), RUNDIR.c_str(), traj_prefix.c_str(), TRAJINARGS.c_str(),
                      RUNDIR.c_str(), REMD.FullArchive().c_str());
          ARIN.Close();
        }
      }
      // Create input for saving all stripped trajs
      for (int run = start_run; run <= stop_run; run++) {
        TextFile ARIN;
        if (ARIN.OpenWrite(ARDIR + "/ar2." + integerToString(run) + ".cpptraj.in")) return 1;
        std::string RUNDIR = "../run." + integerToString(run, runWidth);
        ARIN.Printf("parm %s\nensemble %s%s %s\n"
                    "strip :WAT\nautoimage\ntrajout %s/TRAJ/nowat.nc netcdf remdtraj\n",
                    TOP.c_str(), RUNDIR.c_str(), traj_prefix.c_str(), TRAJINARGS.c_str(), RUNDIR.c_str());
        ARIN.Close();
      }
    } 
  }
  Msg("\n");
  return 0;
}
