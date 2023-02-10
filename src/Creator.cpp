#include <algorithm> //std::max
#include <sstream> // ostringstream
#include "Creator.h"
#include "Messages.h"
#include "StringRoutines.h"
#include "ReplicaDimension.h"
#include "FileRoutines.h" // CheckExists, fileExists
#include "RepIndexArray.h"
#include "TextFile.h"
#include "MdPackage.h"
#include "MdPackage_Amber.h"
#include "CommonOptions.h"

using namespace Messages;
using namespace StringRoutines;
using namespace FileRoutines;

/** CONSTRUCTOR */
Creator::Creator() :
  debug_(0),
  n_md_runs_(0),
  fileExtWidth_(3),
  mdin_needs_read_(false),
  usePrevRestartAsRef_(false),
  remd_diagonal_(false),
  runType_(MD),
  runDescription_("MD")
{}

/** KEEP IN SYNC WITH RUNTYPE */
const char* Creator::RUNTYPESTR_[] = {
  "MD", "T-REMD", "H-REMD", "pH-REMD", "M-REMD"
};

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

void Creator::OptHelp() {
  Msg("Creation input file variables:\n"
      "  CRD_FILE <dir>      : Starting coordinates file/directory (run 0 only).\n"
      "                       Expect name '<CRD_FILE>/XXX.rst7' for REMD.\n"
      "  REF_FILE <dir>      : Reference coordinates file/directory (optional).\n"
      "                       Expect name '<REF_FILE>/XXX.rst7' for REMD.\n"
      "  REF_TYPE <type>     : <type>=single (default): Use single reference. <type>=previous: Use previous run restart.\n"
      "  DIMENSION <file>    : File containing replica dimension information, 1 per dimension\n"
      "    Headers:");
  for (ReplicaAllocator::Token const* ptr = ReplicaAllocator::AllocArray;
                                      ptr->Key != 0; ++ptr)
    Msg(" %s", ptr->Key);
Msg("\n"
      "  REMD_DIAGONAL <opt> : <opt>=no (default): Create replica grid. <opt>=yes: Do diagonal replicas.\n"
      "  TOPOLOGY <file>     : Topology for MD/non-H-REMD run.\n"
      "  MDIN_FILE <file>    : File containing extra MDIN input.\n"
      "  RST_FILE <file>     : File containing NMR restraints (MD only).\n"
      "  TEMPERATURE <T>     : Temperature for 1D HREMD run.\n"
      "  NSTLIM <nstlim>     : MD input; steps per exchange. Required.\n"
      "  DT <step>           : MD input; time step.\n"
      "  IG <seed>           : MD input; random seed.\n"
      "  SOLVPH <ph>         : MD input; solvent pH.\n"
      "  NUMEXCHG <#>        : MD input; number of exchanges. Required for REMD.\n"
      "  NTWX <#>            : MD input; trajectory write frequency in steps.\n"
      "  MDRUNS <#>          : Number of MD runs when not REMD (default 1).\n"
      "  UMBRELLA <#>        : Indicates MD umbrella sampling with write frequency <#>.\n");
      MdPackage_Amber::OptHelp();
      Msg("\n");
}

