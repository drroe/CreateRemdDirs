#include <cstring> // strstr
#include <cstdlib> // atoi, atof
#include "RemdDirs.h"
#include "Messages.h"
#include "TextFile.h"
#include "StringRoutines.h"

RemdDirs::RemdDirs() : nstlim_(-1), ig_(-1), numexchg_(-1),
   dt_(-1.0), temp0_(-1.0), totalReplicas_(0), top_dim_(-1),
   temp0_dim_(-1), debug_(0), n_md_runs_(0), umbrella_(0),
   override_irest_(false), override_ntx_(false)
{}

// DESTRUCTOR
RemdDirs::~RemdDirs() {
  for (DimArray::const_iterator dim = Dims_.begin(); dim != Dims_.end(); ++dim)
    if (*dim != 0) delete *dim;
}

void RemdDirs::OptHelp() {
  Msg("Creation input file variables:\n"
      "  CRD_FILE <dir>     : Starting coordinates location (run 0 only).\n"
      "  DIMENSION <file>   : File containing replica dimension information, 1 per dimension\n"
      "  TRAJOUTARGS <args> : Additional trajectory output args for analysis (--analyze).\n"
      "  FULLARCHIVE <arg>  : Comma-separated list of members to fully archive or NONE.\n"
      "  TOPOLOGY <file>    : Topology for 1D TREMD run.\n"
      "  MDIN_FILE <file>   : File containing extra MDIN input.\n"
      "  RST_FILE <file>    : File containing NMR restraints (MD only).\n"
      "  TEMPERATURE <T>    : Temperature for 1D HREMD run.\n"
      "  NSTLIM <nstlim>    : Input file; steps per exchange. Required.\n"
      "  DT <step>          : Input file; time step. Required.\n"
      "  IG <seed>          : Input file; random seed.\n"
      "  NUMEXCHG <#>       : Input file; number of exchanges. Required for REMD.\n"
      "  MDRUNS <#>         : Number of MD runs when not REMD (default 1).\n"
      "  UMBRELLA <#>       : Indicates MD umbrella sampling with write frequency <#>.\n\n");
}

// RemdDirs::ReadOptions()
int RemdDirs::ReadOptions(std::string const& input_file, int start) {
  // Read options from input file
  if (CheckExists("Input file", input_file)) return 1;
  std::string fname = tildeExpansion( input_file );
  Msg("Reading input from file: %s\n", fname.c_str());
  TextFile infile;
  TextFile::OptArray Options = infile.GetOptionsArray(fname, debug_);
  if (Options.empty()) return 1;
  for (TextFile::OptArray::const_iterator opair = Options.begin(); opair != Options.end(); ++opair)
  {
    std::string const& OPT = opair->first;
    std::string const& VAR = opair->second;
      if (debug_ > 0)
        Msg("    Option: %s  Variable: %s\n", OPT.c_str(), VAR.c_str());
      if      (OPT == "CRD_FILE") {
        if (start != 0)
          Msg("Warning: CRD_FILE only used if start run is 0. Skipping.\n");
        else
          crd_dir_ = VAR;
      }
      else if (OPT == "DIMENSION")
      {
        if (CheckExists("Dimension file", VAR)) { return 1; }
        if (LoadDimension( tildeExpansion(VAR) )) { return 1; }
      }
      else if (OPT == "MDRUNS")
        n_md_runs_ = atoi( VAR.c_str() );
      else if (OPT == "NSTLIM")
        nstlim_ = atoi( VAR.c_str() );
      else if (OPT == "DT")
        dt_ = atof( VAR.c_str() );
      else if (OPT == "IG")
        ig_ = atoi( VAR.c_str() );
      else if (OPT == "NUMEXCHG")
        numexchg_ = atoi( VAR.c_str() );
      else if (OPT == "UMBRELLA")
        umbrella_ = atoi( VAR.c_str() );
      else if (OPT == "TOPOLOGY")
      {
        top_file_ = VAR;
        // If the TOPOLOGY exists at this point assume it is an absolute path
        // and perform tildeExpansion.
        if (fileExists(top_file_))
          top_file_ = tildeExpansion( top_file_ );
      }
      else if (OPT == "TEMPERATURE")
        temp0_ = atof( VAR.c_str() );
      else if (OPT == "TRAJOUTARGS")
        trajoutargs_ = VAR;
      else if (OPT == "FULLARCHIVE")
        fullarchive_ = VAR;
      else if (OPT == "MDIN_FILE")
      {
        if (CheckExists("MDIN file", VAR)) { return 1; }
        mdin_file_ = tildeExpansion( VAR );
      }
      else if (OPT == "RST_FILE")
      {
        rst_file_ = VAR;
        if (fileExists(rst_file_))
          rst_file_ = tildeExpansion( rst_file_ );
      }
      else
      {
        ErrorMsg("Unrecognized option '%s' in input file.\n", OPT.c_str());
        OptHelp();
        return 1;
      }
  }
  // If MDIN file specified, store it in a string.
  override_irest_ = false;
  override_ntx_ = false;
  additionalInput_.clear();
  if (!mdin_file_.empty()) {
    TextFile MDIN;
    if (MDIN.OpenRead(mdin_file_)) return 1;
    const char* buffer;
    while ( (buffer = MDIN.Gets()) != 0) {
      if (strstr(buffer, "irest ") != 0 || strstr(buffer, "irest=") != 0) {
        Msg("Warning: Using 'irest' in '%s'\n", mdin_file_.c_str());
        override_irest_ = true;
      }
      if (strstr(buffer, "ntx ") != 0 || strstr(buffer, "ntx=") != 0) {
        Msg("Warning: Using 'ntx' in '%s'\n", mdin_file_.c_str());
        override_ntx_ = true;
      }
      additionalInput_.append( buffer );
    }
    MDIN.Close();
    if (override_irest_ != override_ntx_) {
      ErrorMsg("Both 'irest' and 'ntx' must be in '%s' if either are.\n", mdin_file_.c_str());
      return 1;
    }
  }
  return 0;
}

