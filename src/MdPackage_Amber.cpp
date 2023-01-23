#include "MdPackage_Amber.h"
#include "FileRoutines.h"
#include "Messages.h"
#include "TextFile.h"
#include "RepIndexArray.h"
#include "MdOptions.h"

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

/** Read amber-specific input from MDIN file. */
int MdPackage_Amber::ReadInputOptions(std::string const& fname) {
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
int MdPackage_Amber::WriteMdInputFile(std::string const& runDescription,
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

int MdPackage_Amber::create_singleMD(std::string const& crd_file,
                                     std::string const& top_file,
                                     std::string const& ref_file)
const
{
  using namespace FileRoutines;
  // Create and change to run directory. // FIXME NEEDS TO BE DONE BEFORE CALLING
//  if (Mkdir(run_dir)) return 1;
//  if (ChangeDir(run_dir)) return 1;
  // Get input coordinates array
  //Creator::Sarray crd_files = creator.InputCoordsNames(start_run, run_num, prevDir);
  if (crd_file.empty()) {
    ErrorMsg("Could not get input coords for MD.\n");
    return 1;
  }
  // Ensure topology exists.
  //std::string top_file = creator.TopologyName();
  if (top_file.empty()) {
    ErrorMsg("Could not get topology file name.\n");
    return 1;
  }
  // Get reference coords if any
  //Creator::Sarray ref_files = creator.RefCoordsNames();

  // Set up run command 
  std::string cmd_opts;
  cmd_opts.assign("-i md.in -p " + top_file + " -c " + crd_file + 
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
  if (!ref_file.empty())
    cmd_opts.append(" -ref " + ref_file);

  creator.WriteRunMD( cmd_opts );
  // Info for this run.
  if (Debug() >= 0) // 1 
      Msg("\tMD: top=%s\n", top_file.c_str());
      //Msg("\tMD: top=%s  temp0=%f\n", top_file.c_str(), temp0_);
  // Create input for non-umbrella runs.
  //if (creator.UmbrellaWriteFreq() == 0) {
    if (creator.MakeMdinForMD("md.in", run_num, "")) return 1;
  //}
  // Input coordinates for next run will be restarts of this
  //crd_dir_ = "../" + run_dir + "/";
  //if (creator.N_MD_Runs() < 2) crd_dir_.append("mdrst.rst7");
  return 0;
}
