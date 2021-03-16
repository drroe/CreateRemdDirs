#include "MdinFile.h"
#include "TextFile.h"
#include "Messages.h"
#include "StringRoutines.h"

/** CONSTRUCTOR */
MdinFile::MdinFile() {}

/** Parse input from MDIN file. */
int MdinFile::ParseFile(std::string const& fname) {
  TextFile MDIN;

  if (MDIN.OpenRead(fname)) {
    ErrorMsg("Could not open MDIN file '%s'\n", fname.c_str());
    return 1;
  }
  // Read the first two lines. If the second line contains &cntrl, assume 
  // we have been given a full MDIN file. If not, assume the file only
  // contains entries that belong in the &cntrl namelist.
  bool is_full_mdin = false;
  const char* buffer = MDIN.Gets();
  if (buffer == 0) {
    ErrorMsg("Nothing in MDIN file '%s'\n", fname.c_str());
    return 1;
  }
  std::string line1(buffer);
  std::string line2;
  buffer = MDIN.Gets();
  if (buffer != 0) {
    line2 = std::string(buffer);
    RemoveAllWhitespace( line2 );
    if (line2 == "&cntrl")
      is_full_mdin = true;
  }
  

  //while ( (buffer = MDIN.Gets()) != 0) {
  return 0;
}