// RemdDirs::LoadDimension()
int RemdDirs::LoadDimension(std::string const& dfile) {
  TextFile infile;
  // File existence already checked.
  if (infile.OpenRead(dfile)) return 1;
  // Determine dimension type from first line.
  std::string firstLine = infile.GetString();
  if (firstLine.empty()) {
    ErrorMsg("Could not read first line of dimension file '%s'\n", dfile.c_str());
    return 1;
  }
  infile.Close(); 
  // Allocate proper dimension type and load.
  ReplicaDimension* dim = ReplicaAllocator::Allocate( firstLine );
  if (dim == 0) {
    ErrorMsg("Unrecognized dimension type: %s\n", firstLine.c_str());
    return 2;
  }
  // Push it here so it will be deallocated if there is an error
  Dims_.push_back( dim ); 
  if (dim->LoadDim( dfile )) {
    ErrorMsg("Loading info from dimension file '%s'\n", dfile.c_str());
    return 1;
  }
  Msg("    Dim %u: %s (%u)\n", Dims_.size(), dim->description(), dim->Size());
  return 0;
}

// RemdDirs::Setup()
int RemdDirs::Setup(std::string const& crdDirIn, bool needsMdin) {
  // Command line input coordinates override any in options file.
  if (!crdDirIn.empty())
    crd_dir_.assign(crdDirIn);
  // Perform tilde expansion on coords if necessary.
  if (crd_dir_[0] == '~')
    crd_dir_ = tildeExpansion(crd_dir_);
  // Figure out what type of run this is.
  runDescription_.clear();
  if (Dims_.empty()) {
    Msg("  No dimensions defined: assuming MD run.\n");
    runType_ = MD;
    runDescription_.assign("MD");
  } else {
    if (Dims_.size() == 1) {
      if (Dims_[0]->Type() == ReplicaDimension::TEMP ||
          Dims_[0]->Type() == ReplicaDimension::SGLD)
        runType_ = TREMD;
      else
        runType_ = HREMD;
      runDescription_.assign( Dims_[0]->name() );
    } else {
      runType_ = MREMD;
      DimArray::const_iterator dim = Dims_.begin();
      runDescription_.assign( "MREMD" );
    }
    // Count total # of replicas, Do some error checking.
    totalReplicas_ = 1;
    temp0_dim_ = -1;
    top_dim_ = -1;
    int providesTemp0 = 0;
    int providesTopFiles = 0;
    for (DimArray::const_iterator dim = Dims_.begin(); dim != Dims_.end(); ++dim)
    {
      totalReplicas_ *= (*dim)->Size();
      if ((*dim)->ProvidesTemp0()) {
        temp0_dim_ = (int)(dim - Dims_.begin());
        providesTemp0++;
      }
      if ((*dim)->ProvidesTopFiles()) {
        top_dim_ = (int)(dim - Dims_.begin());
        providesTopFiles++;
      }
    }
    if (providesTemp0 > 1) {
      ErrorMsg("At most one dimension that provides temperatures should be specified.\n");
      return 1;
    } else if (providesTemp0 == 0 && temp0_ < 0.0) {
      ErrorMsg("No dimension provides temperature and TEMPERATURE not specified.\n");
      return 1;
    }
    if (providesTopFiles > 1) {
      ErrorMsg("At most one dimension that provides topology files should be specified.\n");
      return 1;
    } else if (providesTopFiles == 0 && top_file_.empty()) {
      ErrorMsg("No dimension provides topology files and TOPOLOGY not specified.\n");
      return 1;
    }
    if (debug_ > 0)
      Msg("    Topology dimension: %i\n    Temp0 dimension: %i\n", top_dim_, temp0_dim_);
  }
  // Perform some more error checking
  if (nstlim_ < 1 || (runType_ != MD && numexchg_ < 1)) {
    ErrorMsg("NSTLIM or NUMEXCHG < 1\n");
    return 1;
  }
  if (needsMdin && mdin_file_.empty()) {
    ErrorMsg("No MDIN_FILE specified and '--nomdin' not specified.\n");
    return 1;
  }
  if (umbrella_ > 0 && n_md_runs_ < 2) {
    ErrorMsg("If UMBRELLA is specified MDRUNS must be > 1.\n");
    return 1;
  }

  return 0;
}

