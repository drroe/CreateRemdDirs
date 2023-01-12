#include "Run_SingleMD.h"
#include "Messages.h"
#include "FileRoutines.h"
#include "TextFile.h"
#include "Creator.h"

using namespace Messages;

/** CONSTRUCTOR */
Run_SingleMD::Run_SingleMD() :
  Run(SINGLE_MD)
{}

/** Set up single MD run. */
int Run_SingleMD::InternalSetup(FileRoutines::StrArray const& output_files)
{
  Msg("DEBUG: Run dir path: %s\n", SetupDirName().c_str());
  if (output_files.size() != 1) {
    ErrorMsg("Run_SingleMD::InternalSetup: output files array size is not 1 (%zu)\n", output_files.size());
    return 1;
  }
  return sim_.SetupFromFiles( output_files.front(), "mdcrd.nc", "mdrst.rst7", 1 );
}

/** Print info for run. */
void Run_SingleMD::RunInfo() const {
  static const char* StatStr[2] = {"Incomplete", "Complete"};
  Msg("%-8s : %10s %12.4g %10i %10i\n",
                RunDirName().c_str(),
                StatStr[(int)sim_.Completed()],
                sim_.Total_Time(),
                sim_.Actual_Frames(),
                sim_.Expected_Frames());
}

/** Create run directory. */
int Run_SingleMD::CreateRunDir(Creator const& creator,
                               int start_run, int run_num, std::string const& run_dir, std::string const& prevDir)
const
{
  using namespace FileRoutines;
  // Create and change to run directory.
  if (Mkdir(run_dir)) return 1;
  if (ChangeDir(run_dir)) return 1;
  // Get input coordinates array
  Creator::Sarray crd_files = creator.InputCoordsNames(run_dir, start_run, run_num);
  if (crd_files.empty()) {
    ErrorMsg("Could not get input coords for MD.\n");
    return 1;
  }
  // Ensure topology exists.
  if (!fileExists( creator.TopologyName() )) {
    ErrorMsg("Topology '%s' not found. Must specify absolute path"
             " or path relative to '%s'\n", creator.TopologyName().c_str(), run_dir.c_str());
    return 1;
  }
  // Get reference coords if any
  Creator::Sarray ref_files = creator.RefCoordsNames( run_dir );

  // Set up run command 
  std::string cmd_opts;
  cmd_opts.assign("-i md.in -p " + creator.TopologyName() + " -c " + crd_files.front() + 
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
      Msg("\tMD: top=%s\n", creator.TopologyName().c_str());
      //Msg("\tMD: top=%s  temp0=%f\n", creator.TopologyName().c_str(), temp0_);
  // Create input for non-umbrella runs.
  if (creator.UmbrellaWriteFreq() == 0) {
    if (creator.MakeMdinForMD("md.in", run_num, "",run_dir)) return 1;
  }
  // Input coordinates for next run will be restarts of this
  //crd_dir_ = "../" + run_dir + "/";
  //if (creator.N_MD_Runs() < 2) crd_dir_.append("mdrst.rst7");
  return 0;
}
