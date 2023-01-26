#include "MdPackage_Amber.h"
#include "FileRoutines.h"
#include "Messages.h"
#include "TextFile.h"
#include "RepIndexArray.h"
#include "MdOptions.h"
#include "StringRoutines.h"
#include "Creator.h"
#include "Groups.h"
#include "ReplicaDimension.h"
#include "RunStatus.h"
#include "CpptrajInterface.h"

using namespace Messages;

/** CONSTRUCTOR */
MdPackage_Amber::MdPackage_Amber() :
  override_irest_(false),
  override_ntx_(false)
{}

/** COPY CONSTRUCTOR */
MdPackage_Amber::MdPackage_Amber(MdPackage_Amber const& rhs) :
  MdPackage(rhs),
  additionalInput_(rhs.additionalInput_),
  override_irest_(rhs.override_irest_),
  override_ntx_(rhs.override_ntx_),
  mdinFile_(rhs.mdinFile_)
{}

/** ASSIGMENT */
MdPackage_Amber& MdPackage_Amber::operator=(MdPackage_Amber const& rhs) {
  if (&rhs == this) return *this;
  MdPackage::operator=(rhs);
  additionalInput_ = rhs.additionalInput_;
  override_ntx_ = rhs.override_ntx_;
  override_irest_ = rhs.override_irest_;
  mdinFile_ = rhs.mdinFile_;

  return *this;
}

/** Parse amber-specific creator option. */
int MdPackage_Amber::ParseCreatorOption(std::string const& OPT, std::string const& VAR) {
  if (OPT == "USELOG")
  {
    if (VAR == "yes")
      uselog_ = true;
    else if (VAR == "no")
      uselog_ = false;
    else {
      ErrorMsg("Expected either 'yes' or 'no' for USELOG.\n");
      //OptHelp(); FIXME
      return 1;
    }
  }
  return 0;
}

/** Read amber-specific input from MDIN file. */
int MdPackage_Amber::ReadPackageInput(std::string const& fname) {
  using namespace FileRoutines;

  if (CheckExists("Amber MDIN file", fname)) return 1;
  std::string mdin_fileName = tildeExpansion(fname);

  override_irest_ = false;
  override_ntx_ = false;
  additionalInput_.clear();
  if (mdinFile_.ParseFile( mdin_fileName )) return 1;
  if (Debug() > 0) mdinFile_.PrintNamelists();
  std::string valname = mdinFile_.GetNamelistVar("&cntrl", "irest");
  if (!valname.empty()) {
    Msg("Warning: Using 'irest = %s' in '%s'\n", valname.c_str(), mdin_fileName.c_str());
    override_irest_ = true;
  }
  valname = mdinFile_.GetNamelistVar("&cntrl", "ntx");
  if (!valname.empty()) {
    Msg("Warning: Using 'ntx = %s' in '%s'\n", valname.c_str(), mdin_fileName.c_str());
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
            tkn->first == "ntwx" ||
            tkn->first == "dt" ||
            tkn->first == "ig" ||
            tkn->first == "temp0" ||
            tkn->first == "tempi" ||
            tkn->first == "numexchg" ||
            tkn->first == "solvph"
           )
        {
          Msg("Warning: Not using variable '%s' found in '%s'\n", tkn->first.c_str(), mdin_fileName.c_str());
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
    ErrorMsg("Both 'irest' and 'ntx' must be in '%s' if either are.\n", mdin_fileName.c_str());
    return 1;
  }

  return 0;
}