// RemdDirs::Info()
void RemdDirs::Info() const {
  Msg("  MDIN_FILE        : %s\n", mdin_file_.c_str());
  Msg("  NSTLIM=%i, DT=%f\n", nstlim_, dt_);
  if (runType_ == MD)
    Msg("  CRD              : %s\n", crd_dir_.c_str());
  else { // Some type of replica run
    Msg("  NUMEXCHG=%i\n", numexchg_);
    Msg("  CRD_DIR          : %s\n", crd_dir_.c_str());
    Msg("  %u dimensions, %u total replicas.\n", Dims_.size(), totalReplicas_);
  }
}

// RemdDirs::CreateRuns()
int RemdDirs::CreateRuns(std::string const& TopDir, StrArray const& RunDirs,
                         int start, bool overwrite)
{
  if (crd_dir_.empty()) {
    ErrorMsg("No starting coords directory/file specified.\n");
    return 1;
  }
  int run = start;
  for (StrArray::const_iterator runDir = RunDirs.begin();
                                runDir != RunDirs.end(); ++runDir, ++run)
  {
    if (ChangeDir(TopDir)) return 1;
    // Determine run directory name, see if it is being overwritten.
    Msg("  RUNDIR: %s\n", runDir->c_str());
    if (fileExists(*runDir) && !overwrite) {
      ErrorMsg("Directory '%s' exists and '-O' not specified.\n", runDir->c_str());
      return 1;
    }
    // Create run input
    int err;
    if (runType_ == MD)
      err = CreateMD(start, run, *runDir);
    else
      err = CreateRemd(start, run, *runDir);
    if (err) return 1;
  }
  return 0;
}

