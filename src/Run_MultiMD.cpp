#include "Run_MultiMD.h"
#include "Messages.h"
#include "FileRoutines.h"
#include "Creator.h"
#include "StringRoutines.h"
#include "TextFile.h"

using namespace Messages;

/** CONSTRUCTOR */
Run_MultiMD::Run_MultiMD() : Run(MULTI_MD) {}

/** COPY CONSTRUCTOR */
Run_MultiMD::Run_MultiMD(Run_MultiMD const& rhs) : Run(rhs) {

}

/** ASSIGNMENT */
Run_MultiMD& Run_MultiMD::operator=(Run_MultiMD const& rhs) {
  if (&rhs == this) return *this;
  Run::operator=(rhs);

  return *this;
}

/** <Internal setup description> */
int Run_MultiMD::InternalSetup(FileRoutines::StrArray const& output_files)
{

  return 1;
}

/** Print info for run. */
void Run_MultiMD::RunInfo() const {
  Msg("Multi MD info not yet implemented.\n"); // FIXME
}

/** Create run directory. */
int Run_MultiMD::CreateRunDir(Creator const& creator, int start_run, int run_num, std::string const& run_dir, std::string const& prevDir)
const
{
  using namespace FileRoutines;
  // Create and change to run directory.
  if (Mkdir(run_dir)) return 1;
  if (ChangeDir(run_dir)) return 1;
  // Get input coordinates array
  Creator::Sarray crd_files = creator.InputCoordsNames(run_dir, start_run, run_num, prevDir);
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
  Creator::Sarray ref_files = creator.RefCoordsNames( run_dir );
  // Set up run command 
  std::string cmd_opts;
  TextFile GROUP;

  if (GROUP.OpenWrite(creator.GroupfileName())) return 1;
  for (int grp = 1; grp <= creator.N_MD_Runs(); grp++) {
    std::string EXT = creator.NumericalExt(grp, creator.N_MD_Runs());//integerToString(grp, width);
    std::string mdin_name("md.in");
    if (creator.UmbrellaWriteFreq() > 0) {
      // Create input for umbrella runs
      mdin_name.append("." + EXT);
      if (creator.MakeMdinForMD(mdin_name, run_num, "." + EXT, run_dir)) return 1;
    }
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
  if (creator.UmbrellaWriteFreq() == 0) {
    if (creator.MakeMdinForMD("md.in", run_num, "",run_dir)) return 1;
  }

  return 1;
}
