#include "Run_REMD.h"
#include "Messages.h"
#include "FileRoutines.h"

using namespace Messages;

/** CONSTRUCTOR */
Run_REMD::Run_REMD() : Run(REMD) {}

/** COPY CONSTRUCTOR */
Run_REMD::Run_REMD(Run_REMD const& rhs) : Run(rhs) {

}

/** ASSIGNMENT */
Run_REMD& Run_REMD::operator=(Run_REMD const& rhs) {
  if (&rhs == this) return *this;
  Run::operator=(rhs);

  return *this;
}

/** <Internal setup description> */
int Run_REMD::InternalSetup(FileRoutines::StrArray const& output_files)
{

  return 1;
}

/** Print info for run. */
void Run_REMD::RunInfo() const {

}

/** Create run directory. */
int Run_REMD::CreateRunDir(Creator const& creator, int start_run, int run_num, std::string const& run_dir)
const
{
  using namespace FileRoutines;
  typedef std::vector<unsigned int> Iarray;
  // Create and change to run directory.
  if (Mkdir(run_dir)) return 1;
  if (ChangeDir(run_dir)) return 1;
  // Ensure that coords directory exists.
  if (crdDirSpecified_ && !fileExists(crd_dir_)) {
    ErrorMsg("Coords directory '%s' not found. Must specify absolute path"
             " or path relative to '%s'\n", crd_dir_.c_str(), run_dir.c_str());
    return 1;
  }
  // If constant pH, ensure CPIN file exists
  if (ph_dim_ != -1 && !fileExists(cpin_file_)) {
    ErrorMsg("CPIN file '%s' not found. Must specify absolute path"
             " or path relative to '%s'\n", cpin_file_.c_str(), run_dir.c_str());
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
    if (ph_dim_ != -1)
      MDIN.Printf("    solvph = %f,\n", Dims_[ph_dim_]->SolvPH( Indices[ph_dim_] ));
    MDIN.Printf("    temp0 = %f, tempi = %f,\n%s", currentTemp0, currentTemp0,
                additionalInput_.c_str());
    for (unsigned int id = 0; id != Dims_.size(); id++)
      Dims_[id]->WriteMdin(Indices[id], MDIN);
    MDIN.Printf(" &end\n");
    // Add any additional namelists
    for (MdinFile::const_iterator nl = mdinFile_.nl_begin(); nl != mdinFile_.nl_end(); ++nl)
      if (nl->first != "&cntrl")
        WriteNamelist(MDIN, nl->first, nl->second);
    MDIN.Close();
    // Write to groupfile. Determine restart.
    std::string INPUT_CRD;
    if (crdDirSpecified_ || run_num == 0)
      INPUT_CRD = crd_dir_ + "/" + EXT + ".rst7";
    else
      INPUT_CRD = "../run." + integerToString(run_num-1, width) +
                  "/RST/" + EXT + ".rst7";
    if (start_run == run_num && !fileExists( INPUT_CRD )) {
      // Check if crd_dir_ exists by itself
      if (fileExists(crd_dir_) && IsDirectory(crd_dir_)==0) {
        Msg("\tUsing '%s' for all input coordinates.\n", crd_dir_.c_str());
        INPUT_CRD = crd_dir_;
      } else {
        ErrorMsg("Coords %s not found.\n", INPUT_CRD.c_str());
        return 1;
      }
    }
    if (debug_ > 1)
      Msg("\t\tINPCRD: %s\n", INPUT_CRD.c_str());
    std::string GROUPFILE_LINE = "-O -remlog rem.log -i " + mdin_name +
      " -p " + currentTop + " -c " + INPUT_CRD + " -o OUTPUT/rem.out." + EXT +
      " -inf INFO/reminfo." + EXT + " -r RST/" + EXT + 
      ".rst7 -x TRAJ/rem.crd." + EXT;
    std::string repRef = RefFileName(EXT);
    if (!repRef.empty()) {
      if (!fileExists( repRef )) {
        ErrorMsg("Reference file '%s' not found. Must specify absolute path"
                 " or path relative to '%s'\n", repRef.c_str(), run_dir.c_str());
        return 1;
      }
      GROUPFILE_LINE.append(" -ref " + tildeExpansion(repRef));
    }
    if (uselog_)
      GROUPFILE_LINE.append(" -l LOG/logfile." + EXT);
    if (ph_dim_ != -1) {
      if (run_num == 0)
        GROUPFILE_LINE.append(" -cpin " + cpin_file_);
      else {
        // Use CPrestart from previous run
        std::string prevCP("../run." + integerToString(run_num-1, width) +
                           "/CPH/cprestrt." + EXT);
        if (start_run == run_num && !fileExists( prevCP )) {
          ErrorMsg("Previous CP restart %s not found.\n", prevCP.c_str());
          return 1;
        }
        GROUPFILE_LINE.append(" -cpin " + prevCP);
      }
      GROUPFILE_LINE.append(" -cpout CPH/cpout." + EXT +
                            " -cprestrt CPH/cprestrt." + EXT);
    }
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
  else if (runType_ == PHREMD)
    cmd_opts.assign("-ng " + NG + " -groupfile " + groupfileName_ + " -rem 4");
  else
    cmd_opts.assign("-ng " + NG + " -groupfile " + groupfileName_ + " -rem 1");
  if (WriteRunMD( cmd_opts )) return 1;
  // Create output directories
  if (Mkdir( "OUTPUT" )) return 1;
  if (Mkdir( "TRAJ"   )) return 1;
  if (Mkdir( "RST"    )) return 1;
  if (Mkdir( "INFO"   )) return 1;
  if (Mkdir( "LOG"    )) return 1;
  if (ph_dim_ != -1) {
    if (Mkdir( "CPH" )) return 1;
  }
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
