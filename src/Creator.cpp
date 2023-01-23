#include <algorithm> //std::max
#include <cstdlib> // atof, atoi
#include "Creator.h"
#include "Messages.h"
#include "TextFile.h"
#include "StringRoutines.h"
#include "ReplicaDimension.h"
#include "FileRoutines.h" // CheckExists, fileExists
#include "RepIndexArray.h"
#include "MdPackage.h"

using namespace Messages;
using namespace StringRoutines;
using namespace FileRoutines;

/** CONSTRUCTOR */
Creator::Creator() :
  totalReplicas_(0),
  top_dim_(-1),
  temp0_dim_(-1),
  ph_dim_(-1),
  debug_(0),
  n_md_runs_(0),
//  umbrella_(0),
  fileExtWidth_(3),
//  override_irest_(false),
//  override_ntx_(false),
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
std::string Creator::TopologyName() const {
  std::string topname;
  if (top_dim_ == -1)
    topname = add_path_prefix(top_file_);
  else
    topname = add_path_prefix(Dims_[top_dim_]->TopName( 0 ));
  // Topology must always exist
  if (!fileExists( topname )) {
    ErrorMsg("Topology '%s' not found. Must specify absolute path"
             " or path relative to system directory.\n", topname.c_str());
    return std::string("");
  }
  return topname;
}

/** \return Topology at specified index in topology dimension, or MD topology file if no dim. */
std::string Creator::TopologyName(RepIndexArray const& Indices) const {
  std::string topname;
  if (top_dim_ == -1)
    topname = add_path_prefix(top_file_);
  else
    topname = add_path_prefix(Dims_[top_dim_]->TopName( Indices[top_dim_] ));
  // Topology must always exist
  if (!fileExists( topname )) {
    ErrorMsg("Topology '%s' not found. Must specify absolute path"
             " or path relative to system directory.\n", topname.c_str());
    return std::string("");
  }
  return topname;
}

/** \return Temperature at specified index in temperature dim, or MD temperature if no dim. */
double Creator::Temperature(RepIndexArray const& Indices) const {
  if (temp0_dim_ == -1) return mdopts_.Temperature0().Val();
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
    ErrorMsg("No coordinates directory specified.\n");
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
    ErrorMsg("No coordinates file specified.\n");
    return Sarray();
  }
  return Sarray(1, crdName );
  //return Sarray(1, tildeExpansion(crdName) );
}

/** \return Array of reference coordinates names. FIXME make a mode where reference can be previous restart
  */
Creator::Sarray Creator::RefCoordsNames() const
{
  Sarray crd_files;
  if (runType_ == MD) {
    // MD run
    if (n_md_runs_ > 1) {
      // Multi MD
      if (!ref_file_.empty()) {
        // Single reference for all groups
        for (int grp = 1; grp <= n_md_runs_; grp++)
          crd_files.push_back( add_path_prefix(ref_file_) );
      } else if (!ref_dir_.empty()) {
        // Dir containing files XXX.<ext>
        crd_files = inputCrds_multiple_md(std::string(""), add_path_prefix(ref_dir_));
      }
    } else {
      // Single MD
      if (!ref_file_.empty())
        crd_files.push_back( add_path_prefix(ref_file_) );
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
               " or path relative to system directory.\n", it->c_str());
      return Sarray();
    }
  }

  return crd_files;
}

/** \return Array of input coordinates names.
  * Based on the run type and run number, set up an array containing
  * input coordinates file name(s).
  */
