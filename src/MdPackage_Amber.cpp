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
#include "CommonOptions.h"
#include "Submitter.h"
#include "Cols.h"
#include "FileNameArray.h"

using namespace Messages;

const std::string MdPackage_Amber::groupfileName_( "groupfile" ); // TODO make these options
const std::string MdPackage_Amber::remddimName_("remd.dim");

/** CONSTRUCTOR */
MdPackage_Amber::MdPackage_Amber() //:
  //override_irest_(false),
  //override_ntx_(false)
{}

/** COPY CONSTRUCTOR */
MdPackage_Amber::MdPackage_Amber(MdPackage_Amber const& rhs) :
  MdPackage(rhs),
  additionalInput_(rhs.additionalInput_),
//  override_irest_(rhs.override_irest_),
//  override_ntx_(rhs.override_ntx_),
  mdinFile_(rhs.mdinFile_),
  cpin_file_(rhs.cpin_file_)
{}

/** ASSIGMENT */
MdPackage_Amber& MdPackage_Amber::operator=(MdPackage_Amber const& rhs) {
  if (&rhs == this) return *this;
  MdPackage::operator=(rhs);
  additionalInput_ = rhs.additionalInput_;
//  override_ntx_ = rhs.override_ntx_;
//  override_irest_ = rhs.override_irest_;
  mdinFile_ = rhs.mdinFile_;
  cpin_file_ = rhs.cpin_file_;

  return *this;
}

void MdPackage_Amber::OptHelp() {
  Msg("Amber-specific:\n"
      "  CPIN_FILE <file>   : CPIN file (constant pH only).\n"
//      "  USELOG {yes|no}    : yes (default): use logfile (pmemd), otherwise do not (sander).\n"
     );
}

/** Write amber-specific creator options to file. */
int MdPackage_Amber::WriteCreatorOptions(TextFile& outfile) const {
  if (!cpin_file_.empty())
    outfile.Printf("CPIN_FILE %s\n", cpin_file_.c_str());
  return 0;
}

/** print amber-specific options to stdout */
void MdPackage_Amber::PackageInfo() const {
  Msg("Amber-specific options:\n");
  if (!cpin_file_.empty())
    Msg("  CPIN_FILE : %s\n", cpin_file_.c_str());
}

/** Parse amber-specific creator option.
  * \return 1 if option was parsed, -1 if error, 0 otherwise.
  */ 
int MdPackage_Amber::ParseCreatorOption(std::string const& OPT, std::string const& VAR) {
  if (OPT == "CPIN_FILE") {
    cpin_file_ = VAR;
    if (FileRoutines::fileExists(cpin_file_))
      cpin_file_ = FileRoutines::tildeExpansion( cpin_file_ );
  } else
    return 0;
  return 1;
}

/** Check amber-specific creator options. */
int MdPackage_Amber::CheckCreatorOptions(Creator const& creator) const {
  int errcount = 0;
  if (creator.Dims().HasDim(ReplicaDimension::PH) && cpin_file_.empty()) {
    ErrorMsg("CPIN_FILE must be specified if pH dimension is present.\n");
    errcount++;
  }
  if (creator.MdOpts().pH().IsSet() && cpin_file_.empty()) {
    ErrorMsg("CPIN_FILE must be specified if solvent pH is specified.\n");
    errcount++;
  }
  return errcount;
}

/** Check amber-specific submitter options */
int MdPackage_Amber::CheckSubmitterOptions(Creator const& creator, Submitter const& submitter) const {
  int errcount = 0;
  // Check for MPI suffix
  std::string ext = FileRoutines::Extension( FileRoutines::Basename( submitter.Program() ) );
  bool exe_is_mpi = (ext == "MPI");

  // MPI requires MPIRUN
  if (exe_is_mpi && submitter.MpiRun().empty()) {
    ErrorMsg("Amber MPI programs require MPIRUN be set.\n");
    errcount++;
  }  
 
  // If REMD or multiple groups, ensure mpirun is set
  if (creator.Dims().Ndims() > 0 || creator.N_MD_Runs() > 1) {
    if (submitter.MpiRun().empty()) {
      ErrorMsg("Amber requires MPIRUN be set for multi-group runs.\n");
      errcount++;
    }
    // Check for MPI suffix
    if (!exe_is_mpi) {
      ErrorMsg("Amber requires parallel executable (.MPI) for multi-group runs.\n");
      errcount++;
    }
  }
  // If using multiple groups, needs MP
  return errcount;
}