// RemdDirs::CreateAnalyzeArchive()
int RemdDirs::CreateAnalyzeArchive(std::string const& TopDir, StrArray const& RunDirs,
                                   int start, int stop, bool overwrite, bool check,
                                   bool analyzeEnabled, bool archiveEnabled)
{
  // Find trajectory files
  ChangeDir( TopDir + "/" + RunDirs.front() );
  StrArray TrajFiles;
  std::string traj_prefix;
  if (runType_ == MD)
    TrajFiles = ExpandToFilenames("md.nc.*");
  else
    TrajFiles = ExpandToFilenames("TRAJ/rem.crd.*");
  if (TrajFiles.empty()) {
    if (check) {
      ErrorMsg("No trajectory files found.\n");
      return 1;
    }
    if (runType_ == MD)
      traj_prefix.assign("/md.nc.001");
    else 
      traj_prefix.assign("/TRAJ/rem.crd.001");
    Msg("Warning: Check disabled. Assuming first traj is '%s'\n", traj_prefix.c_str());
  } else
    traj_prefix.assign("/" + TrajFiles.front());

  // Ensure traj 1 for all runs between start and stop exist.
  ChangeDir( TopDir );
  if (check) {
    for (StrArray::const_iterator rdir = RunDirs.begin(); rdir != RunDirs.end(); ++rdir)
    {
      std::string TRAJ1(*rdir + traj_prefix);
      if (CheckExists("Trajectory", TRAJ1)) return 1;
    }
  }

  // Set up input for analysis -------------------
  if (analyzeEnabled) {
    ChangeDir( TopDir );
    Msg("Creating input for analysis.\n");
    std::string CPPDIR = "Analyze." + integerToString(start) + "." +
                                      integerToString(stop);
    if ( fileExists(CPPDIR) ) {
      if (!overwrite) {
        ErrorMsg("Directory '%s' exists and '-O' not specified.\n", CPPDIR.c_str());
        return 1;
      }
    } else
      Mkdir( CPPDIR );
    // Analysis input
    std::string inputName("batch.cpptraj.in"); // TODO check exists? Make option?
    TextFile CPPIN;
    if (CPPIN.OpenWrite( CPPDIR + "/" + inputName )) return 1;
    // If HREMD, need nosort keyword
    std::string TRAJINARGS;
    if (runType_ == MD || runType_ == HREMD)
      TRAJINARGS.assign("nosort");
    CPPIN.Printf("parm %s\n", Topology().c_str());
    for (StrArray::const_iterator rdir = RunDirs.begin(); rdir != RunDirs.end(); ++rdir)
      CPPIN.Printf("ensemble ../%s%s %s\n",
                   rdir->c_str(), traj_prefix.c_str(), TRAJINARGS.c_str());
    CPPIN.Printf("strip :WAT\nautoimage\n"
                 "trajout run%i-%i.nowat.nc netcdf remdtraj %s\n",
                 start, stop, trajoutargs_.c_str());
    CPPIN.Close();
    // Create run script
    std::string scriptName(CPPDIR + "/RunAnalysis.sh");
    if (!overwrite && fileExists(scriptName)) {
      ErrorMsg("Not overwriting existing analysis script: %s\n", scriptName.c_str());
      return 1;
    }
    TextFile runScript;
    if (runScript.OpenWrite( scriptName )) return 1;
    runScript.Printf("#!/bin/bash\n\n# Run executable\nTIME0=`date +%%s`\n"
                     "$MPIRUN $EXEPATH -i %s\n"
                     "if [[ $? -ne 0 ]] ; then\n  echo \"CPPTRAJ error.\"\n  exit 1\nfi\n"
                     "TIME1=`date +%%s`\n((TOTAL = $TIME1 - $TIME0))\n"
                     "echo \"$TOTAL seconds.\"\nexit 0\n", inputName.c_str());
    runScript.Close();
    ChangePermissions( scriptName ); 
  }
  // Set up input for archiving ------------------
  if (archiveEnabled) {
    ChangeDir( TopDir );
    Msg("Creating input for archiving.\n");
    // Set up input for archiving. This will be done in 2 separate runs. 
    // The first sorts and saves fully solvated trajectories of interest
    // (FULLARCHIVE). The second saves all stripped trajs.
    if (fullarchive_.empty()) {
      ErrorMsg("FULLARCHIVE must contain a comma-separated list of ensemble members to"
               " save full coordinates, or NONE to skip this step.\n");
      return 1;
    }
    std::string ARDIR="Archive." + integerToString(start) + "." +
                                   integerToString(stop);
    if ( fileExists(ARDIR) ) {
      if (!overwrite) {
        ErrorMsg("Directory '%s' exists and '-O' not specified.\n", ARDIR.c_str());
        return 1;
      }
    } else
      Mkdir( ARDIR );
    // If HREMD, need nosort keyword
    std::string TRAJINARGS;
    if (runType_ == MD || runType_ == HREMD)
      TRAJINARGS.assign("nosort");
    std::string TOP = Topology();
    // Create input for archiving each run.
    int run = start;
    for (StrArray::const_iterator rdir = RunDirs.begin(); rdir != RunDirs.end(); ++rdir, ++run)
    {
      // Check if traj archive already exists for this run.
      std::string TARFILE( ARDIR + "/traj." + *rdir + ".tgz" );
      if (!overwrite && fileExists(TARFILE)) {
        ErrorMsg("Trajectory archive %s already exists.\n", TARFILE.c_str());
        return 1;
      }
      // Check if non-traj archive exists for this run
      TARFILE.assign( *rdir + ".tgz" );
      if (!overwrite && fileExists(TARFILE)) {
        ErrorMsg("Run archive %s already exists.\n", TARFILE.c_str());
        return 1;
      }
      // Create cpptraj input
      TextFile ARIN;
      if ( fullarchive_ != "NONE") {
        // Create input for full archiving of selected members of this run
        std::string AR1("ar1." + integerToString(run) + ".cpptraj.in");
        if (ARIN.OpenWrite(ARDIR + "/" + AR1)) return 1;
        ARIN.Printf("parm %s\nensemble ../%s%s %s\n"
                    "trajout ../%s/TRAJ/wat.nc netcdf remdtraj onlymembers %s\n",
                    TOP.c_str(), rdir->c_str(), traj_prefix.c_str(), TRAJINARGS.c_str(),
                    rdir->c_str(), fullarchive_.c_str());
        ARIN.Close();
      }
      // Create input for archiving stripped trajectories
      std::string AR2("ar2." + integerToString(run) + ".cpptraj.in");
      if (ARIN.OpenWrite(ARDIR + "/" + AR2)) return 1;
      ARIN.Printf("parm %s\nensemble ../%s%s %s\n"
                  "strip :WAT\nautoimage\ntrajout ../%s/TRAJ/nowat.nc netcdf remdtraj\n",
                  TOP.c_str(), rdir->c_str(), traj_prefix.c_str(), TRAJINARGS.c_str(),
                  rdir->c_str());
      ARIN.Close();
    }

    // Create run script.
    const char* CPPTRAJERR =
      "  if [[ $? -ne 0 ]] ; then\n    echo \"CPPTRAJ error.\"\n    exit 1\n  fi";
    std::string scriptName("RunArchive." + integerToString(start) + "."
                           + integerToString(stop) + ".sh");
    if (!overwrite && fileExists(scriptName)) {
      ErrorMsg("Not overwriting existing archive script: %s\n", scriptName.c_str());
      return 1;
    }
    TextFile runScript;
    if (runScript.OpenWrite( scriptName )) return 1;
    runScript.Printf("#!/bin/bash\n\nTOTALTIME0=`date +%%s`\nRUN=%i\nfor DIR in", start);
    for (StrArray::const_iterator rdir = RunDirs.begin(); rdir != RunDirs.end(); ++rdir)
      runScript.Printf(" %s", rdir->c_str());
    const char* tprefix;
    if (runType_ == MD)
      tprefix = "md.nc";
    else
      tprefix = "TRAJ";
    runScript.Printf(" ; do\n  TIME0=`date +%%s`\n"
                     "  # Put everything but trajectories into a separate archive.\n"
                     "  TARFILE=$DIR.tgz\n"
                     "  FILELIST=""\n  for FILE in `find $DIR -name \"*\"` ; do\n"
                     "    if [[ ! -d $FILE ]] ; then\n"
                     "      if [[ `echo \"$FILE\" | awk '{print index($0,\"%s\");}'` -eq 0 ]] ; then\n"
                     "        # Not a TRAJ directory file\n"
                     "        FILELIST=$FILELIST\" $FILE\"\n      fi\n    fi\n"
                     "  done\n  echo \"tar -czvf $TARFILE\"\n  tar -czvf $TARFILE $FILELIST\n", tprefix);
    if ( fullarchive_ != "NONE") {
      // Add command to script for full archive of this run
      runScript.Printf(
        "  # Sort and save the unbiased fully-solvated trajs\n"
        "  cd %s\n  $MPIRUN $EXEPATH -i ar1.$RUN.cpptraj.in\n%s\n"
        "  cd ..\n  FILELIST=`ls $DIR/TRAJ/wat.nc.*`\n"
        "  if [[ -z $FILELIST ]] ; then\n"
        "    echo \"Error: Sorted solvated trajectories not found.\" >> /dev/stderr\n"
        "    exit 1\n  fi\n", ARDIR.c_str(), CPPTRAJERR); 
    }
    // Add command to script for stripped archive of this run
    runScript.Printf(
        "  # Save all of the stripped trajs.\n"
        "  cd %s\n  $MPIRUN $EXEPATH -i ar2.$RUN.cpptraj.in\n%s\n" 
        "  cd ..\n"
        "  for OUTTRAJ in `ls $DIR/TRAJ/nowat.nc.*` ; do\n"
        "    FILELIST=$FILELIST\" $OUTTRAJ\"\n"
        "  done\n  TARFILE=%s/traj.$DIR.tgz\n"
        "  echo \"tar -czvf $TARFILE\"\n"
        "  tar -czvf $TARFILE $FILELIST\n"
        "  TIME1=`date +%%s`\n  ((TOTAL = $TIME1 - $TIME0))\n"
        "  echo \"$DIR took $TOTAL seconds to archive.\"\n"
        "  echo \"$TARFILE\" >> TrajArchives.txt\n"
        "  echo \"--------------------------------------------------------------\"\n"
        "  ((RUN++))\n"
        "done\nTOTALTIME1=`date +%%s`\n((TOTAL = $TOTALTIME1 - $TOTALTIME0))\n"
        "echo \"$TOTAL seconds total.\"\nexit 0\n",
        ARDIR.c_str(), CPPTRAJERR, ARDIR.c_str());
    runScript.Close();
    ChangePermissions( scriptName );
  } // END archive input

  return 0;
}

