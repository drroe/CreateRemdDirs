#include <algorithm> //std::max
#include <sstream> // ostringstream
#include <cstdlib> // atof, atoi
#include "Creator.h"
#include "Messages.h"
#include "StringRoutines.h"
#include "ReplicaDimension.h"
#include "FileRoutines.h" // CheckExists, fileExists
#include "RepIndexArray.h"
#include "MdPackage.h"
#include "TextFile.h"

using namespace Messages;
using namespace StringRoutines;
using namespace FileRoutines;

/** CONSTRUCTOR */
Creator::Creator() :
  totalReplicas_(0),
  debug_(0),
  n_md_runs_(0),
  fileExtWidth_(3),
  crd_ext_("rst7") //FIXME this is Amber-specific
{}

/** KEEP IN SYNC WITH RUNTYPE */
const char* Creator::RUNTYPESTR_[] = {
  "MD", "T-REMD", "H-REMD", "pH-REMD", "M-REMD"
};

/** If given path is a relative path, add a '../' prefix. */ // FIXME move to FileRoutines
static inline std::string add_path_prefix(std::string const& path) {
  if (path.empty() || path[0] == '/')
    // No path or absolute path
    return path;
  else
    // Not an absolute path
    return std::string("../" + path);
}

/** \return First topology file name from the top_dim_ dimension (REMD) or MD topology file. */
std::string Creator::TopologyName() const {
  std::string topname;
  if (Dims_.HasDim(ReplicaDimension::TOPOLOGY)) {
    TopologyDim const& topdim = static_cast<TopologyDim const&>( Dims_.Dim(ReplicaDimension::TOPOLOGY) );
    topname = add_path_prefix( topdim.TopName( 0 ) );
  } else
    topname = add_path_prefix( top_file_ );
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
  if (!Indices.Empty() && Dims_.HasDim(ReplicaDimension::TOPOLOGY)) {
    TopologyDim const& topdim = static_cast<TopologyDim const&>( Dims_.Dim(ReplicaDimension::TOPOLOGY) );
    topname = add_path_prefix( topdim.TopName( Indices[Dims_.DimIdx(ReplicaDimension::TOPOLOGY)] ) );
  } else
    topname = add_path_prefix( top_file_ );
  // Topology must always exist
  if (!fileExists( topname )) {
    ErrorMsg("Topology '%s' not found. Must specify absolute path"
             " or path relative to system directory.\n", topname.c_str());
    return std::string("");
  }
  return topname;
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
                                               std::string const& def,
                                               unsigned int nfiles)
const
{
  Msg("DEBUG: inputCrds_multiple_md spec='%s'  def='%s'\n", specified.c_str(), def.c_str());
  Sarray crd_files;
  crd_files.reserve( nfiles );
  std::string crdDirName;
  if (!specified.empty())
    crdDirName = specified;
  else
    crdDirName = def;
  if (crdDirName.empty()) {
    ErrorMsg("No coordinates directory specified.\n");
    return Sarray();
  }
  if (IsDirectory( crdDirName )) {
    // Expect <crdDirName>/XXX.<crd_ext_>
    for (unsigned int grp=1; grp <= nfiles; grp++) {
      crd_files.push_back(crdDirName + "/" + NumericalExt(grp, nfiles) + "." + crd_ext_);
      Msg("DEBUG: crd %u '%s'\n", grp, crd_files.back().c_str());
      //crd_files.push_back(tildeExpansion(crdDirName + "/" +
      //                                   NumericalExt(grp, nfiles) + "." + crd_ext_));
    }
  } else {
    // Using same file for everything
    Msg("Warning: Using single input coords for multiple replicas/groups.\n");
    for (unsigned int grp = 1; grp <= nfiles; grp++) {
      crd_files.push_back(crdDirName);
      Msg("DEBUG: crd %u '%s'\n", grp, crd_files.back().c_str());
    }
  }

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
        crd_files = inputCrds_multiple_md(std::string(""), add_path_prefix(ref_dir_), n_md_runs_);
      }
    } else {
      // Single MD
      if (!ref_file_.empty())
        crd_files.push_back( add_path_prefix(ref_file_) );
      else if (!ref_dir_.empty())
        Msg("Warning: Not using ref dir '%s' for single MD run.\n", ref_dir_.c_str());
    }
  } else {
    if (!ref_file_.empty()) {
      // Single reference for all replicas
      for (unsigned int rep = 0; rep != totalReplicas_; rep++)
        crd_files.push_back( add_path_prefix(ref_file_) );
    } else if (!ref_dir_.empty()) {
      // Dir containing files XXX.<ext>
      crd_files = inputCrds_multiple_md(std::string(""), add_path_prefix(ref_dir_), totalReplicas_);
    }
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
        crd_files = inputCrds_multiple_md( add_path_prefix(specified_crd_), add_path_prefix(crd_dir_), n_md_runs_ );
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
      crd_files = inputCrds_multiple_md( add_path_prefix(specified_crd_), add_path_prefix(crd_dir_), totalReplicas_ );
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
        crd_files = inputCrds_multiple_md( add_path_prefix(specified), add_path_prefix(prevDir), n_md_runs_ );
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
      crd_files = inputCrds_multiple_md( add_path_prefix(specified), add_path_prefix(prev_dir), totalReplicas_ );
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
      "  REF_FILE <dir>     : Reference coordinates directory (optional).\n"
      "                       Expect name '<REF_FILE>/XXX.rst7' for REMD.\n"
      "  REFERENCE <file>   : Single reference coordinates (optional).\n"
      "  DIMENSION <file>   : File containing replica dimension information, 1 per dimension\n"
      "    Headers:");
  for (ReplicaAllocator::Token const* ptr = ReplicaAllocator::AllocArray;
                                      ptr->Key != 0; ++ptr)
    Msg(" %s", ptr->Key);