/** Read amber-specific input from MDIN file. */
int MdPackage_Amber::ReadPackageInput(MdOptions& opts, std::string const& fname) {
  using namespace FileRoutines;
  using namespace StringRoutines;

  if (CheckExists("Amber MDIN file", fname)) return 1;
  std::string mdin_fileName = tildeExpansion(fname);

//  override_irest_ = false;
//  override_ntx_ = false;
  additionalInput_.clear();
  if (mdinFile_.ParseFile( mdin_fileName )) return 1;
  if (Debug() > 0) mdinFile_.PrintNamelists();
/*  std::string valname = mdinFile_.GetNamelistVar("&cntrl", "irest");
  if (!valname.empty()) {
    Msg("Warning: Using 'irest = %s' in '%s'\n", valname.c_str(), mdin_fileName.c_str());
    override_irest_ = true;
  }
  valname = mdinFile_.GetNamelistVar("&cntrl", "ntx");
  if (!valname.empty()) {
    Msg("Warning: Using 'ntx = %s' in '%s'\n", valname.c_str(), mdin_fileName.c_str());
    override_ntx_ = true;
  }*/
  // Add any &cntrl variables to additionalInput_
  for (MdinFile::const_iterator nl = mdinFile_.nl_begin(); nl != mdinFile_.nl_end(); ++nl)
  {
    if (nl->first == "&cntrl") {
      unsigned int col = 0;
      for (MdinFile::token_iterator tkn = nl->second.begin(); tkn != nl->second.end(); ++tkn)
      {
        // Vars to set in opts
        if (tkn->first == "ntwx") {
          opts.Set_TrajWriteFreq().SetVal( convertToInteger(tkn->second) );
          continue;
        } else if (tkn->first == "dt") {
          opts.Set_TimeStep().SetVal( convertToDouble(tkn->second) );
          continue;
        } else if (tkn->first == "temp0") {
          opts.Set_Temperature0().SetVal( convertToDouble(tkn->second) );
          continue;
        } else if (tkn->first == "solvph") {
          opts.Set_pH().SetVal( convertToDouble(tkn->second) );
          continue;
        } else if (tkn->first == "ig") {
          opts.Set_RandomSeed().SetVal( convertToInteger(tkn->second) );
        // Avoid vars which will be set
        } else if (tkn->first == "imin" ||
            tkn->first == "nstlim" ||
            tkn->first == "irest" ||
            tkn->first == "ntx" ||
            tkn->first == "ntwx" ||
            tkn->first == "tempi" ||
            tkn->first == "numexchg"
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

  /*if (override_irest_ != override_ntx_) {
    ErrorMsg("Both 'irest' and 'ntx' must be in '%s' if either are.\n", mdin_fileName.c_str());
    return 1;
  }*/

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
                                      std::string const& fname, bool is_initial, 
                                      RepIndexArray const& Indices, unsigned int rep)
const
{
   // Create input
  // Get temperature for this MDIN
  double currentTemp0 = mdopts.Temperature0().Val();

  int irest = 1;
  int ntx = 5;
  //if (!override_irest_) {
    if (is_initial) {
      if (rep == 0)
        Msg("    Initial run: irest=0, ntx=1\n");
      irest = 0;
      ntx = 1;
    }
  //} else
  //  Msg("    Using irest/ntx from MDIN.\n");
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

  //if (!override_irest_)
    MDIN.Printf("    irest = %i, ntx = %i, ig = %i,\n",
                irest, ntx, mdopts.RandomSeed().Val());
  //else
  //  MDIN.Printf("    ig = %i,\n", mdopts.RandomSeed().Val());
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
int MdPackage_Amber::CreateInputFiles(Creator const& creator, Submitter const& submitter, int start_run, int run_num, std::string const& run_dir, std::string const& prevDir)
const
{
  int err = 1;
  // Determine if we are using sander or pmemd to figure out if we need the logfile flag or not
  bool uselog = false;
  if (!submitter.Program().empty()) {
    std::string basename = FileRoutines::Basename( submitter.Program() );
    if ( basename.find("pmemd") != std::string::npos ) {
      Msg("DEBUG: pmemd detected, using logfile.\n");
      uselog = true;
    }
  }
  
  if (creator.Dims().Empty()) {
    if (creator.N_MD_Runs() > 1)
      err = create_multimd_input( creator, start_run, run_num, run_dir, prevDir, uselog );
    else
      err = create_singlemd_input( creator, start_run, run_num, run_dir, prevDir, uselog );
  } else
    err = create_remd_input( creator, start_run, run_num, run_dir, prevDir, uselog );
  return err;
}

/** Create input files for Amber multi-group MD run */
int MdPackage_Amber::create_multimd_input(Creator const& creator, int start_run, int run_num, std::string const& run_dir, std::string const& prevDir, bool uselog)
const
{
  using namespace FileRoutines;
//  // Create and change to run directory.
//  if (Mkdir(run_dir)) return 1;
//  if (ChangeDir(run_dir)) return 1;
  // Get input coordinates array
  FileNameArray inpcrd_files(creator.CrdDir(), prevDir, FileNameArray::IS_DIR, "rst7", 3);
  if (inpcrd_files.Generate(creator.N_MD_Runs(), (start_run == run_num))) {
    ErrorMsg("Generating input coords file names for multi-MD failed.\n");
    return 1;
  }
  // Get any ref coords
  FileNameArray refcrd_files(creator.RefDir(), prevDir, FileNameArray::IS_DIR, "rst7", 3);
  bool ref_is_initial;
  if (creator.UsePrevRestartAsRef())
    ref_is_initial = (start_run == run_num);
  else
    ref_is_initial = true;
  if (refcrd_files.Generate(creator.N_MD_Runs(), ref_is_initial)) {
    ErrorMsg("Generating ref coords file names for multi-MD failed.\n");
    return 1;
  }
  // Ensure topology exists.
  std::string topname = creator.TopologyName();
  if (topname.empty()) {
    ErrorMsg("Could not get topology name.\n");
    return 1;
  }
  // Set up run command 
  std::string cmd_opts;
  TextFile GROUP;

  if (GROUP.OpenWrite(groupfileName_)) return 1;
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
                           mdin_name, (run_num==start_run), RepIndexArray(), grp))
      {
        ErrorMsg("Create input failed for group %i\n", grp);
        return 1;
      }
      
    //}
    GROUP.Printf("-i %s -p %s -c %s -x md.nc.%s -r %s.rst7 -o md.out.%s -inf md.info.%s",
                 mdin_name.c_str(), topname.c_str(), inpcrd_files[grp-1].c_str(),
                 EXT.c_str(), EXT.c_str(), EXT.c_str(), EXT.c_str());
    if (!refcrd_files.empty()) {
      std::string const& repRef = refcrd_files[grp-1];//RefFileName(integerToString(grp, width));
      if (!repRef.empty()) {
        GROUP.Printf(" -ref %s", repRef.c_str());
      }
    }
    if (uselog)
      GROUP.Printf(" -l %s.log", EXT.c_str());
    GROUP.Printf("\n");
  } 
  GROUP.Close();
  cmd_opts.assign("-ng " + StringRoutines::integerToString(creator.N_MD_Runs()) + " -groupfile " + groupfileName_);
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
int MdPackage_Amber::create_singlemd_input(Creator const& creator, int start_run, int run_num, std::string const& run_dir, std::string const& prevDir, bool uselog)
const
{
  using namespace FileRoutines;
//  // Create and change to run directory.
//  if (Mkdir(run_dir)) return 1;
//  if (ChangeDir(run_dir)) return 1;
  // Get input coordinates array
  FileNameArray inpcrd_files(creator.CrdDir(), prevDir + "/mdrst.rst7", FileNameArray::IS_FILE, "rst7", 3);
  if (inpcrd_files.Generate(1, (start_run == run_num))) {
    ErrorMsg("Generating input coords file name failed.\n");
    return 1;
  }
  // Ensure topology exists.
  std::string topname = creator.TopologyName();
  if (topname.empty()) {
    ErrorMsg("Could not get topology file name.\n");
    return 1;
  }
  // Get reference coords if any
  FileNameArray refcrd_files(creator.RefDir(), prevDir + "/mdrst.rst7", FileNameArray::IS_FILE, "rst7", 3);
  bool ref_is_initial;
  if (creator.UsePrevRestartAsRef())
    ref_is_initial = (start_run == run_num);
  else
    ref_is_initial = true;
  if (refcrd_files.Generate(1, ref_is_initial)) {
    ErrorMsg("Generating reference coords file name failed.\n");
    return 1;
  }

  // Info for this run.
  if (Debug() >= 0) // 1 
      Msg("\tMD: top=%s\n", topname.c_str());
      //Msg("\tMD: top=%s  temp0=%f\n", topname.c_str(), temp0_);
  // Generate MD input  
  MdOptions currentMdOpts;
  if (creator.MakeMdinForMD(currentMdOpts, "")) {
    ErrorMsg("Making input options for MD failed.\n");
    return 1;
  }
  if (writeMdInputFile(creator.RunDescription(), currentMdOpts,
                       "md.in", (run_num==start_run), RepIndexArray(), 0)) // TODO customize md.in name
  {
    ErrorMsg("Create input failed for MD\n");
    return 1;
  }

  // Set up run command 
  std::string cmd_opts;
  cmd_opts.assign("-i md.in -p " + topname + " -c " + inpcrd_files[0] + 
                    " -x mdcrd.nc -r mdrst.rst7 -o md.out -inf md.info");
  if (!refcrd_files.empty())
    cmd_opts.append(" -ref " + refcrd_files[0]);
  // Constant pH run command setup
  FileNameArray cpin_files;
  if (currentMdOpts.pH().IsSet()) {
    cpin_files = FileNameArray(cpin_file_, prevDir + "/md.cprestrt", FileNameArray::IS_FILE, "cprestrt", 3);
    if (cpin_files.Generate(1, (start_run == run_num))) {
      ErrorMsg("Generating CPIN file name failed.\n");
      return 1;
    }
    cmd_opts.append(" -cpin " + cpin_files[0]);
    cmd_opts.append(" -cpout md.cpout");
    cmd_opts.append(" -cprestrt md.cprestrt");
  }
  if (uselog)
    cmd_opts.append(" -l md.log");
  // Write the run script
  creator.WriteRunMD( cmd_opts );

  return 0;
}

/** Create input files for Amber REMD run. */
int MdPackage_Amber::create_remd_input(Creator const& creator, int start_run, int run_num, std::string const& run_dir, std::string const& prevDir, bool uselog)
const
{
  using namespace FileRoutines;
  using namespace StringRoutines;
//  // Create and change to run directory.
//  if (Mkdir(run_dir)) return 1;
//  if (ChangeDir(run_dir)) return 1;
  // Hold current indices in each dimension.
  RepIndexArray Indices( creator.Dims(), creator.RemdDiagonal() );
  // Get Coords
  FileNameArray inpcrd_files(creator.CrdDir(), prevDir + "/RST", FileNameArray::IS_DIR, "rst7", 3);
  if (inpcrd_files.Generate(Indices.TotalReplicas(), (run_num==start_run))) {
    ErrorMsg("Generating input coords file names for REMD failed.\n");
    return 1;
  }
  // Get any ref coords
  FileNameArray refcrd_files(creator.RefDir(), prevDir + "/RST", FileNameArray::IS_DIR, "rst7", 3);
  bool ref_is_initial;
  if (creator.UsePrevRestartAsRef())
    ref_is_initial = (start_run == run_num);
  else
    ref_is_initial = true;
  if (refcrd_files.Generate(Indices.TotalReplicas(), ref_is_initial)) {
    ErrorMsg("Generating ref coords file names for REMD failed.\n");
    return 1;
  }

  // Constant pH setup
  FileNameArray cpin_files, cpout_files;
  if (creator.Dims().HasDim(ReplicaDimension::PH)) {
    // Constant pH. Ensure CPIN exists.
    //if (!fileExists(cpin_file_)) {
    //  ErrorMsg("CPIN file '%s' not found. Must specify absolute path"
    //           " or path relative to system dir.\n", cpin_file_.c_str());
    //  return 1;
    //}
    cpin_files = FileNameArray(cpin_file_, prevDir + "/CPH", FileNameArray::IS_DIR, "cprestrt", 3);
    if (cpin_files.Generate(Indices.TotalReplicas(), (run_num==start_run))) {
      ErrorMsg("Generating CPIN file names for REMD failed.\n");
      return 1;
    }
  }

  // Do we need to setup groups for MREMD?
  Groups groups_;
  bool setupGroups = (groups_.Empty() && creator.Dims().Ndims() > 1);
  if (setupGroups)
    groups_.SetupGroups( creator.Dims().Ndims() );
  // Create INPUT directory if not present.
  std::string input_dir("INPUT");
  if (Mkdir(input_dir)) return 1;
  // Open GROUPFILE
  TextFile GROUPFILE;
  if (GROUPFILE.OpenWrite(groupfileName_)) return 1; 

  // Loop over all replicas
  for (unsigned int rep = 0; rep != Indices.TotalReplicas(); rep++)
  {
    // Get replica extension
    std::string EXT = creator.NumericalExt(rep+1, Indices.TotalReplicas());
    // Get topology for this replica
    std::string currentTop = creator.TopologyName( Indices );
    if (currentTop.empty()) {
      ErrorMsg("Could not get topology name.\n");
      return 1;
    }
    // Save group info
    if (setupGroups)
      groups_.AddReplica( Indices.Indices(), rep+1 );
    // Create input
    std::string mdin_name = input_dir + "/in." + EXT;
    MdOptions currentMdOpts;
    if (creator.MakeMdinForMD(currentMdOpts, EXT, Indices)) {
      ErrorMsg("Making input options for rep %u failed.\n", rep+1);
      return 1;
    }
    // Info for this replica.
    if (Debug() > 1) {
      Msg("\tReplica %u: top=%s  temp0=%f", rep+1, currentTop.c_str(), currentMdOpts.Temperature0().Val());
      Msg("  { %s }\n", Indices.IndicesStr(0).c_str());
    }
    if (writeMdInputFile(creator.RunDescription(), currentMdOpts,
                         mdin_name, (run_num==start_run), Indices, rep))
    {
      ErrorMsg("Create input failed for rep %u\n", rep+1);
      return 1;
    }

    // Write to groupfile. Determine restart.
    std::string const& INPUT_CRD = inpcrd_files[rep];
    if (Debug() > 1)
      Msg("\t\tINPCRD: %s\n", INPUT_CRD.c_str());
    std::string GROUPFILE_LINE = "-O -remlog rem.log -i " + mdin_name +
      " -p " + currentTop + " -c " + INPUT_CRD + " -o OUTPUT/rem.out." + EXT +
      " -inf INFO/reminfo." + EXT + " -r RST/" + EXT + 
      ".rst7 -x TRAJ/rem.crd." + EXT;
    if (!refcrd_files.empty()) {
      std::string const& repRef = refcrd_files[rep];
      GROUPFILE_LINE.append(" -ref " + tildeExpansion(repRef));
    }
    if (uselog)
      GROUPFILE_LINE.append(" -l LOG/logfile." + EXT);
    if (!cpin_files.empty()) {
      GROUPFILE_LINE.append(" -cpin " + cpin_files[rep]);
      GROUPFILE_LINE.append(" -cpout CPH/" + EXT + ".cpout" +
                            " -cprestrt CPH/" + EXT + ".cprestrt");
    }
    /// Add any other dimension-specific flags to groupline
    for (unsigned int id = 0; id != creator.Dims().Ndims(); id++) {
      if (creator.Dims()[id].Type() == ReplicaDimension::AMD_DIHEDRAL)
        GROUPFILE_LINE += std::string(" -amd AMD/amd." + EXT);
    }
    GROUPFILE.Printf("%s\n", GROUPFILE_LINE.c_str());
    // Increment first (fastest growing) index.
    Indices.Increment( creator.Dims() );

  } // END loop over all replicas
  GROUPFILE.Close();
  if (Debug() > 1 && !groups_.Empty())
    groups_.PrintGroups();
  // Create remd.dim if necessary.
  if (creator.Dims().Ndims() > 1) {
    TextFile REMDDIM;
    if (REMDDIM.OpenWrite(remddimName_)) return 1;
    for (unsigned int id = 0; id != creator.Dims().Ndims(); id++)
      groups_.WriteRemdDim(REMDDIM, id, creator.Dims()[id].exch_type(), creator.Dims()[id].description());
    REMDDIM.Close();
  }
  // Create Run script
  std::string cmd_opts;
  std::string NG = integerToString( Indices.TotalReplicas() );
  if (creator.Dims().Ndims() > 1)
    cmd_opts.assign("-ng " + NG + " -groupfile " + groupfileName_ + " -remd-file " + remddimName_);
  else {
    // 1 replica dimension
    if (creator.Dims()[0].Type() == ReplicaDimension::TOPOLOGY) // Hamiltonian
      cmd_opts.assign("-ng " + NG + " -groupfile " + groupfileName_ + " -rem 3");
    else if (creator.Dims()[0].Type() == ReplicaDimension::PH)
      cmd_opts.assign("-ng " + NG + " -groupfile " + groupfileName_ + " -rem 4");
    else
      cmd_opts.assign("-ng " + NG + " -groupfile " + groupfileName_ + " -rem 1");
  }
  if (creator.WriteRunMD( cmd_opts )) return 1;
  // Create output directories
  if (Mkdir( "OUTPUT" )) return 1;
  if (Mkdir( "TRAJ"   )) return 1;
  if (Mkdir( "RST"    )) return 1;
  if (Mkdir( "INFO"   )) return 1;
  if (uselog)
    if (Mkdir( "LOG"    )) return 1;
  if (!cpin_files.empty()) {
    if (Mkdir( "CPH" )) return 1;
  }
  // Create any dimension-specific directories
  for (unsigned int id = 0; id != creator.Dims().Ndims(); id++) {
    if (creator.Dims()[id].Type() == ReplicaDimension::AMD_DIHEDRAL) {
      if (!fileExists("AMD")) {
        if (Mkdir("AMD")) return 1;
      }
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
          //Msg("DEBUG: Top: %s\n", topname.c_str());
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
  int current_nsteps = -1;
  while (ptr != 0) {
    std::string ptrstr(ptr);
    if (ptrstr.compare(0, 8, " NSTEP =") == 0) {
      Cols nstepline;
      nstepline.Split(ptrstr, " =");
      current_nsteps = convertToInteger( nstepline[1] );
      //Msg("DEBUG: current n steps %s %i\n", ptrstr.c_str(), current_nsteps);
      currentStat.Set_CurrentNsteps(current_nsteps);
    } else if (ptrstr.compare(0, 14, "   5.  TIMINGS") == 0) {
      completed = true;
      break;
    }
    ptr = mdout.Gets();
  }
  // Get timings
  while (ptr != 0) {
    std::string ptrstr(ptr);
    if (ptrstr.compare(0, 29, "|     Average timings for all") == 0 ||
        ptrstr.compare(0, 25, "| Average timings for all"    ) == 0)
    {
      ptr = mdout.Gets(); //|     Elapsed(s) =
      ptr = mdout.Gets(); //|         ns/day =
      std::string timingLine(ptr);
      Cols timing;
      timing.Split(timingLine, "| =\r\n");
      std::string nspd = timing.GetKey("ns/day");
      if (!nspd.empty())
        currentStat.Set_NsPerDay().SetVal( convertToDouble( nspd ) );
      break;
    }
    ptr = mdout.Gets();
  }
  mdout.Close();

  if (completed) {
    currentStat.Set_Status( RunStatus::COMPLETE );
    if (current_nsteps < 0)
      currentStat.Set_CurrentNsteps( currentStat.Opts().N_Steps().Val() );
  } else
    currentStat.Set_Status( RunStatus::INCOMPLETE );

  // DEBUG
  //currentStat.Opts().PrintOpts(false, -1, -1);

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
  std::string output_name;
  std::string traj_name;
  bool has_input = false;
  bool has_runscript = false;
  bool has_submitscript = false;

  std::string const& runscriptname = CommonOptions::Opt_RunScriptName().Val();

  // Scan through files. Determine what is here.
  for (std::vector<std::string>::const_iterator fname = files.begin();
                                                fname != files.end(); ++fname)
  {
    if (CommonOptions::IsSubmitScript(*fname))
      has_submitscript = true;
    else if (*fname == runscriptname)
      has_runscript = true;
    else if (*fname == "md.in")
      // MD input
      has_input = true;
    else if (*fname == "INPUT") {
      // REMD input
      FileRoutines::StrArray remd_infiles = FileRoutines::ExpandToFilenames("INPUT/*", false);
      if (!remd_infiles.empty())
        has_input = true;
    } else if (*fname == "md.out")
      // MD output
      output_name = *fname;
    else if (*fname == "OUTPUT") {
      // REMD output
      FileRoutines::StrArray remd_outfiles = FileRoutines::ExpandToFilenames("OUTPUT/*", false);
      if (!remd_outfiles.empty()) {
        output_name = remd_outfiles.front();
      }
    } else if (*fname == "mdcrd.nc")
      // MD trajectory
      traj_name = *fname;
    else if (*fname == "TRAJ") {
      // REMD trajectory
      FileRoutines::StrArray remd_trajfiles = FileRoutines::ExpandToFilenames("TRAJ/*", false);
      if (!remd_trajfiles.empty()) {
        traj_name = remd_trajfiles.front();
      }
    }
  } // END loop over files

  // Determine status
  RunStatus currentStat;
  std::string top_name;
  if (!output_name.empty()) {
    if (read_mdout( currentStat, output_name, top_name )) {
      ErrorMsg("Could not read '%s'\n", output_name.c_str());
    }
  }

  if (!top_name.empty() && !traj_name.empty()) {
    if (read_traj_nframes( currentStat, top_name, traj_name )) {
      ErrorMsg("Could not read # frames from '%s'\n", traj_name.c_str());
    }
  }

  if (currentStat.CurrentStat() == RunStatus::UNKNOWN) {
    if (has_input && has_runscript && output_name.empty()) {
      if (has_submitscript)
        currentStat.Set_Status(RunStatus::READY);
      else
        currentStat.Set_Status(RunStatus::PENDING);
    }
  }
  //DEBUG
  //currentStat.Opts().PrintOpts(false, -1, -1);
  return currentStat;
}