// =============================================================================
int RemdDirs::WriteRunMD(std::string const& cmd_opts) const {
  TextFile RunMD;
  if (RunMD.OpenWrite("RunMD.sh")) return 1;
  RunMD.Printf("#!/bin/bash\n\n# Run executable\nTIME0=`date +%%s`\n$MPIRUN $EXEPATH -O %s\n"
                "TIME1=`date +%%s`\n"
                "((TOTAL = $TIME1 - $TIME0))\necho \"$TOTAL seconds.\"\n\nexit 0\n",
                cmd_opts.c_str());
  RunMD.Close();
  ChangePermissions("RunMD.sh");
  return 0;
}

const std::string RemdDirs::groupfileName_( "groupfile" ); // TODO make these options
const std::string RemdDirs::remddimName_("remd.dim");

// RemdDirs::CreateRemd()
int RemdDirs::CreateRemd(int start_run, int run_num, std::string const& run_dir) {
  typedef std::vector<unsigned int> Iarray;
  // Create and change to run directory.
  if (Mkdir(run_dir)) return 1;
  if (ChangeDir(run_dir)) return 1;
  // Ensure that coords directory exists.
  if (!fileExists(crd_dir_)) {
    ErrorMsg("Coords directory '%s' not found. Must specify absolute path"
             " or path relative to '%s'\n", crd_dir_.c_str(), run_dir.c_str());
    return 1;
  }
  // Calculate ps per exchange
  double ps_per_exchg = dt_ * (double)nstlim_;
  // Do we need to setup groups for MREMD?
  bool setupGroups = (groups_.Empty() && Dims_.size() > 1);
  if (setupGroups)
    groups_.SetupGroups( Dims_.size() );
  // Create INPUT directory if not present.
  std::string input_dir("INPUT");
  if (Mkdir(input_dir)) return 1;
  // Open GROUPFILE
  TextFile GROUPFILE;
  if (GROUPFILE.OpenWrite(groupfileName_)) return 1; 
  // Figure out max width of replica extension
  int width = std::max(DigitWidth( totalReplicas_ ), 3);
  // Hold current indices in each dimension.
  Iarray Indices( Dims_.size(), 0 );
  std::string currentTop = top_file_;
  double currentTemp0 = temp0_;
  for (unsigned int rep = 0; rep != totalReplicas_; rep++)
  {
    // Get topology/temperature for this replica if necessary.
    if (top_dim_ != -1) currentTop = Dims_[top_dim_]->TopName( Indices[top_dim_]  );
    if (temp0_dim_ != -1) currentTemp0 = Dims_[temp0_dim_]->Temp0( Indices[temp0_dim_] );
    // Ensure topology exists.
    if (!fileExists( currentTop )) {
      ErrorMsg("Topology '%s' not found. Must specify absolute path"
               " or path relative to '%s'\n", currentTop.c_str(), run_dir.c_str());
      return 1;
    }
    // Info for this replica.
    if (debug_ > 1) {
      Msg("\tReplica %u: top=%s  temp0=%f", rep+1, currentTop.c_str(), currentTemp0);
      Msg("  {");
      for (Iarray::const_iterator count = Indices.begin(); count != Indices.end(); ++count)
        Msg(" %u", *count);
      Msg(" }\n");
    }
    // Save group info
    if (setupGroups)
      groups_.AddReplica( Indices, rep+1 );
    // Replica extension. 
    std::string EXT = integerToString(rep+1, width);
    // Create input
    int irest = 1;
    int ntx = 5;
    if (!override_irest_) {
      if (run_num == 0) {
        if (rep ==0)
          Msg("    Run 0: irest=0, ntx=1\n");
        irest = 0;
        ntx = 1;
      }
    } else
      Msg("    Using irest/ntx from MDIN.\n");
    std::string mdin_name(input_dir + "/in." + EXT);
    if (debug_ > 1)
      Msg("\t\tMDIN: %s\n", mdin_name.c_str());
    TextFile MDIN;
    if (MDIN.OpenWrite(mdin_name)) return 1;
    MDIN.Printf("%s", runDescription_.c_str());
    // Write indices to mdin for MREMD
    if (Dims_.size() > 1) {
      MDIN.Printf(" {");
      for (Iarray::const_iterator count = Indices.begin(); count != Indices.end(); ++count)
        MDIN.Printf(" %u", *count + 1);
      MDIN.Printf(" }");
    }
    // for Top %u at %g K 
    MDIN.Printf(" (rep %u), %g ps/exchg\n"
                " &cntrl\n"
                "    imin = 0, nstlim = %i, dt = %f,\n",
                rep+1, ps_per_exchg, nstlim_, dt_);
    if (!override_irest_)
      MDIN.Printf("    irest = %i, ntx = %i, ig = %i, numexchg = %i,\n",
                  irest, ntx, ig_, numexchg_);
    else
      MDIN.Printf("    ig = %i, numexchg = %i,\n", ig_, numexchg_);
    MDIN.Printf("    temp0 = %f, tempi = %f,\n%s", currentTemp0, currentTemp0,
                additionalInput_.c_str());
    for (unsigned int id = 0; id != Dims_.size(); id++)
      Dims_[id]->WriteMdin(Indices[id], MDIN);
    MDIN.Printf(" &end\n");
    MDIN.Close();
    // Write to groupfile
    std::string INPUT_CRD = crd_dir_ + "/" + EXT + ".rst7";
    if (run_num == start_run && !fileExists( INPUT_CRD )) {
      ErrorMsg("Coords %s not found.\n", INPUT_CRD.c_str());
      return 1;
    }
    if (debug_ > 1)
      Msg("\t\tINPCRD: %s\n", INPUT_CRD.c_str());
    std::string GROUPFILE_LINE = "-O -remlog rem.log -i " + mdin_name +
      " -p " + currentTop + " -c " + INPUT_CRD + " -o OUTPUT/rem.out." + EXT +
      " -inf INFO/reminfo." + EXT + " -r RST/" + EXT + 
      ".rst7 -x TRAJ/rem.crd." + EXT + " -l LOG/logfile." + EXT;
    for (unsigned int id = 0; id != Dims_.size(); id++)
      GROUPFILE_LINE += Dims_[id]->Groupline(EXT);
    GROUPFILE.Printf("%s\n", GROUPFILE_LINE.c_str());
    // Increment first (fastest growing) index.
    Indices[0]++;
    // Increment remaining indices if necessary.
    for (unsigned int id = 0; id != Dims_.size() - 1; id++)
    {
      if (Indices[id] == Dims_[id]->Size()) {
        Indices[id] = 0; // Set this index to zero.
        Indices[id+1]++; // Increment next index.
      }
    }
  }
  GROUPFILE.Close();
  if (debug_ > 1 && !groups_.Empty())
    groups_.PrintGroups();
  // Create remd.dim if necessary.
  if (Dims_.size() > 1) {
    TextFile REMDDIM;
    if (REMDDIM.OpenWrite(remddimName_)) return 1;
    for (unsigned int id = 0; id != Dims_.size(); id++)
      groups_.WriteRemdDim(REMDDIM, id, Dims_[id]->exch_type(), Dims_[id]->description());
    REMDDIM.Close();
  }
  // Create Run script
  std::string cmd_opts;
  std::string NG = integerToString( totalReplicas_ );
  if (runType_ == MREMD)
    cmd_opts.assign("-ng " + NG + " -groupfile " + groupfileName_ + " -remd-file " + remddimName_);
  else if (runType_ == HREMD)
    cmd_opts.assign("-ng " + NG + " -groupfile " + groupfileName_ + " -rem 3");
  else
    cmd_opts.assign("-ng " + NG + " -groupfile " + groupfileName_ + " -rem 1");
  if (WriteRunMD( cmd_opts )) return 1;
  // Create output directories
  if (Mkdir( "OUTPUT" )) return 1;
  if (Mkdir( "TRAJ"   )) return 1;
  if (Mkdir( "RST"    )) return 1;
  if (Mkdir( "INFO"   )) return 1;
  if (Mkdir( "LOG"    )) return 1;
  // Create any dimension-specific directories
  for (DimArray::const_iterator dim = Dims_.begin(); dim != Dims_.end(); ++dim) {
    if ((*dim)->OutputDir() != 0) {
      if (Mkdir( std::string((*dim)->OutputDir()))) return 1;
    }
  }
  // Input coordinates for next run will be restarts of this
  crd_dir_ = "../" + run_dir + "/RST";
  return 0;
}

