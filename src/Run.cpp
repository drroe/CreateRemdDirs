#include "Run.h"

/** Try to determine the run type based on the output files. */
Run::Type Run::DetectType(FileRoutines::StrArray& output_files) {
  Type runType = UNKNOWN;
  // Determine where the output files are. 
  output_files = FileRoutines::ExpandToFilenames("OUTPUT/rem.out.*", false);
  if (!output_files.empty()) {
    runType = REMD;
  } else {
    // Not REMD. Multiple MD?
    output_files = FileRoutines::ExpandToFilenames("md.out.*", false);
    if (!output_files.empty()) {
      runType = MULTI_MD;
    } else {
      if (FileRoutines::fileExists("md.out")) {
        output_files.push_back("md.out");
        runType = SINGLE_MD;
      }
    }
  }
  return runType;
}
