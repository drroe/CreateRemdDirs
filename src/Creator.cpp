#include <algorithm> //std::max
#include <cstdlib> // atof, atoi
#include "Creator.h"
#include "Messages.h"
#include "TextFile.h"
#include "StringRoutines.h"
#include "ReplicaDimension.h"
#include "FileRoutines.h" // CheckExists, fileExists
#include "RepIndexArray.h"

using namespace Messages;
using namespace StringRoutines;
using namespace FileRoutines;

/** CONSTRUCTOR */
Creator::Creator() :
  nstlim_(-1),
  ig_(-1),
  numexchg_(-1),
  dt_(-1.0),
  temp0_(-1.0),
  totalReplicas_(0),
  top_dim_(-1),
  temp0_dim_(-1),
  ph_dim_(-1),
  debug_(0),
  n_md_runs_(0),
  umbrella_(0),
  fileExtWidth_(3),
  override_irest_(false),
  override_ntx_(false),
  uselog_(true),
  crdDirSpecified_(false),
  crd_ext_("rst7") //FIXME this is Amber-specific
{}

// DESTRUCTOR
Creator::~Creator() {
  for (DimArray::const_iterator dim = Dims_.begin(); dim != Dims_.end(); ++dim)
    if (*dim != 0) delete *dim;
}

/** \return First topology file name from the top_dim_ dimension (REMD) or MD topology file. */
std::string const& Creator::TopologyName() const {
  if (top_dim_ == -1) return top_file_;
  return Dims_[top_dim_]->TopName( 0 );
}

/** \return Topology at specified index in topology dimension, or MD topology file if no dim. */
std::string const& Creator::TopologyName(RepIndexArray const& Indices) const {
  if (top_dim_ == -1) return top_file_;
  return Dims_[top_dim_]->TopName( Indices[top_dim_] );
}

/** \return Temperature at specified index in temperature dim, or MD temperature if no dim. */
double Creator::Temperature(RepIndexArray const& Indices) const {
  if (temp0_dim_ == -1) return temp0_;
  return Dims_[temp0_dim_]->Temp0( Indices[temp0_dim_] );
}

/** \return File numerical prefix/extension.
  * Determines a numerical prefix/extension based on max number of expected
  * files and the current default width.
  */
std::string Creator::NumericalExt(int num, int max) const {
  int width = std::max(DigitWidth(max), fileExtWidth_);
  return integerToString(num, width);
}

/** \return Array of input coords for multiple MD. */
Creator::Sarray Creator::inputCrds_multiple_md(std::string const& specified,
                                               std::string const& def)
const
{
  Msg("DEBUG: inputCrds_multiple_md spec='%s'  def='%s'\n", specified.c_str(), def.c_str());
  Sarray crd_files;
  std::string crdDirName;
  if (!specified.empty())
    crdDirName = specified;
  else
    crdDirName = def;
  if (crdDirName.empty()) {
    ErrorMsg("No coordinates directory specified for MD groups.\n");
    return Sarray();
  }
  for (int grp=1; grp <= n_md_runs_; grp++)
    crd_files.push_back(crdDirName + "/" + NumericalExt(grp, n_md_runs_) + "." + crd_ext_);
    //crd_files.push_back(tildeExpansion(crdDirName + "/" +
    //                                   NumericalExt(grp, n_md_runs_) + "." + crd_ext_));

  return crd_files;
}

/** \return Array containing single input coords for MD run. */
Creator::Sarray Creator::inputCrds_single_md(std::string const& specified,
                                             std::string const& def)
const
{
  Msg("DEBUG: inputCrds_single_md spec='%s'  def='%s'\n", specified.c_str(), def.c_str());
  std::string crdName;
  if (!specified.empty())
    crdName = specified;
  else
    crdName = def;
  if (crdName.empty()) {
    ErrorMsg("No coordinates file specified for single MD.\n");
    return Sarray();
  }
  return Sarray(1, crdName );
  //return Sarray(1, tildeExpansion(crdName) );
}