Msg("\n"
      "  TOPOLOGY <file>    : Topology for 1D TREMD run.\n"
      "  MDIN_FILE <file>   : File containing extra MDIN input.\n"
      "  RST_FILE <file>    : File containing NMR restraints (MD only).\n"
      "  TEMPERATURE <T>    : Temperature for 1D HREMD run.\n"
      "  NSTLIM <nstlim>    : Input file; steps per exchange. Required.\n"
      "  DT <step>          : Input file; time step. Required.\n"
      "  IG <seed>          : Input file; random seed.\n"
      "  NUMEXCHG <#>       : Input file; number of exchanges. Required for REMD.\n"
      "  NTWX <#>           : Input file; trajectory write frequency in steps.\n"
      "  MDRUNS <#>         : Number of MD runs when not REMD (default 1).\n"
      "  UMBRELLA <#>       : Indicates MD umbrella sampling with write frequency <#>.\n"
      "Amber-specific:\n"
      "  CPIN_FILE <file>   : CPIN file (constant pH only).\n"
      "  USELOG {yes|no}    : yes (default): use logfile (pmemd), otherwise do not (sander).\n"
      "\n");
}

/** Parse a creator option from file.
  * \return 1 if option was parsed.
  * \return 0 if option was not parsed.
  * \return -1 if an error occurred.
  */
int Creator::ParseFileOption( OptArray::OptPair const& opair ) {
  std::string const& OPT = opair.first;
  std::string const& VAR = opair.second;
  if (debug_ > 0)
    Msg("    Option: %s  Variable: %s\n", OPT.c_str(), VAR.c_str());
  if      (OPT == "CRD_FILE") {
    crd_dir_ = VAR;
  } else if (OPT == "DIMENSION") {
    if (CheckExists("Dimension file", VAR)) { return -1; }
    if (LoadDimension( tildeExpansion(VAR) )) { return -1; }
  } else if (OPT == "MDRUNS")
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
  else if (OPT == "TOPOLOGY") {
    top_file_ = VAR;
    // If the TOPOLOGY exists at this point assume it is an absolute path
    // and perform tildeExpansion.
    if (fileExists(top_file_))
      top_file_ = tildeExpansion( top_file_ );
  } else if (OPT == "REFERENCE") // Format: <ref_file_>.EXT
    ref_file_ = VAR;
  else if (OPT == "REF_FILE")  // Format: <ref_dir>/EXT.rst7
    ref_dir_ = VAR;
  else if (OPT == "TEMPERATURE")
    mdopts_.Set_Temperature0().SetVal( atof( VAR.c_str() ) );
  else if (OPT == "NTWX")
    mdopts_.Set_TrajWriteFreq().SetVal( atoi( VAR.c_str() ) );
//  else if (OPT == "TRAJOUTARGS")
//    trajoutargs_ = VAR;
//  else if (OPT == "FULLARCHIVE")
//    fullarchive_ = VAR;
  else if (OPT == "MDIN_FILE") {
    if (CheckExists("MDIN file", VAR)) { return -1; }
    mdin_file_ = tildeExpansion( VAR );
  } else if (OPT == "RST_FILE") {
    if (fileExists( VAR ))
      mdopts_.Set_RstFilename().SetVal( tildeExpansion( VAR ) );
  } else {
    // Not recognized.
    return 0;
  }
  return 1;
}

