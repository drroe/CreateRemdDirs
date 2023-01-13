#include "Run_REMD.h"
#include "Messages.h"
#include "FileRoutines.h"
#include "Creator.h"
#include "Groups.h"
#include "RepIndexArray.h"
#include "TextFile.h"
#include "ReplicaDimension.h"
#include "StringRoutines.h"

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
int Run_REMD::CreateRunDir(Creator const& creator, int start_run, int run_num, std::string const& run_dir, std::string const& prevDir)
const
{
  using namespace FileRoutines;
  using namespace StringRoutines;
  // Create and change to run directory.
  if (Mkdir(run_dir)) return 1;
  if (ChangeDir(run_dir)) return 1;
  // Get Coords
  Creator::Sarray crd_files = creator.InputCoordsNames(run_dir, start_run, run_num, prevDir);
  if (crd_files.empty()) {
    ErrorMsg("Could not get COORDS for REMD.\n");
    return 1;
  }
  // Get any ref coords
  Creator::Sarray ref_files = creator.RefCoordsNames(run_dir);
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
    // Ensure topology exists.
    if (!fileExists( currentTop )) {
      ErrorMsg("Topology '%s' not found. Must specify absolute path"
               " or path relative to '%s'\n", currentTop.c_str(), run_dir.c_str());
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
    if (creator.MakeMdinForMD(mdin_name, run_num,
                              EXT, run_dir, Indices, rep))
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
    if (creator.UseLog())
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