// =============================================================================
/** Create input file for MD.
  * \param fname Name of MDIN file.
  * \param run_num Run number, for setting irest/ntx.
  * \param EXT Extension for restraint/dumpave files when umbrella sampling.
  */
int RemdDirs::MakeMdinForMD(std::string const& fname, int run_num, 
                            std::string const& EXT, std::string const& run_dir) const
{
  // Create input
  double total_time = dt_ * (double)nstlim_;
  int irest = 1;
  int ntx = 5;
  if (!override_irest_) {
    if (run_num == 0) {
      Msg("    Run 0: irest=0, ntx=1\n");
      irest = 0;
      ntx = 1;
    }
  } else
    Msg("    Using irest/ntx from MDIN.\n");
  TextFile MDIN;
  if (MDIN.OpenWrite(fname)) return 1;
  MDIN.Printf("%s %g ps\n"
              " &cntrl\n"
              "    imin = 0, nstlim = %i, dt = %f,\n",
              runDescription_.c_str(), total_time, nstlim_, dt_);
  if (!override_irest_)
    MDIN.Printf("    irest = %i, ntx = %i, ig = %i,\n",
                irest, ntx, ig_);
  else
    MDIN.Printf("    ig = %i,\n", ig_);
  MDIN.Printf("    temp0 = %f, tempi = %f,\n%s",
              temp0_, temp0_, additionalInput_.c_str());
  if (!rst_file_.empty()) {
    MDIN.Printf("    nmropt=1,\n");
    Msg("    Using NMR restraints.\n");
  }
  MDIN.Printf(" &end\n");
  if (!rst_file_.empty()) {
    // Restraints
    std::string rf_name(rst_file_ + EXT);
    // Ensure restraint file exists if specified.
    if (!fileExists( rf_name )) {
      ErrorMsg("Restraint file '%s' not found. Must specify absolute path"
               " or path relative to '%s'\n", rf_name.c_str(), run_dir.c_str());
      return 1;
    }
    if (umbrella_ > 0)
      MDIN.Printf("&wt\n   TYPE=\"DUMPFREQ\", istep1 = %i,\n&end\n", umbrella_);
    MDIN.Printf("&wt\n   TYPE=\"END\",\n&end\nDISANG=%s\n", rf_name.c_str());
    if (umbrella_ > 0) // TODO: customize dumpave name?
      MDIN.Printf("DUMPAVE=dumpave%s\n", EXT.c_str());
    MDIN.Printf("/\n");
  }
  MDIN.Close();
  return 0;
}

