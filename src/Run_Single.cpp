#include "Run_Single.h"
#include "Messages.h"

/** CONSTRUCTOR */
Run_Single::Run_Single() :
  Run(SINGLE_MD)
{}

/** Set up single MD run. */
int Run_Single::InternalSetup(FileRoutines::StrArray const& output_files)
{
  if (output_files.size() != 1) {
    Messages::ErrorMsg("Run_Single::InternalSetup: output files array size is not 1 (%zu)\n", output_files.size());
    return 1;
  }
  return sim_.SetupFromFiles( output_files.front(), "mdcrd.nc", "mdrst.rst7", 1 );
}
