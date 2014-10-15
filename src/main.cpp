#include <cstdlib> //atoi
#include "RemdDirs.h"
#include "Messages.h"
#include "FileRoutines.h"
#include "TextFile.h"

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
  enum RunModeType { CREATE=0, MD, ANALYZE, ARCHIVE };
  RunModeType runMode = CREATE;
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
    else if (Arg == "--analyze")
      runMode = ANALYZE;
    else if (Arg == "--archive")
      runMode = ARCHIVE;
    else {
      ErrorMsg("Unrecognized CMD line opt: %s\n", argv[iarg]);
      CmdLineHelp();
      return 1;
    }
  }

  // Read options from input file
  RemdDirs REMD;
  REMD.SetDebug(debug);
  if (REMD.ReadOptions( input_file )) return 1;

  // If no dimensions defined assume normal MD run.
  if (REMD.Ndims() == 0) {
    Msg("  No dimensions defined: assuming MD run.\n");
    runMode = MD;
    // If no input coords specified on input line, use coords from file.
    // First run only.
    if (start_run == 0 && crd_dir.empty())
      crd_dir = REMD.CrdFile();
  }

  // Write options
  Msg("  START            : %i\n", start_run);
  Msg("  STOP             : %i\n", stop_run);
  if (runMode == CREATE || runMode == MD) {
    Msg("  MDIN_FILE        : %s\n", REMD.mdin_file());
    Msg("  NSTLIM=%i, DT=%f\n",  REMD.Nstlim(), REMD.Dt());
    if (runMode == CREATE) {
      Msg("  NUMEXCHG=%i\n", REMD.Numexchg());
      Msg("  CRD_DIR          : %s\n", crd_dir.c_str());
      Msg("  %u dimensions :\n", REMD.Ndims());
      // Load dimensions
      if (REMD.LoadDimensions()) return 1;
    } else
      Msg("  CRD              : %s\n", crd_dir.c_str());
  }

  // Check options
  if (runMode == CREATE || runMode == MD) {
    if (crd_dir.empty()) {
      ErrorMsg("No starting coords directory/file specified.\n");
      return 1;
    }
    // If the COORDS dir exists at this point assume it is an absolute path
    // and perform tildeExpansion.
    if (fileExists(crd_dir))
      crd_dir = tildeExpansion( crd_dir );
    if (REMD.Nstlim() < 1 || (runMode == CREATE &&REMD.Numexchg() < 1)) {
      ErrorMsg("NSTLIM or NUMEXCHG < 1\n");
      return 1;
    }
    if (hasMdin && REMD.Mdin_File().empty()) {
      ErrorMsg("No MDIN_FILE specified and '--nomdin' not specified.\n");
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

  if (runMode == CREATE || runMode == MD) {
    Msg("Creating %i runs from %i to %i\n", stop_run - start_run + 1, start_run, stop_run);
    // Create runs
    REMD.SetCrdDir( crd_dir );
    for (int run = start_run; run <= stop_run; run++)
    {
      if (ChangeDir(TopDir)) return 1;
      // Determine run directory name, see if it is being overwritten.
      std::string RUNDIR = "run." + integerToString(run, runWidth);
      Msg("  RUNDIR: %s\n", RUNDIR.c_str());
      if (fileExists(RUNDIR) && !overwrite) {
        ErrorMsg("Directory '%s' exists and '-O' not specified.\n", RUNDIR.c_str());
        return 1;
      }
      // Create run input
      if (runMode == CREATE) {
        if (REMD.CreateRun(start_run, run, RUNDIR)) return 1;
      } else { // MD
        if (REMD.CreateMD(start_run, run, RUNDIR)) return 1;
      }
    }
  } else {
    // Ensure traj 1 for all runs between start and stop exist.
    for (int run = start_run; run <= stop_run; run++)
    {
      std::string TRAJ1 = "run." + integerToString(run, runWidth) +
                           "/TRAJ/rem.crd.001";
      if (CheckExists("Trajectory", TRAJ1)) return 1;
    }
    if (runMode == ANALYZE) {
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
      if (REMD.RunType() == "HREMD")
        TRAJINARGS.assign("nosort");
      CPPIN.Printf("parm %s\n", REMD.Topology().c_str());
      for (int run = start_run; run <= stop_run; run++)
        CPPIN.Printf("ensemble ../run.%0*i/TRAJ/rem.crd.001 %s\n", 
                     runWidth, run, TRAJINARGS.c_str());
      CPPIN.Printf("strip :WAT\nautoimage\n"
                   "trajout run%i-%i.nowat.nc netcdf remdtraj %s\n",
                   start_run, stop_run, REMD.TrajoutArgs().c_str());
      CPPIN.Close();
    } else if (runMode == ARCHIVE) {
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
      if (REMD.RunType() == "HREMD")
        TRAJINARGS.assign("nosort");
      std::string TOP = REMD.Topology();
      if ( REMD.FullArchive() != "NONE") {
        // Create input for fully archiving selected members of each run 
        for (int run = start_run; run <= stop_run; run++) {
          TextFile ARIN;
          if (ARIN.OpenWrite(ARDIR + "/ar1." + integerToString(run) + ".cpptraj.in")) return 1;
          std::string RUNDIR = "../run." + integerToString(run, runWidth);
          ARIN.Printf("parm %s\nensemble %s/TRAJ/rem.crd.001 %s\n"
                      "trajout %s/TRAJ/wat.nc netcdf remdtraj onlymembers %s\n",
                      TOP.c_str(), RUNDIR.c_str(), TRAJINARGS.c_str(),
                      RUNDIR.c_str(), REMD.FullArchive().c_str());
          ARIN.Close();
        }
      }
      // Create input for saving all stripped trajs
      for (int run = start_run; run <= stop_run; run++) {
        TextFile ARIN;
        if (ARIN.OpenWrite(ARDIR + "/ar2." + integerToString(run) + ".cpptraj.in")) return 1;
        std::string RUNDIR = "../run." + integerToString(run, runWidth);
        ARIN.Printf("parm %s\nensemble %s/TRAJ/rem.crd.001 %s\n"
                    "strip :WAT\nautoimage\ntrajout %s/TRAJ/nowat.nc netcdf remdtraj\n",
                    TOP.c_str(), RUNDIR.c_str(), TRAJINARGS.c_str(), RUNDIR.c_str());
        ARIN.Close();
      }
    } else {
      // Sanity check
      ErrorMsg("Unrecognized mode.\n");
      return 1;
    } 
  }
  Msg("\n");
  return 0;
}
