#include "Run.h"

/** \return Description of given type. */
const char* Run::typeStr(Type t) {
  const char* ptr = 0;
  switch (t) {
    case UNKNOWN   : ptr = "Unknown"; break;
    case REMD      : ptr = "REMD"   ; break;
    case MULTI_MD  : ptr = "MultipleMD"; break;
    case SINGLE_MD : ptr = "SingleMD"; break;
  }
  return ptr;
}

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