// RemdDirs::CreateMD()
int RemdDirs::CreateMD(int start_run, int run_num, std::string const& run_dir) {
  // Create and change to run directory.
  if (Mkdir(run_dir)) return 1;
  if (ChangeDir(run_dir)) return 1;
  // Do some set up for groupfile runs.
  int width = 3;
  std::vector<std::string> crd_files;
  if (n_md_runs_ > 1) {
    Msg("Number of MD runs: %i\n", n_md_runs_);
    // Figure out max width of md filename extension
    width = std::max(DigitWidth( n_md_runs_ ), 3);
    for (int grp=1; grp <= n_md_runs_; grp++)
      crd_files.push_back(crd_dir_ + "/" + integerToString(grp, width) + ".rst7");
  }
  // Ensure that coords file exists for first run.
  if (run_num == start_run) {
    if (n_md_runs_ < 2) {
      if (!fileExists(crd_dir_)) {
        ErrorMsg("Coords file '%s' not found. Must specify absolute path"
                 " or path relative to '%s'\n", crd_dir_.c_str(), run_dir.c_str());
        return 1;
      }
    } else {
      for (std::vector<std::string>::const_iterator file = crd_files.begin();
                                                    file != crd_files.end(); ++file)
        if (!fileExists(*file)) {
          ErrorMsg("Coords file '%s' not found. Must specify absolute path"
                 " or path relative to '%s'\n", file->c_str(), run_dir.c_str());
          return 1;
        }
    }
  }
  // Ensure topology exists.
  if (!fileExists( top_file_ )) {
    ErrorMsg("Topology '%s' not found. Must specify absolute path"
             " or path relative to '%s'\n", top_file_.c_str(), run_dir.c_str());
    return 1;
  }
  // Set up run command 
  std::string cmd_opts;
  if (n_md_runs_ < 2) {
    cmd_opts.assign("-i md.in -p " + top_file_ + " -c " + crd_dir_ + 
                    " -x mdcrd.nc -r mdrst.rst7 -o md.out -inf md.info");
  } else {
    TextFile GROUP;
    if (GROUP.OpenWrite(groupfileName_)) return 1;
    for (int grp = 1; grp <= n_md_runs_; grp++) {
      std::string EXT = "." + integerToString(grp, width);
      std::string mdin_name("md.in");
      if (umbrella_ > 0) {
        // Create input for umbrella runs
        mdin_name.append(EXT);
        if (MakeMdinForMD(mdin_name, run_num, EXT, run_dir)) return 1;
      }
      GROUP.Printf("-i %s -p %s -c %s -x md.nc%s -r %0*i.rst7 -o md.out%s -inf md.info%s\n",
                   mdin_name.c_str(), top_file_.c_str(), crd_files[grp-1].c_str(), EXT.c_str(),
                   width, grp, EXT.c_str(), EXT.c_str());
    } 
    GROUP.Close();
    cmd_opts.assign("-ng " + integerToString(n_md_runs_) + " -groupfile " + groupfileName_);
  }
  WriteRunMD( cmd_opts );
  // Info for this run.
  if (debug_ >= 0) // 1 
      Msg("\tMD: top=%s  temp0=%f\n", top_file_.c_str(), temp0_);
  // Create input for non-umbrella runs.
  if (umbrella_ == 0) {
    if (MakeMdinForMD("md.in", run_num, "",run_dir)) return 1;
  }
  // Input coordinates for next run will be restarts of this
  crd_dir_ = "../" + run_dir + "/";
  if (n_md_runs_ < 2) crd_dir_.append("mdrst.rst7");
  return 0;
}