/** Write given namelist to input file. */
void MdPackage_Amber::writeNamelist(TextFile& MDIN, std::string const& namelist,
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

/** Write amber MDIN file. */
int MdPackage_Amber::writeMdInputFile(std::string const& runDescription,
                                      MdOptions const& mdopts,
                                      std::string const& fname, int run_num, 
                                      RepIndexArray const& Indices, unsigned int rep)
const
{
   // Create input
  // Get temperature for this MDIN
  double currentTemp0 = mdopts.Temperature0().Val();

  int irest = 1;
  int ntx = 5;
  if (!override_irest_) {
    if (run_num == 0) {
      if (rep == 0)
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
  double total_time = mdopts.TimeStep().Val() * (double)mdopts.N_Steps().Val();
  if (Indices.Empty()) {
    // MD header
    MDIN.Printf("%s %g ps\n"
                " &cntrl\n"
                "    imin = 0, nstlim = %i, dt = %f,\n",
                runDescription.c_str(), total_time, mdopts.N_Steps().Val(), mdopts.TimeStep().Val());
  } else {
    // REMD header
    MDIN.Printf("%s", runDescription.c_str());
    // Write indices to mdin for MREMD
    if (Indices.Indices().size() > 1) {
      MDIN.Printf(" { %s }", Indices.IndicesStr(1).c_str());
    }
    // for Top %u at %g K 
    MDIN.Printf(" (rep %u), %g ps/exchg\n"
                " &cntrl\n"
                "    imin = 0, nstlim = %i, dt = %f,\n",
                rep+1, total_time, mdopts.N_Steps().Val(), mdopts.TimeStep().Val());
  }
  if (mdopts.TrajWriteFreq().IsSet())
    MDIN.Printf("    ntwx = %i,\n", mdopts.TrajWriteFreq().Val());

  if (!override_irest_)
    MDIN.Printf("    irest = %i, ntx = %i, ig = %i,\n",
                irest, ntx, mdopts.RandomSeed().Val());
  else
    MDIN.Printf("    ig = %i,\n", mdopts.RandomSeed().Val());
  if (mdopts.N_Exchanges().Val() > -1)
    MDIN.Printf("    numexchg = %i,\n", mdopts.N_Exchanges().Val());
  if (mdopts.pH().IsSet())
    MDIN.Printf("    solvph = %f,\n", mdopts.pH().Val());
  MDIN.Printf("    temp0 = %f, tempi = %f,\n%s",
              currentTemp0, currentTemp0, additionalInput_.c_str());
  if (mdopts.RstFilename().IsSet()) {
    MDIN.Printf("    nmropt=1,\n");
    Msg("    Using NMR restraints.\n");
  }
  if (mdopts.AmdBoost().IsSet()) {
    int iamd = 0;
    if (mdopts.AmdBoost().Val() == MdOptions::AMD_TORSIONS)
      iamd = 2;
    else {
      ErrorMsg("Unsupported AMD value.\n");
      return 1;
    }
    MDIN.Printf("    iamd=%i, EthreshD=%f, alphaD=%f,\n", iamd,
                mdopts.AmdEthresh().Val(), mdopts.AmdAlpha().Val());
  }
  if (mdopts.Sgld().IsSet()) {
    int isgld = 0;
    if (mdopts.Sgld().Val() == MdOptions::SGLD)
      isgld = 1;
    else {
      ErrorMsg("Unsupported SGLD value.\n");
      return 1;
    }
    MDIN.Printf("    isgld=%i, tsgavg=%f, tempsg=%f\n", isgld,
                mdopts.SgldAvgTime().Val(), mdopts.SgldTemp().Val());
  }

  //for (unsigned int id = 0; id != Dims_.size(); id++)
  //    Dims_[id]->WriteMdin(Indices[id], MDIN);
  MDIN.Printf(" &end\n");
  // Add any additional namelists
  for (MdinFile::const_iterator nl = mdinFile_.nl_begin(); nl != mdinFile_.nl_end(); ++nl)
    if (nl->first != "&cntrl")
      writeNamelist(MDIN, nl->first, nl->second);

  if (mdopts.RstFilename().IsSet()) {
    // Restraints
    if (mdopts.RstWriteFreq().IsSet())
      MDIN.Printf("&wt\n   TYPE=\"DUMPFREQ\", istep1 = %i,\n&end\n", mdopts.RstWriteFreq().Val());
    MDIN.Printf("&wt\n   TYPE=\"END\",\n&end\nDISANG=%s\n", mdopts.RstFilename().Val().c_str());
    if (mdopts.RstWriteFile().IsSet()) {
      MDIN.Printf("DUMPAVE=%s\n", mdopts.RstWriteFile().Val().c_str());
    }
    MDIN.Printf("/\n");
  }
  MDIN.Close();
  return 0;
}

/** Create input files for Amber run. Should already be in run_dir. */
int MdPackage_Amber::CreateInputFiles(Creator const& creator, int start_run, int run_num, std::string const& run_dir, std::string const& prevDir)
const
{
  int err = 1;
  if (creator.IsSetupForMD()) {
    if (creator.N_MD_Runs() > 1)
      err = create_multimd_input( creator, start_run, run_num, run_dir, prevDir );
    else
      err = create_singlemd_input( creator, start_run, run_num, run_dir, prevDir );
  } else
    err = create_remd_input( creator, start_run, run_num, run_dir, prevDir );
  return err;
}

/** Create input files for Amber multi-group MD run */
int MdPackage_Amber::create_multimd_input(Creator const& creator, int start_run, int run_num, std::string const& run_dir, std::string const& prevDir)
const
{
  using namespace FileRoutines;
//  // Create and change to run directory.
//  if (Mkdir(run_dir)) return 1;
//  if (ChangeDir(run_dir)) return 1;
  // Get input coordinates array
  Creator::Sarray crd_files = creator.InputCoordsNames(start_run, run_num, prevDir);
  if (crd_files.empty()) {
    ErrorMsg("Could not get input coords for MD.\n");
    return 1;
  }
  // Ensure topology exists.
  std::string topname = creator.TopologyName();
  if (topname.empty()) {
    ErrorMsg("Could not get topology name.\n");
    return 1;
  }
  // Get reference coords if any
  Creator::Sarray ref_files = creator.RefCoordsNames();
  // Set up run command 
  std::string cmd_opts;
  TextFile GROUP;

  if (GROUP.OpenWrite(creator.GroupfileName())) return 1;
  for (int grp = 1; grp <= creator.N_MD_Runs(); grp++) {
    std::string EXT = creator.NumericalExt(grp, creator.N_MD_Runs());//integerToString(grp, width);
    std::string mdin_name("md.in");
    //if (creator.UmbrellaWriteFreq() > 0) {
      // Create input for umbrella runs
      mdin_name.append("." + EXT);
      MdOptions currentMdOpts;
      if (creator.MakeMdinForMD(currentMdOpts, EXT)) {
        ErrorMsg("Making input options for group %i failed.\n", grp);
        return 1;
      }
      if (writeMdInputFile(creator.RunDescription(), currentMdOpts,
                           mdin_name, run_num, RepIndexArray(), grp))
      {
        ErrorMsg("Create input failed for group %i\n", grp);
        return 1;
      }
      
    //}
    GROUP.Printf("-i %s -p %s -c %s -x md.nc.%s -r %s.rst7 -o md.out.%s -inf md.info.%s",
                 mdin_name.c_str(), topname.c_str(), crd_files[grp-1].c_str(),
                 EXT.c_str(), EXT.c_str(), EXT.c_str(), EXT.c_str());
    std::string const& repRef = ref_files[grp-1];//RefFileName(integerToString(grp, width));
    if (!repRef.empty()) {
      /*if (!fileExists( repRef )) {
        ErrorMsg("Reference file '%s' not found. Must specify absolute path"
                 " or path relative to '%s'\n", repRef.c_str(), run_dir.c_str());
        return 1;
      }
      repRef = tildeExpansion(repRef);*/
      GROUP.Printf(" -ref %s", repRef.c_str());
    }
    GROUP.Printf("\n");
  } 
  GROUP.Close();
  cmd_opts.assign("-ng " + StringRoutines::integerToString(creator.N_MD_Runs()) + " -groupfile " + creator.GroupfileName());
  creator.WriteRunMD( cmd_opts );
  // Info for this run.
  if (Debug() >= 0) // 1 
      Msg("\tMultiMD: top=%s\n", topname.c_str());
      //Msg("\tMD: top=%s  temp0=%f\n", topname.c_str(), temp0_);
  // Create input for non-umbrella runs.
  //if (creator.UmbrellaWriteFreq() == 0) {
  //  if (creator.MakeMdinForMD("md.in", run_num, "")) return 1;
  //}

  return 0;
}

/** Create input files for Amber single MD run. */
int MdPackage_Amber::create_singlemd_input(Creator const& creator, int start_run, int run_num, std::string const& run_dir, std::string const& prevDir)
const
{
  using namespace FileRoutines;
//  // Create and change to run directory.
//  if (Mkdir(run_dir)) return 1;
//  if (ChangeDir(run_dir)) return 1;
  // Get input coordinates array
  Creator::Sarray crd_files = creator.InputCoordsNames(start_run, run_num, prevDir);
  if (crd_files.empty()) {
    ErrorMsg("Could not get input coords for MD.\n");
    return 1;
  }
  // Ensure topology exists.
  std::string topname = creator.TopologyName();
  if (topname.empty()) {
    ErrorMsg("Could not get topology file name.\n");
    return 1;
  }
  // Get reference coords if any
  Creator::Sarray ref_files = creator.RefCoordsNames();

  // Set up run command 
  std::string cmd_opts;
  cmd_opts.assign("-i md.in -p " + topname + " -c " + crd_files.front() + 
                    " -x mdcrd.nc -r mdrst.rst7 -o md.out -inf md.info");
/*    std::string mdRef;
    if (!ref_file_.empty() || !ref_dir_.empty()) {
      if (!ref_file_.empty())
        mdRef = ref_file_;
      else if (!ref_dir_.empty())
        mdRef = ref_dir_;
      if (!ref_file_.empty() && !ref_dir_.empty())
        Msg("Warning: Both reference dir and prefix defined. Using '%s'\n", mdRef.c_str());
      if (!fileExists( mdRef )) {
        ErrorMsg("Reference file '%s' not found. Must specify absolute path"
                 " or path relative to '%s'\n", mdRef.c_str(), run_dir.c_str());
        return 1;
      }
      cmd_opts.append(" -ref " + tildeExpansion(mdRef));
    }*/
  if (!ref_files.empty())
    cmd_opts.append(" -ref " + ref_files.front());

  creator.WriteRunMD( cmd_opts );
  // Info for this run.
  if (Debug() >= 0) // 1 
      Msg("\tMD: top=%s\n", topname.c_str());
      //Msg("\tMD: top=%s  temp0=%f\n", topname.c_str(), temp0_);
  // Create input for non-umbrella runs.
  //if (creator.UmbrellaWriteFreq() == 0) {
    
    MdOptions currentMdOpts;
    if (creator.MakeMdinForMD(currentMdOpts, "")) {
      ErrorMsg("Making input options for MD failed.\n");
      return 1;
    }
    if (writeMdInputFile(creator.RunDescription(), currentMdOpts,
                         "md.in", run_num, RepIndexArray(), 0)) // TODO customize md.in name
    {
      ErrorMsg("Create input failed for MD\n");
      return 1;
    }
  //}
  // Input coordinates for next run will be restarts of this
  //crd_dir_ = "../" + run_dir + "/";
  //if (creator.N_MD_Runs() < 2) crd_dir_.append("mdrst.rst7");

  return 0;
}

/** Create input files for Amber REMD run. */
int MdPackage_Amber::create_remd_input(Creator const& creator, int start_run, int run_num, std::string const& run_dir, std::string const& prevDir)
const
{
  using namespace FileRoutines;
  using namespace StringRoutines;
//  // Create and change to run directory.
//  if (Mkdir(run_dir)) return 1;
//  if (ChangeDir(run_dir)) return 1;
  // Get Coords
  Creator::Sarray crd_files = creator.InputCoordsNames(start_run, run_num, prevDir);
  if (crd_files.empty()) {
    ErrorMsg("Could not get COORDS for REMD.\n");
    return 1;
  }
  // Get any ref coords
  Creator::Sarray ref_files = creator.RefCoordsNames();
/*
  // Ensure that coords directory exists.
  if (crdDirSpecified_ && !fileExists(crd_dir_)) {
    ErrorMsg("Coords directory '%s' not found. Must specify absolute path"
             " or path relative to '%s'\n", crd_dir_.c_str(), run_dir.c_str());
    return 1;
  }*/
  // If constant pH, ensure CPIN file exists
  if (creator.TypeOfRun() == Creator::PHREMD &&
      !fileExists(creator.CPIN_Name()))
  {
    ErrorMsg("CPIN file '%s' not found. Must specify absolute path"
             " or path relative to '%s'\n", creator.CPIN_Name().c_str(), run_dir.c_str());
    return 1;
  }
  // Do we need to setup groups for MREMD?
  Groups groups_;
  bool setupGroups = (groups_.Empty() && creator.Dims().size() > 1);
  if (setupGroups)
    groups_.SetupGroups( creator.Dims().size() );
  // Create INPUT directory if not present.
  std::string input_dir("INPUT");
  if (Mkdir(input_dir)) return 1;
  // Open GROUPFILE
  TextFile GROUPFILE;
  if (GROUPFILE.OpenWrite(creator.GroupfileName())) return 1; 
  // Hold current indices in each dimension.
  RepIndexArray Indices( creator.Dims().size() );
  for (unsigned int rep = 0; rep != creator.TotalReplicas(); rep++)
  {
    // Get replica extension
    std::string EXT = creator.NumericalExt(rep, creator.TotalReplicas());
    // Get topology for this replica
    std::string currentTop = creator.TopologyName( Indices );
    if (currentTop.empty()) {
      ErrorMsg("Could not get topology name.\n");
      return 1;
    }
    // Get temperature for this replica
    double currentTemp0 = creator.Temperature( Indices );
    // Info for this replica.
    if (Debug() > 1) {
      Msg("\tReplica %u: top=%s  temp0=%f", rep+1, currentTop.c_str(), currentTemp0);
      Msg("  { %s }\n", Indices.IndicesStr(0).c_str());
    }
    // Save group info
    if (setupGroups)
      groups_.AddReplica( Indices.Indices(), rep+1 );
    // Create input
    std::string mdin_name = input_dir + "/in." + EXT;
    MdOptions currentMdOpts;
    if (creator.MakeMdinForMD(currentMdOpts, EXT, Indices)) {
      ErrorMsg("Making input options for rep %u failed.\n", rep);
      return 1;
    }
    if (writeMdInputFile(creator.RunDescription(), currentMdOpts,
                         mdin_name, run_num, Indices, rep))
    {
      ErrorMsg("Create input failed for rep %u\n", rep);
      return 1;
    }

    // Write to groupfile. Determine restart.
    std::string const& INPUT_CRD = crd_files[rep];
    if (Debug() > 1)
      Msg("\t\tINPCRD: %s\n", INPUT_CRD.c_str());
    std::string GROUPFILE_LINE = "-O -remlog rem.log -i " + mdin_name +
      " -p " + currentTop + " -c " + INPUT_CRD + " -o OUTPUT/rem.out." + EXT +
      " -inf INFO/reminfo." + EXT + " -r RST/" + EXT + 
      ".rst7 -x TRAJ/rem.crd." + EXT;
    if (!ref_files.empty()) {
      std::string const& repRef = ref_files[rep];
      if (!fileExists( repRef )) {
        ErrorMsg("Reference file '%s' not found. Must specify absolute path"
                 " or path relative to '%s'\n", repRef.c_str(), run_dir.c_str());
        return 1;
      }
      GROUPFILE_LINE.append(" -ref " + tildeExpansion(repRef));
    }
    if (uselog_)
      GROUPFILE_LINE.append(" -l LOG/logfile." + EXT);
    if (creator.TypeOfRun() == Creator::PHREMD) {
      if (run_num == 0)
        GROUPFILE_LINE.append(" -cpin " + creator.CPIN_Name());
      else {
        // Use CPrestart from previous run
        std::string prevCP("../" + prevDir + "/CPH/cprestrt." + EXT);
        if (start_run == run_num && !fileExists( prevCP )) {
          ErrorMsg("Previous CP restart %s not found.\n", prevCP.c_str());
          return 1;
        }
        GROUPFILE_LINE.append(" -cpin " + prevCP);
      }
      GROUPFILE_LINE.append(" -cpout CPH/cpout." + EXT +
                            " -cprestrt CPH/cprestrt." + EXT);
    }
    for (unsigned int id = 0; id != creator.Dims().size(); id++)
      GROUPFILE_LINE += creator.Dims()[id]->Groupline(EXT);
    GROUPFILE.Printf("%s\n", GROUPFILE_LINE.c_str());
    // Increment first (fastest growing) index.
    Indices.Increment( creator.Dims() );

  } // END loop over all replicas
  GROUPFILE.Close();
  if (Debug() > 1 && !groups_.Empty())
    groups_.PrintGroups();
  // Create remd.dim if necessary.
  if (creator.Dims().size() > 1) {
    TextFile REMDDIM;
    if (REMDDIM.OpenWrite(creator.RemdDimName())) return 1;
    for (unsigned int id = 0; id != creator.Dims().size(); id++)
      groups_.WriteRemdDim(REMDDIM, id, creator.Dims()[id]->exch_type(), creator.Dims()[id]->description());
    REMDDIM.Close();
  }
  // Create Run script
  std::string cmd_opts;
  std::string NG = integerToString( creator.TotalReplicas() );
  if (creator.TypeOfRun() == Creator::MREMD)
    cmd_opts.assign("-ng " + NG + " -groupfile " + creator.GroupfileName() + " -remd-file " + creator.RemdDimName());
  else if (creator.TypeOfRun() == Creator::HREMD)
    cmd_opts.assign("-ng " + NG + " -groupfile " + creator.GroupfileName() + " -rem 3");
  else if (creator.TypeOfRun() == Creator::PHREMD)
    cmd_opts.assign("-ng " + NG + " -groupfile " + creator.GroupfileName() + " -rem 4");
  else
    cmd_opts.assign("-ng " + NG + " -groupfile " + creator.GroupfileName() + " -rem 1");
  if (creator.WriteRunMD( cmd_opts )) return 1;
  // Create output directories
  if (Mkdir( "OUTPUT" )) return 1;
  if (Mkdir( "TRAJ"   )) return 1;
  if (Mkdir( "RST"    )) return 1;
  if (Mkdir( "INFO"   )) return 1;
  if (Mkdir( "LOG"    )) return 1;
  if (creator.TypeOfRun() == Creator::PHREMD) {
    if (Mkdir( "CPH" )) return 1;
  }
  // Create any dimension-specific directories
  for (Creator::DimArray::const_iterator dim = creator.Dims().begin(); dim != creator.Dims().end(); ++dim) {
    if ((*dim)->OutputDir() != 0) {
      if (Mkdir( std::string((*dim)->OutputDir()))) return 1;
    }
  }
  // Input coordinates for next run will be restarts of this
  //crd_dir_ = "../" + run_dir + "/RST";
  return 0;
}

// -----------------------------------------------------------------------------
/** Read run info from an mdout file. */
int MdPackage_Amber::read_mdout(RunStatus& currentStat, std::string const& fname,
                                std::string& topname)
const
{
  using namespace StringRoutines;
  TextFile mdout;
  if (mdout.OpenRead( fname )) {
    ErrorMsg("Could not open output file '%s'\n", fname.c_str());
    return 1;
  }
  // Read the '2. CONTROL DATA FOR THE RUN' section of MDOUT
  int readInput = 0;
  const char* SEP = " ,=\r\n";
  int ncols = mdout.GetColumns(SEP);
  while (ncols > -1) {
    if (readInput == 0 && ncols >= 2) {
      if (mdout.Token(0) == "2." && mdout.Token(1) == "CONTROL")
        readInput = 1;
      else if (mdout.Token(0) == "File" && mdout.Token(1) == "Assignments:")
        readInput = 2;
    } else if (readInput == 2) {
      if (ncols == 0)
        readInput = 0;
      else {
        if (mdout.Token(1) == "PARM:") {
          topname = mdout.Token(2);
          Msg("Top: %s\n", topname.c_str());
        }
      }
    } else if (readInput == 1 && ncols > 1) {
      if (mdout.Token(0) == "3." && mdout.Token(1) == "ATOMIC")
        break;
      else {
        for (int col = 0; col != ncols - 1; col++) {
          if (mdout.Token(col) == "nstlim")
            currentStat.Set_Opts().Set_N_Steps().SetVal( convertToInteger(mdout.Token(col+1)) );
          else if (mdout.Token(col) == "dt")
            currentStat.Set_Opts().Set_TimeStep().SetVal( convertToDouble(mdout.Token(col+1)) );
          else if (mdout.Token(col) == "numexchg")
            currentStat.Set_Opts().Set_N_Exchanges().SetVal( convertToInteger(mdout.Token(col+1)) );
          else if (mdout.Token(col) == "ntwx")
            currentStat.Set_Opts().Set_TrajWriteFreq().SetVal( convertToInteger(mdout.Token(col+1)) );
        }
      }
    }
    ncols = mdout.GetColumns(SEP);
  }
  // Scan down to '5. TIMINGS'
  bool completed = false;
  const char* ptr = mdout.Gets();
  while (ptr != 0) {
    if (std::string(ptr).compare(0, 14, "   5.  TIMINGS") == 0) {
      completed = true;
      break;
    }
    ptr = mdout.Gets();
  }
  mdout.Close();
  if (completed)
    currentStat.Set_Status( RunStatus::COMPLETE );
  else
    currentStat.Set_Status( RunStatus::INCOMPLETE );

  return 0;
}

/** Read # of traj frames from trajectory using cpptraj. */
int MdPackage_Amber::read_traj_nframes(RunStatus& currentStatus, std::string const& topname, std::string const& trajname) const {
  CpptrajInterface cpptraj;

  if (!cpptraj.Available()) return 0;

  int nframes = cpptraj.GetTrajFrames(topname, trajname);
  //Msg("DEBUG: nframes %i\n", nframes);
  currentStatus.Set_CurrentTrajFrames( nframes );

  return 0;
}

/** \return Info on a current run. */
RunStatus MdPackage_Amber::RunCurrentStatus(std::vector<std::string> const& files) const {
  // Special cases
  if (files.size() == 2) {
    if (files[0] == "RunMD.sh" && files[1] == "md.in")
      return RunStatus(RunStatus::PENDING);
  }
  // Scan through files
  RunStatus currentStat;
  std::string topname;
  std::string trajname;
  for (std::vector<std::string>::const_iterator fname = files.begin();
                                                fname != files.end(); ++fname)
  {
    if (*fname == "md.out") {
      if (read_mdout( currentStat, *fname, topname )) {
        ErrorMsg("Could not read '%s'\n", fname->c_str());
      }
    } else if (*fname == "mdcrd.nc") {
       trajname = *fname;
    }
  }
  if (!topname.empty() && !trajname.empty()) {
    if (read_traj_nframes( currentStat, topname, trajname )) {
      ErrorMsg("Could not read # frames from '%s'\n", trajname.c_str());
    }
  } 
  return currentStat;
}