/** \return Array of reference coordinates names.
  */
Creator::Sarray Creator::RefCoordsNames(std::string const& run_dir)
const
{
  Sarray crd_files;
  if (runType_ == MD) {
    // MD run
    if (n_md_runs_ > 1) {
      // Multi MD
      if (!ref_file_.empty()) {
        // Single reference for all groups
        for (int grp = 1; grp <= n_md_runs_; grp++)
          crd_files.push_back( ref_file_ );
      } else if (!ref_dir_.empty()) {
        // Dir containing files XXX.<ext>
        crd_files = inputCrds_multiple_md(std::string(""), ref_dir_);
      }
    } else {
      // Single MD
      if (!ref_file_.empty())
        crd_files.push_back( ref_file_ );
      else if (!ref_dir_.empty())
        Msg("Warning: Not using ref dir '%s' for single MD run.\n", ref_dir_.c_str());
    }
  } else {
    ErrorMsg("RefCoordsNames not set up for REMD yet.\n");
  }
  // Ensure ref files exist
  for (Sarray::const_iterator it = crd_files.begin(); it != crd_files.end(); ++it) {
    if (!fileExists( *it )) {
      ErrorMsg("Reference coords file '%s' not found. Must specify absolute path"
               " or path relative to '%s'\n", it->c_str(), run_dir.c_str());
      return Sarray();
    }
  }

  return crd_files;
}

/** \return Array of input coordinates names.
  * Based on the run type and run number, set up an array containing
  * input coordinates file name(s).
  */
Creator::Sarray Creator::InputCoordsNames(std::string const& run_dir, int startRunNum, int runNum) const {
  Sarray crd_files;
  if (runNum == 0) {
    // Very first run. Use specified_crd_ if set; otherwise use crd_dir_.
    if (runType_ == MD) {
      // MD run
      if (n_md_runs_ > 1) {
        // Multiple input coords, one for each MD group. Expect files named
        // <DIR>/XXX.<ext>
        crd_files = inputCrds_multiple_md( specified_crd_, crd_dir_ );
      } else {
        // Single input coords for MD.
        crd_files = inputCrds_single_md( specified_crd_, crd_dir_);
      }
    } else if (runType_ == TREMD ||
               runType_ == HREMD ||
               runType_ == PHREMD ||
               runType_ == MREMD)
    {
      // REMD run
      crd_files = inputCrds_multiple_md( specified_crd_, crd_dir_ );
      return Sarray();
    } else {
      ErrorMsg("Unhandled run type in InputCoordsNames()\n");
      return Sarray();
    }
  } else {
    // Starting after run 0. If this is the first run in the series and
    // specified_crd_ is set, use that; otherwise set up to use
    // coordinates from previous runs.
    std::string specified;
    if (startRunNum == runNum)
      specified = specified_crd_;
    if (runType_ == MD) {
      // MD run
      if (n_md_runs_ > 1) {
        std::string prev_dir = "../run." + integerToString(runNum-1, 3); // FIXME run and width should be vars
        crd_files = inputCrds_multiple_md( specified, prev_dir );
      } else {
        std::string prev_name = "../run." + integerToString(runNum-1, 3) + "/mdrst.rst7"; // FIXME run and width should be vars, mdrst.rst7 is Amber-specific
        crd_files = inputCrds_single_md( specified, prev_name );
      }
    } else if (runType_ == TREMD ||
               runType_ == HREMD ||
               runType_ == PHREMD ||
               runType_ == MREMD)
    {
      // REMD run
      std::string prev_dir = "../run." + integerToString(runNum-1, 3) + "/RST"; // FIXME run and width should be vars
      crd_files = inputCrds_multiple_md( specified, prev_dir );
      return Sarray();
    } else {
      ErrorMsg("Unhandled run type in InputCoordsNames()\n");
      return Sarray();
    }
  }
  // Ensure that files exist for the first run
  if (startRunNum == runNum) {
    for (Sarray::const_iterator it = crd_files.begin(); it != crd_files.end(); ++it) {
      if (!fileExists( *it )) {
        ErrorMsg("Coords file '%s' not found. Must specify absolute path"
                 " or path relative to '%s'\n", it->c_str(), run_dir.c_str());
        return Sarray();
      }
    }
  }
  return crd_files;
}