/** Write current options to a file. */
int Creator::WriteOptions(std::string const& output_file) const {
  if (fileExists(output_file)) {
    Msg("Warning: '%s' exists.\n", output_file.c_str());
    bool overwrite = YesNoPrompt("Overwrite?");
    if (!overwrite) return 0;
  }
  Msg("Writing create options to '%s'\n", output_file.c_str());
  TextFile outfile;
  if (outfile.OpenWrite( output_file )) {
    ErrorMsg("Opening '%s' for write failed.\n");
    return 1;
  }
  if (n_md_runs_ > 0) // TODO should be 1?
    outfile.Printf("MDRUNS %i\n", n_md_runs_);
  // Files
  if (!top_file_.empty())
    outfile.Printf("TOPOLOGY %s\n", top_file_.c_str());
  if (!crd_dir_.empty())
    outfile.Printf("CRD_FILE %s\n", crd_dir_.c_str());
  if (!ref_dir_.empty())
    outfile.Printf("REF_FILE %s\n", ref_dir_.c_str());
  if (!ref_file_.empty())
    outfile.Printf("REFERENCE %s\n", ref_file_.c_str());
  if (!mdin_file_.empty())
    outfile.Printf("MDIN_FILE %s\n", mdin_file_.c_str());
  if (!dim_files_.empty()) {
    for (Sarray::const_iterator it = dim_files_.begin(); it != dim_files_.end(); it++)
      outfile.Printf("DIMENSION %s\n", it->c_str());
  }
  // Md Options
  if (mdopts_.N_Steps().IsSet())
    outfile.Printf("NSTLIM %i\n", mdopts_.N_Steps().Val());
  if (mdopts_.TrajWriteFreq().IsSet())
    outfile.Printf("NTWX %i\n", mdopts_.TrajWriteFreq().Val());
  if (mdopts_.TimeStep().IsSet())
    outfile.Printf("DT %f\n", mdopts_.TimeStep().Val());
  if (mdopts_.RandomSeed().IsSet())
    outfile.Printf("IG %i\n", mdopts_.RandomSeed().Val());
  if (mdopts_.N_Exchanges().IsSet())
    outfile.Printf("NUMEXCHG %i\n", mdopts_.N_Exchanges().Val());
  if (mdopts_.Temperature0().IsSet())
    outfile.Printf("TEMPERATURE %f\n", mdopts_.Temperature0().Val());
  if (mdopts_.RstFilename().IsSet())
    outfile.Printf("RST_FILE %s\n", mdopts_.RstFilename().Val().c_str());
  if (mdopts_.RstWriteFreq().IsSet())
    outfile.Printf("UMBRELLA %i\n", mdopts_.RstWriteFreq().Val());
  // Package options

  return 0;
}

// Creator::ReadOptions()
int Creator::ReadOptions(std::string const& input_file) {
  package_opts_.clear();
  // Read options from input file
  if (CheckExists("Input file", input_file)) return 1;
  std::string fname = tildeExpansion( input_file );
  Msg("Reading input from file: %s\n", fname.c_str());
  TextFile infile;
  OptArray Options = infile.GetOptionsArray(fname, debug_);
  if (Options.empty()) return 1;
  for (OptArray::const_iterator opair = Options.begin(); opair != Options.end(); ++opair)
  {
    int ret = ParseFileOption( *opair );
    if (ret == -1) {
      ErrorMsg("Could not parse option '%s' = '%s'\n", opair->first.c_str(), opair->second.c_str());
      return 1;
    } else if (ret == 0) {
      // Potentially package-specific
      package_opts_.AddOpt( *opair );
    }
  } // END loop over options from file

  if (setupCreator()) {
    Msg("Warning: Invalid or missing options in file '%s'\n", input_file.c_str());
    //return 1;
  }
  return 0;
}

