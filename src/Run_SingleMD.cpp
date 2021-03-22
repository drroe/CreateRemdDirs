#include "Run_SingleMD.h"
#include "Messages.h"

/** CONSTRUCTOR */
Run_SingleMD::Run_SingleMD() :
  Run(SINGLE_MD)
{}

/** Set up single MD run. */
int Run_SingleMD::InternalSetup(FileRoutines::StrArray const& output_files)
{
  Messages::Msg("DEBUG: Run dir path: %s\n", SetupDirName().c_str());
  if (output_files.size() != 1) {
    Messages::ErrorMsg("Run_SingleMD::InternalSetup: output files array size is not 1 (%zu)\n", output_files.size());
    return 1;
  }
  return sim_.SetupFromFiles( output_files.front(), "mdcrd.nc", "mdrst.rst7", 1 );
}

/** Print info for run. */
void Run_SingleMD::RunInfo() const {
  static const char* StatStr[2] = {"Incomplete", "Complete"};
  Messages::Msg("%-8s : %10s %12.4g %10i %10i\n",
                RunDirName().c_str(),
                StatStr[(int)sim_.Completed()],
                sim_.Total_Time(),
                sim_.Actual_Frames(),
                sim_.Expected_Frames());
}