void Creator::OptHelp() {
  Msg("Creation input file variables:\n"
      "  CRD_FILE <dir>     : Starting coordinates location (run 0 only).\n"
      "                       Expect name '<CRD_FILE>/XXX.rst7' for REMD.\n"
      "  REF_FILE <dir>     : Reference coordinates (optional).\n"
      "                       Expect name '<REF_FILE>/XXX.rst7' for REMD.\n"
      "  REFERENCE <prefix> : Reference coordinates (optional, groupfile only).\n"
      "                       Expect name '<REFERENCE>.XXX'.\n"
      "  DIMENSION <file>   : File containing replica dimension information, 1 per dimension\n"
      "    Headers:");
  for (ReplicaAllocator::Token const* ptr = ReplicaAllocator::AllocArray;
                                      ptr->Key != 0; ++ptr)
    Msg(" %s", ptr->Key);
Msg("\n  TRAJOUTARGS <args> : Additional trajectory output args for analysis (--analyze).\n"
      "  FULLARCHIVE <arg>  : Comma-separated list of members to fully archive or NONE.\n"
      "  TOPOLOGY <file>    : Topology for 1D TREMD run.\n"
      "  MDIN_FILE <file>   : File containing extra MDIN input.\n"
      "  RST_FILE <file>    : File containing NMR restraints (MD only).\n"
      "  CPIN_FILE <file>   : CPIN file (constant pH only).\n"
      "  USELOG {yes|no}    : yes (default): use logfile (pmemd), otherwise do not (sander).\n"
      "  TEMPERATURE <T>    : Temperature for 1D HREMD run.\n"
      "  NSTLIM <nstlim>    : Input file; steps per exchange. Required.\n"
      "  DT <step>          : Input file; time step. Required.\n"
      "  IG <seed>          : Input file; random seed.\n"
      "  NUMEXCHG <#>       : Input file; number of exchanges. Required for REMD.\n"
      "  MDRUNS <#>         : Number of MD runs when not REMD (default 1).\n"
      "  UMBRELLA <#>       : Indicates MD umbrella sampling with write frequency <#>.\n\n");
}