/// Template function for potentially overwriting MD options
template <typename T> void set_mdopt(T& currentOpt, T const& newOpt, std::string const& desc) {
  std::ostringstream oss;
  if (newOpt.IsSet()) {
    if (currentOpt.IsSet())
      oss << desc << " is already set to " << currentOpt.Val() << ", will not overwrite with " << newOpt.Val();
    else {
      oss << "Using " << desc << " of " << newOpt.Val() << " from package input";
      currentOpt.SetVal( newOpt.Val() );
    }
    Msg("\t%s.\n", oss.str().c_str());
  }
}

/** Set MD options from external source. */
int Creator::SetMdOptions(MdOptions const& opts) {
  set_mdopt< Option<int> >(mdopts_.Set_TrajWriteFreq(), opts.TrajWriteFreq(), "Trajectory write frequency");
  set_mdopt< Option<double> >(mdopts_.Set_TimeStep(), opts.TimeStep(), "Time step");
  set_mdopt< Option<double> >(mdopts_.Set_Temperature0(), opts.Temperature0(), "Temperature");

  return 0;
}

// Creator::LoadDimension()
int Creator::LoadDimension(std::string const& dfile) {
  if (Dims_.LoadDimension(dfile)) {
    ErrorMsg("Could not load dimension from '%s'\n", dfile.c_str());
    return 1;
  }
  dim_files_.push_back( dfile );

  return 0;
}