Creator::Sarray Creator::InputCoordsNames(int startRunNum, int runNum, std::string const& prevDir) const {
  Msg("DEBUG: InputCoordsNames: start=%i run=%i prev_dir='%s'\n", startRunNum, runNum, prevDir.c_str());
  Sarray crd_files;
  if (runNum == 0) {
    // Very first run. Use specified_crd_ if set; otherwise use crd_dir_.
    if (runType_ == MD) {
      // MD run
      if (n_md_runs_ > 1) {
        // Multiple input coords, one for each MD group. Expect files named
        // <DIR>/XXX.<ext>
        crd_files = inputCrds_multiple_md( add_path_prefix(specified_crd_), add_path_prefix(crd_dir_) );
      } else {
        // Single input coords for MD.
        crd_files = inputCrds_single_md( add_path_prefix(specified_crd_), add_path_prefix(crd_dir_) );
      }
    } else if (runType_ == TREMD ||
               runType_ == HREMD ||
               runType_ == PHREMD ||
               runType_ == MREMD)
    {
      // REMD run
      crd_files = inputCrds_multiple_md( add_path_prefix(specified_crd_), add_path_prefix(crd_dir_) );
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
        crd_files = inputCrds_multiple_md( add_path_prefix(specified), add_path_prefix(prevDir) );
      } else {
        std::string prev_name = prevDir + "/mdrst.rst7";
        crd_files = inputCrds_single_md( add_path_prefix(specified), add_path_prefix(prev_name) );
      }
    } else if (runType_ == TREMD ||
               runType_ == HREMD ||
               runType_ == PHREMD ||
               runType_ == MREMD)
    {
      // REMD run
      std::string prev_dir = prevDir + "/RST";
      crd_files = inputCrds_multiple_md( add_path_prefix(specified), add_path_prefix(prev_dir) );
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
                 " or path relative to system directory.\n", it->c_str());
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
  std::string mdin_file;
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
        mdopts_.Set_N_Steps().SetVal( atoi( VAR.c_str() ) );
      else if (OPT == "DT")
        mdopts_.Set_TimeStep().SetVal( atof( VAR.c_str() ) );
      else if (OPT == "IG")
        mdopts_.Set_RandomSeed().SetVal( atoi( VAR.c_str() ) );
      else if (OPT == "NUMEXCHG")
        mdopts_.Set_N_Exchanges().SetVal( atoi( VAR.c_str() ) );
      else if (OPT == "UMBRELLA")
        mdopts_.Set_RstWriteFreq().SetVal( atoi( VAR.c_str() ) );
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
        mdopts_.Set_Temperature0().SetVal( atof( VAR.c_str() ) );
      else if (OPT == "TRAJOUTARGS")
        trajoutargs_ = VAR;
      else if (OPT == "FULLARCHIVE")
        fullarchive_ = VAR;
      else if (OPT == "MDIN_FILE")
      {
        if (CheckExists("MDIN file", VAR)) { return 1; }
        mdin_file = tildeExpansion( VAR );
      }
      else if (OPT == "RST_FILE")
      {
        if (fileExists( VAR ))
          mdopts_.Set_RstFilename().SetVal( tildeExpansion( VAR ) );
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
  // MD-package specific stuff
  if (mdInterface_.AllocatePackage(MdInterface::AMBER)) {
    ErrorMsg("MD package allocate failed.\n");
    return 1;
  }
  if (!mdin_file.empty()) {
    if (mdInterface_.Package()->ReadInputOptions( mdin_file )) {
      ErrorMsg("Reading MD package input options failed.\n");
      return 1;
    }
  }
/*
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
  }*/
  if (setupCreator()) {
    ErrorMsg("Invalid or missing options in file '%s'\n", input_file.c_str());
    return 1;
  }
  return 0;
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
  //bool needsMdin = true; // TODO does this need to be an option?
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
    if (!mdopts_.Temperature0().IsSet()) {
      Msg("Warning: TEMPERATURE not specified. Using default value: %g\n", mdopts_.Temperature0().Val());
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
    } else if (providesTemp0 == 0 && !mdopts_.Temperature0().IsSet()) {
      Msg("Warning: No dimension provides temperature and TEMPERATURE not specified.\n");
      Msg("Warning:   Using default temperature: %g\n", mdopts_.Temperature0().Val());
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
  if (runType_ == MD) {
    if (!mdopts_.N_Steps().IsSet()) {
      ErrorMsg("NSTLIM not set.\n");
      return 1;
    }
  } else {
    // REMD
    if (!mdopts_.N_Steps().IsSet() || !mdopts_.N_Exchanges().IsSet()) {
      ErrorMsg("NSTLIM or NUMEXCHG not set.\n");
      return 1;
    }
  }
  if (!mdopts_.TimeStep().IsSet()) {
    Msg("Warning: DT not specified. Using default: %g\n", mdopts_.TimeStep().Val());
  }
  // FIXME check for all needed input eventually
/*  if (needsMdin && mdin_file.empty()) {
    ErrorMsg("No MDIN_FILE specified and '--nomdin' not specified.\n");
    return 1;
  }*/
  /*if (umbrella_ > 0 && n_md_runs_ < 2) {
    ErrorMsg("If UMBRELLA is specified MDRUNS must be > 1.\n");
    return 1;
  }*/

  return 0;
}

// Creator::Info()
void Creator::Info() const {
  //Msg(    "  MDIN_FILE           : %s\n", mdin_file_.c_str());
  Msg(    "  Time step             : %g\n", mdopts_.TimeStep().Val());
  Msg(    "  Random seed           : %i\n", mdopts_.RandomSeed().Val());
  if (mdopts_.RstWriteFreq().IsSet())
    Msg(  "  Restraint write freq. : %i\n", mdopts_.RstWriteFreq().Val());
  if (runType_ == MD) {
    // Regular MD
    Msg(  "  Number of steps       : %i\n", mdopts_.N_Steps().Val());
    Msg(  "  Temperature           : %g\n", mdopts_.Temperature0().Val());
    if (mdopts_.pH().IsSet())
      Msg("  pH                    : %g\n", mdopts_.pH().Val());

    Msg(  "  CRD                   : %s\n", crd_dir_.c_str());
    if (!ref_file_.empty())
      Msg("  REF                   : %s\n", ref_file_.c_str());
    if (!ref_dir_.empty())
      Msg("  REF                   : %s\n", ref_dir_.c_str());
    if (!top_file_.empty())
      Msg("  TOP                   : %s\n", top_file_.c_str());
  } else {
    // Some type of replica exchange run
    if (temp0_dim_ == -1)
      Msg("  Temperature           : %g\n", mdopts_.Temperature0().Val());
    if (ph_dim_ == -1)
      Msg("  pH                    : %g\n", mdopts_.pH().Val());
    Msg(  "  Number of exchanges   : %i\n", mdopts_.N_Exchanges().Val());
    Msg(  "  Steps per exchange    : %i\n", mdopts_.N_Steps().Val());

    Msg(  "  CRD_DIR               : %s\n", crd_dir_.c_str());
    if (!ref_file_.empty())
      Msg("  REF_PREFIX            : %s\n", ref_file_.c_str());
    else if (!ref_dir_.empty())
      Msg("  REF_DIR               : %s\n", ref_dir_.c_str());
    Msg(  "  %u dimensions, %u total replicas.\n", Dims_.size(), totalReplicas_);
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

// =============================================================================
/** Create input file for MD.
  * \param fname Name of MDIN file.
  * \param run_num Run number.
  * \param EXT Numerical extension (group #) corresponding to this MDIN when multiple MDINs needed.
  * \param Indices Array of replica indices if REMD
  * \param rep Overall replica index if REMD
  */
int Creator::MakeMdinForMD(std::string const& fname, int run_num, 
                           std::string const& EXT, 
                           RepIndexArray const& Indices, unsigned int rep)
const
{
  // Create input
  MdOptions currentMdOpts = mdopts_;
  currentMdOpts.Set_Temperature0().SetVal( Temperature( Indices ) );
  if (ph_dim_ != -1)
    currentMdOpts.Set_pH().SetVal( Dims_[ph_dim_]->SolvPH( Indices[ph_dim_] ));
  if (mdopts_.RstFilename().IsSet()) {
    // Restraints
    std::string rf_name = add_path_prefix(mdopts_.RstFilename().Val() + EXT);
    // Ensure restraint file exists if specified.
    if (!fileExists( rf_name )) {
      ErrorMsg("Restraint file '%s' not found. Must specify absolute path"
               " or path relative to system dir.\n", rf_name.c_str());
      return 1;
    }
    currentMdOpts.Set_RstFilename().SetVal( rf_name );
    if (mdopts_.RstWriteFreq().IsSet() &&
        mdopts_.RstWriteFreq().Val() > 0)
    {
      if (EXT.empty())
        currentMdOpts.Set_RstWriteFile().SetVal("dumpave"); // FIXME allow user to customize
      else
        currentMdOpts.Set_RstWriteFile().SetVal("dumpave." + EXT);
    }
  }
  // Dimension-specific options
  for (unsigned int id = 0; id != Dims_.size(); id++) {
    if (Dims_[id]->Type() == ReplicaDimension::AMD_DIHEDRAL) { // TODO check for multiple amd dims?
      currentMdOpts.Set_AmdBoost().SetVal( MdOptions::AMD_TORSIONS );
      AmdDihedralDim const& amd = static_cast<AmdDihedralDim const&>( *(Dims_[id]) );
      currentMdOpts.Set_AmdEthresh().SetVal( amd.Ethresh()[ Indices[id] ] );
      currentMdOpts.Set_AmdAlpha().SetVal( amd.Alpha()[ Indices[id] ] );
    } else if (Dims_[id]->Type() == ReplicaDimension::ReplicaDimension::SGLD) { // TODO check for multiple sgld dims?
      currentMdOpts.Set_Sgld().SetVal( MdOptions::SGLD );
      SgldDim const& sgld = static_cast<SgldDim const&>( *(Dims_[id]) );
      currentMdOpts.Set_SgldTemp().SetVal( sgld.SgTemps()[ Indices[id] ] );
      // FIXME put in sgld dim
      double sgldAvgTime = 0.2;
      Msg("Warning: Using default SGLD avg time of %f\n", sgldAvgTime);
      currentMdOpts.Set_SgldAvgTime().SetVal( sgldAvgTime );
    }
  }

  return mdInterface_.Package()->WriteMdInputFile(runDescription_, currentMdOpts, fname, run_num, Indices, rep);
}

/** Create input file for MD.
  * \param fname Name of MDIN file.
  * \param run_num Run number, for setting irest/ntx.
  * \param EXT Extension for restraint/dumpave files when umbrella sampling.
  */
int Creator::MakeMdinForMD(std::string const& fname, int run_num, 
                           std::string const& EXT)
const
{
  return MakeMdinForMD(fname, run_num, EXT, RepIndexArray(), 0);
}