// Creator::ReadOptions()
int Creator::ReadOptions(std::string const& input_file) {
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
        //if (start != 0) // FIXME implement this as a warning somewhere else
        //  Msg("Warning: CRD_FILE only used if start run is 0. Skipping.\n");
        //else
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
      else if (OPT == "REFERENCE") // Format: <ref_file_>.EXT
        ref_file_ = VAR;
      else if (OPT == "REF_FILE")  // Format: <ref_dir>/EXT.rst7
        ref_dir_ = VAR;
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
      else if (OPT == "CPIN_FILE")
      {
        cpin_file_ = VAR;
        if (fileExists(cpin_file_))
          cpin_file_ = tildeExpansion( cpin_file_ );
      }
      else if (OPT == "USELOG")
      {
        if (VAR == "yes")
          uselog_ = true;
        else if (VAR == "no")
          uselog_ = false;
        else {
          ErrorMsg("Expected either 'yes' or 'no' for USELOG.\n");
          OptHelp();
          return 1;
        }
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
    if (mdinFile_.ParseFile( mdin_file_ )) return 1;
    if (debug_ > 0) mdinFile_.PrintNamelists();
    std::string valname = mdinFile_.GetNamelistVar("&cntrl", "irest");
    if (!valname.empty()) {
      Msg("Warning: Using 'irest = %s' in '%s'\n", valname.c_str(), mdin_file_.c_str());
      override_irest_ = true;
    }
    valname = mdinFile_.GetNamelistVar("&cntrl", "ntx");
    if (!valname.empty()) {
      Msg("Warning: Using 'ntx = %s' in '%s'\n", valname.c_str(), mdin_file_.c_str());
      override_ntx_ = true;
    }
    // Add any &cntrl variables to additionalInput_
    for (MdinFile::const_iterator nl = mdinFile_.nl_begin(); nl != mdinFile_.nl_end(); ++nl)
    {
      if (nl->first == "&cntrl") {
        unsigned int col = 0;
        for (MdinFile::token_iterator tkn = nl->second.begin(); tkn != nl->second.end(); ++tkn)
        {
          // Avoid vars which will be set
          if (tkn->first == "imin" ||
              tkn->first == "nstlim" ||
              tkn->first == "dt" ||
              tkn->first == "ig" ||
              tkn->first == "temp0" ||
              tkn->first == "tempi" ||
              tkn->first == "numexchg" ||
              tkn->first == "solvph"
             )
          {
            Msg("Warning: Not using variable '%s' found in '%s'\n", tkn->first.c_str(), mdin_file_.c_str());
            continue;
          }
          if (col == 0)
            additionalInput_.append("   ");
          
          additionalInput_.append( tkn->first + " = " + tkn->second + ", " );
          col++;
          if (col == 4) {
            additionalInput_.append("\n");
            col = 0;
          }
        }
        if (col != 0)
          additionalInput_.append("\n");
      } else
        Msg("Warning: MDIN file contains additonal namelist '%s'\n", nl->first.c_str());
    }

    if (override_irest_ != override_ntx_) {
      ErrorMsg("Both 'irest' and 'ntx' must be in '%s' if either are.\n", mdin_file_.c_str());
      return 1;
    }
  }
  return setupCreator();
}