/** Write current options to a file. */
int Creator::WriteOptions(TextFile& outfile) const {
  if (n_md_runs_ > 0) // TODO should be 1?
    outfile.Printf("MDRUNS %i\n", n_md_runs_);
  // Files
  if (!top_file_.empty())
    outfile.Printf("TOPOLOGY %s\n", top_file_.c_str());
  if (!crd_dir_.empty())
    outfile.Printf("CRD_FILE %s\n", crd_dir_.c_str());
  if (!ref_dir_.empty()) {
    outfile.Printf("REF_FILE %s\n", ref_dir_.c_str());
    if (usePrevRestartAsRef_)
      outfile.Printf("REF_TYPE previous\n");
  }
  if (!mdin_file_.empty())
    outfile.Printf("MDIN_FILE %s\n", mdin_file_.c_str());
  if (!dim_files_.empty()) {
    for (Sarray::const_iterator it = dim_files_.begin(); it != dim_files_.end(); it++)
      outfile.Printf("DIMENSION %s\n", it->c_str());
    if (remd_diagonal_)
      outfile.Printf("REMD_DIAGONAL yes\n");
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
  if (mdopts_.pH().IsSet())
    outfile.Printf("SOLVPH %f\n", mdopts_.pH().Val());
  if (mdopts_.N_Exchanges().IsSet())
    outfile.Printf("NUMEXCHG %i\n", mdopts_.N_Exchanges().Val());
  if (mdopts_.Temperature0().IsSet())
    outfile.Printf("TEMPERATURE %f\n", mdopts_.Temperature0().Val());
  if (mdopts_.RstFilename().IsSet())
    outfile.Printf("RST_FILE %s\n", mdopts_.RstFilename().Val().c_str());
  if (mdopts_.RstWriteFreq().IsSet())
    outfile.Printf("UMBRELLA %i\n", mdopts_.RstWriteFreq().Val());
  // Package options TODO FIXME

  return 0;
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
    n_md_runs_ = convertToInteger( VAR );
  else if (OPT == "NSTLIM")
    mdopts_.Set_N_Steps().SetVal( convertToInteger( VAR ) );
  else if (OPT == "DT")
    mdopts_.Set_TimeStep().SetVal( convertToDouble( VAR ) );
  else if (OPT == "IG")
    mdopts_.Set_RandomSeed().SetVal( convertToInteger( VAR ) );
  else if (OPT == "SOLVPH")
    mdopts_.Set_pH().SetVal( convertToDouble( VAR ) );
  else if (OPT == "NUMEXCHG")
    mdopts_.Set_N_Exchanges().SetVal( convertToInteger( VAR ) );
  else if (OPT == "UMBRELLA")
    mdopts_.Set_RstWriteFreq().SetVal( convertToInteger( VAR ) );
  else if (OPT == "TOPOLOGY") {
    top_file_ = VAR;
    // If the TOPOLOGY exists at this point assume it is an absolute path
    // and perform tildeExpansion.
    if (fileExists(top_file_))
      top_file_ = tildeExpansion( top_file_ );
  } else if (OPT == "REFERENCE")
    ref_dir_ = VAR;
  else if (OPT == "REF_FILE")  // Format: <ref_dir>/EXT.rst7
    ref_dir_ = VAR;
  else if (OPT == "REF_TYPE") {
    if (VAR == "single")
      usePrevRestartAsRef_ = false;
    else if (VAR == "previous")
      usePrevRestartAsRef_ = true;
    else {
      ErrorMsg("Unrecognized option '%s' for REF_TYPE.\n", VAR.c_str());
      return -1;
    }
  } else if (OPT == "REMD_DIAGONAL") {
    if (VAR == "yes")
      remd_diagonal_ = true;
    else if (VAR == "no")
      remd_diagonal_ = false;
    else {
      ErrorMsg("Unrecognized option '%s' for REMD_DIAGONAL.\n", VAR.c_str());
      return -1;
    }
  } else if (OPT == "TEMPERATURE")
    mdopts_.Set_Temperature0().SetVal( convertToDouble( VAR ) );
  else if (OPT == "NTWX")
    mdopts_.Set_TrajWriteFreq().SetVal( convertToInteger( VAR ) );
//  else if (OPT == "TRAJOUTARGS")
//    trajoutargs_ = VAR;
//  else if (OPT == "FULLARCHIVE")
//    fullarchive_ = VAR;
  else if (OPT == "MDIN_FILE") {
    if (CheckExists("MDIN file", VAR)) { return -1; }
    mdin_file_ = tildeExpansion( VAR );
    mdin_needs_read_ = true;
  } else if (OPT == "RST_FILE") {
    if (fileExists( VAR ))
      mdopts_.Set_RstFilename().SetVal( tildeExpansion( VAR ) );
  } else {
    // Not recognized.
    return 0;
  }
  return 1;
}

// Creator::ReadOptions()
int Creator::ReadOptions(std::string const& input_file) {
  package_opts_.clear();
  // Read options from input file
  if (CheckExists("Create options file", input_file)) return 1;
  std::string fname = tildeExpansion( input_file );
  Msg("Reading Create options from file: %s\n", fname.c_str());
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

  //if (setupCreator()) {
  //  Msg("Warning: Invalid or missing options in file '%s'\n", input_file.c_str());
  //  //return 1;
  //}
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

/** Set MD options from a set external variable if not already set here. FIXME do all options*/
int Creator::SetMdOptions(MdOptions const& opts) {
  set_mdopt< Option<int> >(mdopts_.Set_TrajWriteFreq(), opts.TrajWriteFreq(), "Trajectory write frequency");
  set_mdopt< Option<double> >(mdopts_.Set_TimeStep(), opts.TimeStep(), "Time step");
  set_mdopt< Option<double> >(mdopts_.Set_Temperature0(), opts.Temperature0(), "Temperature");
  set_mdopt< Option<double> >(mdopts_.Set_pH(), opts.pH(), "Solvent pH");
  set_mdopt< Option<int>    >(mdopts_.Set_RandomSeed(), opts.RandomSeed(), "Random seed");

  return 0;
}

// Creator::LoadDimension()
int Creator::LoadDimension(std::string const& dfile) {
  if (Dims_.LoadDimension(dfile)) {
    ErrorMsg("Could not load dimension from '%s'\n", dfile.c_str());
    return 1;
  }
  dim_files_.push_back( dfile );
  // Do some run type detection
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
    runDescription_.assign("MREMD");
  }

  return 0;
}

/// \return true if the file exists
bool check_file_exists(std::string const& dirpath, std::string const& path)
{
  if (FileRoutines::is_absolute_path( path ))
    return fileExists( path );
  else
    return fileExists( dirpath + "/" + path );
}

/** Check that Creator options are valid. Count number of replicas if needed. 
  */ 
int Creator::CheckCreator(std::string const& dirpath) const {
  int errcount = 0;
  if (crd_dir_.empty()) {
    ErrorMsg("No CRD_FILE specified.\n");
    errcount++;
  }
  if (!check_file_exists(dirpath, crd_dir_)) {
    ErrorMsg("CRD_FILE '%s' not found.\n", crd_dir_.c_str());
    errcount++;
  }
  if (!ref_dir_.empty()) {
    if (!check_file_exists(dirpath, ref_dir_)) {
      ErrorMsg("REF_FILE '%s' not found.\n", ref_dir_.c_str());
      errcount++;
    }
  }
  // Do some checking based on what type of run this is.
  if (Dims_.Empty()) {
    Msg("  No dimensions defined: assuming MD run.\n");
    if (!mdopts_.Temperature0().IsSet()) {
      Msg("Warning: TEMPERATURE not specified. Using default value: %g\n", mdopts_.Temperature0().Val());
    }
    if (top_file_.empty()) {
      ErrorMsg("TOPOLOGY not specified.\n");
      errcount++;
    }
    if (!check_file_exists(dirpath, top_file_)) {
      ErrorMsg("TOPOLOGY '%s' not found.\n", top_file_.c_str());
      errcount++;
    }
  } else {
    if (!Dims_.HasDim(ReplicaDimension::TEMP) && !mdopts_.Temperature0().IsSet()) {
      Msg("Warning: No dimension provides temperature and TEMPERATURE not specified.\n");
      Msg("Warning:   Using default temperature: %g\n", mdopts_.Temperature0().Val());
    }
    if (!Dims_.HasDim(ReplicaDimension::TOPOLOGY)) {
      if (top_file_.empty()) {
        ErrorMsg("No dimension provides topology files and TOPOLOGY not specified.\n");
        errcount++;
      }
      if (!check_file_exists(dirpath, top_file_)) {
        ErrorMsg("TOPOLOGY '%s' not found.\n", top_file_.c_str());
        errcount++;
      }
    }
    if (!mdopts_.N_Exchanges().IsSet()) {
      ErrorMsg("Number of exchanges NUMEXCHG not set for REMD run.\n");
      errcount++;
    }
    if (debug_ > 0)
      Msg("    Topology dimension: %i\n    Temp0 dimension: %i    pH dimension: %i\n",
          Dims_.DimIdx(ReplicaDimension::TOPOLOGY),
          Dims_.DimIdx(ReplicaDimension::TEMP),
          Dims_.DimIdx(ReplicaDimension::PH));
  }
  // Errors
  if (!mdopts_.N_Steps().IsSet()) {
    ErrorMsg("Number of steps NSTLIM not set.\n");
    errcount++;
  }
  // Warnings
  if (!mdopts_.TrajWriteFreq().IsSet())
    Msg("Warning: Trajectory write frequency NTWX is not set.\n");
  // Perform some more error checking
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

  return errcount;
}

// Creator::Info()
void Creator::Info() const {
  // This corresponds to usePrevRestartAsRef_
  static const char* ref_prev_str[] = { "single", "previous" };

  Msg("Creator options:\n");
  Msg(    "  Run type              : %s\n", RUNTYPESTR_[runType_]);
  mdopts_.PrintOpts( (runType_ == MD), Dims_.DimIdx(ReplicaDimension::TEMP), Dims_.DimIdx(ReplicaDimension::PH));
  Msg(    "  MDIN_FILE             : %s\n", mdin_file_.c_str());
  if (runType_ == MD) {
    // Regular MD
    Msg(  "  TOPOLOGY              : %s\n", top_file_.c_str());
    Msg(  "  CRD_FILE              : %s\n", crd_dir_.c_str());
    if (!ref_dir_.empty()) {
      Msg("  REF_FILE              : %s\n", ref_dir_.c_str());
      Msg("  REF_TYPE              : %s\n", ref_prev_str[(int)usePrevRestartAsRef_]);
    }
  } else {
    // Some type of replica exchange run
    if (!Dims_.HasDim(ReplicaDimension::TOPOLOGY))
      Msg("  TOPOLOGY              : %s\n", top_file_.c_str());
    Msg(  "  CRD_FILE              : %s\n", crd_dir_.c_str());
    if (!ref_dir_.empty()) {
      Msg("  REF_FILE              : %s\n", ref_dir_.c_str());
      Msg("  REF_TYPE              : %s\n", ref_prev_str[(int)usePrevRestartAsRef_]);
    }
    // Use replica index array to get total replica count
    if (remd_diagonal_)
      Msg("  REMD_DIAGONAL         : yes\n");
    RepIndexArray Indices( Dims_, remd_diagonal_ );
    Msg(  "  %u dimensions, %u total replicas.\n", Dims_.Ndims(), Indices.TotalReplicas());
    for (unsigned int idim = 0; idim != Dims_.Ndims(); idim++)
      Msg("    %u : %s\n", idim, Dims_[idim].description());
  }
}

/** Write run script. */
int Creator::WriteRunMD(std::string const& cmd_opts) const {
  TextFile RunMD;
  if (RunMD.OpenWrite( CommonOptions::Opt_RunScriptName().Val() )) return 1;
  RunMD.Printf("#!/bin/bash\n\n# Run executable\nTIME0=`date +%%s`\n$MPIRUN $EXEPATH -O %s\n"
                "TIME1=`date +%%s`\n"
                "((TOTAL = $TIME1 - $TIME0))\necho \"$TOTAL seconds.\"\n\nexit 0\n",
                cmd_opts.c_str());
  RunMD.Close();
  ChangePermissions( CommonOptions::Opt_RunScriptName().Val() );
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