// Creator::Setup()
int Creator::setupCreator() {
  // Perform tilde expansion on coords if necessary.
  if (!crd_dir_.empty() && crd_dir_[0] == '~')
    crd_dir_ = tildeExpansion(crd_dir_);
  // Figure out what type of run this is.
  runDescription_.clear();
  if (Dims_.Empty()) {
    totalReplicas_ = 0;
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
    if (Dims_.Ndims() == 1) {
      if (Dims_.FirstDim().Type() == ReplicaDimension::TEMP ||
          Dims_.FirstDim().Type() == ReplicaDimension::SGLD)
        runType_ = TREMD;
      else if (Dims_.FirstDim().Type() == ReplicaDimension::PH)
        runType_ = PHREMD;
      else
        runType_ = HREMD;
      runDescription_.assign( Dims_.FirstDim().name() );
    } else {
      runType_ = MREMD;
      //DimArray::const_iterator dim = Dims_.begin();
      runDescription_.assign( "MREMD" );
    }
    // Count total # of replicas, Do some error checking.
    totalReplicas_ = 1;
    for (unsigned int idim = 0; idim != Dims_.Ndims(); idim++)
    {
      totalReplicas_ *= Dims_[idim].Size();
    }
    if (!Dims_.HasDim(ReplicaDimension::TEMP) && !mdopts_.Temperature0().IsSet()) {
      Msg("Warning: No dimension provides temperature and TEMPERATURE not specified.\n");
      Msg("Warning:   Using default temperature: %g\n", mdopts_.Temperature0().Val());
    }
    if (!Dims_.HasDim(ReplicaDimension::TOPOLOGY) && top_file_.empty()) {
      ErrorMsg("No dimension provides topology files and TOPOLOGY not specified.\n");
      return 1;
    }
    if (debug_ > 0)
      Msg("    Topology dimension: %i\n    Temp0 dimension: %i    pH dimension: %i\n",
          Dims_.DimIdx(ReplicaDimension::TOPOLOGY),
          Dims_.DimIdx(ReplicaDimension::TEMP),
          Dims_.DimIdx(ReplicaDimension::PH));
  }
  if (!mdopts_.TrajWriteFreq().IsSet())
    Msg("Warning: Trajectory write frequency is not set.\n");
  // Perform some more error checking
  if (runType_ == MD) {
    if (!mdopts_.N_Steps().IsSet()) {
      ErrorMsg("NSTLIM not set.\n");
      return 1;
    }
  } else {
    // REMD
    if (!mdopts_.N_Steps().IsSet() || !mdopts_.N_Exchanges().IsSet()) {
      ErrorMsg("NSTLIM or NUMEXCHG not set for REMD run.\n");
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

/** Refresh creator based on current options. */
int Creator::RefreshCreator() {
  return setupCreator();
}

// Creator::Info()
void Creator::Info() const {
  //Msg(    "  MDIN_FILE           : %s\n", mdin_file_.c_str());
  Msg("  Run type: %s\n", RUNTYPESTR_[runType_]);
  mdopts_.PrintOpts( (runType_ == MD), Dims_.DimIdx(ReplicaDimension::TEMP), Dims_.DimIdx(ReplicaDimension::PH));
  if (runType_ == MD) {
    // Regular MD
    Msg(  "  TOP                   : %s\n", top_file_.c_str());
    Msg(  "  CRD                   : %s\n", crd_dir_.c_str());
    if (!ref_file_.empty())
      Msg("  REF                   : %s\n", ref_file_.c_str());
    if (!ref_dir_.empty())
      Msg("  REF                   : %s\n", ref_dir_.c_str());
  } else {
    // Some type of replica exchange run
    if (!Dims_.HasDim(ReplicaDimension::TOPOLOGY))
      Msg("  TOP                   : %s\n", top_file_.c_str());
    Msg(  "  CRD_DIR               : %s\n", crd_dir_.c_str());
    if (!ref_file_.empty())
      Msg("  REF_PREFIX            : %s\n", ref_file_.c_str());
    else if (!ref_dir_.empty())
      Msg("  REF_DIR               : %s\n", ref_dir_.c_str());
    Msg(  "  %u dimensions, %u total replicas.\n", Dims_.Ndims(), totalReplicas_);
    for (unsigned int idim = 0; idim != Dims_.Ndims(); idim++)
      Msg("    %u : %s\n", idim, Dims_[idim].description());
  }
}

/** Write run script. */
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

/** Create input options for MD/REMD.
  * \param currentMdOpts options to set.
  * \param EXT Numerical extension (group #) corresponding to this MDIN when multiple MDINs needed.
  * \param Indices Array of replica indices if REMD
  */
int Creator::MakeMdinForMD(MdOptions& currentMdOpts,
                           std::string const& EXT, 
                           RepIndexArray const& Indices)
const
{
  // Create input
  currentMdOpts = mdopts_;
  
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
  for (unsigned int id = 0; id != Dims_.Ndims(); id++) {
    if (Dims_[id].Type() == ReplicaDimension::AMD_DIHEDRAL) {
      currentMdOpts.Set_AmdBoost().SetVal( MdOptions::AMD_TORSIONS );
      AmdDihedralDim const& amd = static_cast<AmdDihedralDim const&>( Dims_[id] );
      currentMdOpts.Set_AmdEthresh().SetVal( amd.Ethresh( Indices[id] ) );
      currentMdOpts.Set_AmdAlpha().SetVal( amd.Alpha( Indices[id] ) );
    } else if (Dims_[id].Type() == ReplicaDimension::SGLD) {
      currentMdOpts.Set_Sgld().SetVal( MdOptions::SGLD );
      SgldDim const& sgld = static_cast<SgldDim const&>( Dims_[id] );
      currentMdOpts.Set_SgldTemp().SetVal( sgld.SgTemp( Indices[id] ) );
      // FIXME put in sgld dim
      double sgldAvgTime = 0.2;
      Msg("Warning: Using default SGLD avg time of %f\n", sgldAvgTime);
      currentMdOpts.Set_SgldAvgTime().SetVal( sgldAvgTime );
    } else if (Dims_[id].Type() == ReplicaDimension::PH) {
      PhDim const& phdim = static_cast<PhDim const&>( Dims_[id] );
      currentMdOpts.Set_pH().SetVal( phdim.SolvPH( Indices[id] ) );
    } else if (Dims_[id].Type() == ReplicaDimension::TEMP) {
      TemperatureDim const& tempdim = static_cast<TemperatureDim const&>( Dims_[id] );
      currentMdOpts.Set_Temperature0().SetVal( tempdim.Temp0( Indices[id] ) );
    }
  }

  return 0;
}

/** Create input options for MD.
  * \param currentMdOpts Options to set. 
  * \param EXT Extension for restraint/dumpave files when umbrella sampling.
  */
int Creator::MakeMdinForMD(MdOptions& currentMdOpts, std::string const& EXT)
const
{
  return MakeMdinForMD(currentMdOpts, EXT, RepIndexArray());
}