// Creator::LoadDimension()
int Creator::LoadDimension(std::string const& dfile) {
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

// Creator::Setup()
//int Creator::Setup(std::string const& crdDirIn, bool needsMdin) {
int Creator::setupCreator() {
  bool needsMdin = true; // TODO does this need to be an option?
  // Command line input coordinates override any in options file.
//  if (!crdDirIn.empty()) {
//    crdDirSpecified_ = true;
//    crd_dir_.assign(crdDirIn);
//  } else
//    crdDirSpecified_ = false;
  // Perform tilde expansion on coords if necessary.
  if (!crd_dir_.empty() && crd_dir_[0] == '~')
    crd_dir_ = tildeExpansion(crd_dir_);
  // Figure out what type of run this is.
  runDescription_.clear();
  if (Dims_.empty()) {
    Msg("  No dimensions defined: assuming MD run.\n");
    runType_ = MD;
    runDescription_.assign("MD");
    if (temp0_ < 0.0) {
      ErrorMsg("TEMPERATURE not specified.\n");
      return 1;
    }
    if (top_file_.empty()) {
      ErrorMsg("TOPOLOGY not specified.\n");
      return 1;
    }
  } else {
    if (Dims_.size() == 1) {
      if (Dims_[0]->Type() == ReplicaDimension::TEMP ||
          Dims_[0]->Type() == ReplicaDimension::SGLD)
        runType_ = TREMD;
      else if (Dims_[0]->Type() == ReplicaDimension::PH)
        runType_ = PHREMD;
      else
        runType_ = HREMD;
      runDescription_.assign( Dims_[0]->name() );
    } else {
      runType_ = MREMD;
      //DimArray::const_iterator dim = Dims_.begin();
      runDescription_.assign( "MREMD" );
    }
    // Count total # of replicas, Do some error checking.
    totalReplicas_ = 1;
    temp0_dim_ = -1;
    top_dim_ = -1;
    ph_dim_ = -1;
    int providesTemp0 = 0;
    int providesPh = 0;
    int providesTopFiles = 0;
    for (DimArray::const_iterator dim = Dims_.begin(); dim != Dims_.end(); ++dim)
    {
      totalReplicas_ *= (*dim)->Size();
      if ((*dim)->ProvidesTemp0()) {
        temp0_dim_ = (int)(dim - Dims_.begin());
        providesTemp0++;
      }
      if ((*dim)->ProvidesPh()) {
        ph_dim_ = (int)(dim - Dims_.begin());
        providesPh++;
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
    if (providesPh > 1) {
      ErrorMsg("At most one dimension that provides pH should be specified.\n");
      return 1;
    } else if (providesPh == 1 && cpin_file_.empty()) {
      ErrorMsg("CPIN_FILE must be specified if pH dimension is present.\n");
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
      Msg("    Topology dimension: %i\n    Temp0 dimension: %i    pH dimension: %i\n",
          top_dim_, temp0_dim_, ph_dim_);
  }
  // Perform some more error checking
  if (nstlim_ < 1 || (runType_ != MD && numexchg_ < 1)) {
    ErrorMsg("NSTLIM or NUMEXCHG < 1\n");
    return 1;
  }
  if (dt_ < 0.0) {
    ErrorMsg("DT must be specified.\n");
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

// Creator::Info()
void Creator::Info() const {
  Msg("  MDIN_FILE        : %s\n", mdin_file_.c_str());
  Msg("  NSTLIM=%i, DT=%f\n", nstlim_, dt_);
  if (runType_ == MD) {
    Msg("  CRD              : %s\n", crd_dir_.c_str());
    if (!ref_file_.empty())
      Msg("  REF              : %s\n", ref_file_.c_str());
    if (!ref_dir_.empty())
      Msg("  REF              : %s\n", ref_dir_.c_str());
    if (!top_file_.empty())
      Msg("  TOP              : %s\n", top_file_.c_str());
  } else { // Some type of replica run
    Msg("  NUMEXCHG=%i\n", numexchg_);
    Msg("  CRD_DIR          : %s\n", crd_dir_.c_str());
    if (!ref_file_.empty())
      Msg("  REF_PREFIX       : %s\n", ref_file_.c_str());
    else if (!ref_dir_.empty())
      Msg("  REF_DIR          : %s\n", ref_dir_.c_str());
    Msg("  %u dimensions, %u total replicas.\n", Dims_.size(), totalReplicas_);
  }
}

// Creator::CreateRuns()
/*
int Creator::CreateRuns(std::string const& TopDir, StrArray const& RunDirs,
                         int start, bool overwrite)
{
  if (start < 1 && crd_dir_.empty()) {
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
      ErrorMsg("Directory '%s' exists and 'overwrite' not specified.\n", runDir->c_str());
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
*/

// Creator::CreateAnalyzeArchive()
/*
int Creator::CreateAnalyzeArchive(std::string const& TopDir, StrArray const& RunDirs,
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
    if ( ileExists(ARDIR) ) {
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
*/
// =============================================================================
int Creator::WriteRunMD(std::string const& cmd_opts) const {
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

const std::string Creator::groupfileName_( "groupfile" ); // TODO make these options
const std::string Creator::remddimName_("remd.dim");

// TODO deprecate
std::string Creator::RefFileName(std::string const& EXT) const {
  std::string repRef;
  if (!ref_file_.empty())
    repRef.assign(ref_file_ + "." + EXT);
  else if (!ref_dir_.empty())
    repRef.assign(ref_dir_ + "/" + EXT + ".rst7");
  return repRef;
}

void Creator::WriteNamelist(TextFile& MDIN, std::string const& namelist,
                             MdinFile::TokenArray const& tokens)
const
{
  MDIN.Printf(" %s\n", namelist.c_str());
  unsigned int col = 0;
  for (MdinFile::token_iterator tkn = tokens.begin(); tkn != tokens.end(); ++tkn)
  {
    if (col == 0)
      MDIN.Printf("   ");
    
    MDIN.Printf("%s = %s, ", tkn->first.c_str(), tkn->second.c_str());
    col++;
    if (col == 4) {
      MDIN.Printf("\n");
      col = 0;
    }
  }
  if (col != 0)
    MDIN.Printf("\n");
  MDIN.Printf(" &end\n");
}

// Creator::CreateRemd()
//int Creator::CreateRemd(int start_run, int run_num, std::string const& run_dir) {
//}

// =============================================================================
/** Create input file for MD.
  * \param fname Name of MDIN file.
  * \param run_num Run number, for setting irest/ntx.
  * \param EXT Extension for restraint/dumpave files when umbrella sampling.
  */
int Creator::MakeMdinForMD(std::string const& fname, int run_num, 
                           std::string const& EXT, std::string const& run_dir,
                           RepIndexArray const& Indices, unsigned int rep)
const
{
  // Create input
  double total_time = dt_ * (double)nstlim_;
  // Calculate ps per exchange
  double ps_per_exchg = dt_ * (double)nstlim_;
  // Get temperature for this MDIN
  double currentTemp0 = Temperature( Indices );

  int irest = 1;
  int ntx = 5;
  if (!override_irest_) {
    if (run_num == 0) {
      if (Indices.IsZero())
        Msg("    Run 0: irest=0, ntx=1\n");
      irest = 0;
      ntx = 1;
    }
  } else
    Msg("    Using irest/ntx from MDIN.\n");
  //if (debug_ > 1)
    Msg("\t\tMDIN: %s\n", fname.c_str()); // DEBUG

  TextFile MDIN;
  if (MDIN.OpenWrite(fname)) return 1;
  if (runType_ == MD) {
    // MD header
    MDIN.Printf("%s %g ps\n"
                " &cntrl\n"
                "    imin = 0, nstlim = %i, dt = %f,\n",
                runDescription_.c_str(), total_time, nstlim_, dt_);
  } else {
    // REMD header
    MDIN.Printf("%s", runDescription_.c_str());
    // Write indices to mdin for MREMD
    if (Dims_.size() > 1) {
      MDIN.Printf(" { %s }", Indices.IndicesStr(1).c_str());
    }
    // for Top %u at %g K 
    MDIN.Printf(" (rep %u), %g ps/exchg\n"
                " &cntrl\n"
                "    imin = 0, nstlim = %i, dt = %f,\n",
                rep+1, ps_per_exchg, nstlim_, dt_);
  }

  if (!override_irest_)
    MDIN.Printf("    irest = %i, ntx = %i, ig = %i,\n",
                irest, ntx, ig_);
  else
    MDIN.Printf("    ig = %i,\n", ig_);
  if (numexchg_ > -1)
    MDIN.Printf("    numexchg = %i,\n", numexchg_);
  if (ph_dim_ != -1)
      MDIN.Printf("    solvph = %f,\n", Dims_[ph_dim_]->SolvPH( Indices[ph_dim_] ));
  MDIN.Printf("    temp0 = %f, tempi = %f,\n%s",
              currentTemp0, currentTemp0, additionalInput_.c_str());
  if (!rst_file_.empty()) {
    MDIN.Printf("    nmropt=1,\n");
    Msg("    Using NMR restraints.\n");
  }
  for (unsigned int id = 0; id != Dims_.size(); id++)
      Dims_[id]->WriteMdin(Indices[id], MDIN);
  MDIN.Printf(" &end\n");
  // Add any additional namelists
  for (MdinFile::const_iterator nl = mdinFile_.nl_begin(); nl != mdinFile_.nl_end(); ++nl)
    if (nl->first != "&cntrl")
      WriteNamelist(MDIN, nl->first, nl->second);

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

/** Create input file for MD.
  * \param fname Name of MDIN file.
  * \param run_num Run number, for setting irest/ntx.
  * \param EXT Extension for restraint/dumpave files when umbrella sampling.
  */
int Creator::MakeMdinForMD(std::string const& fname, int run_num, 
                           std::string const& EXT, std::string const& run_dir)
const
{
  return MakeMdinForMD(fname, run_num, EXT, run_dir, RepIndexArray(0), 0);
}
