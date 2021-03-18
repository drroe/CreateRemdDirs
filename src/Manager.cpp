#include "Manager.h"
#include "Messages.h"
#include "TextFile.h"

Manager::Manager() {}

/** Initialize with input file. */
int Manager::InitManager(std::string const& inputFileName) {
  if (inputFileName.empty()) {
    ErrorMsg("No manager input file given.\n");
    return 1;
  }

  TextFile input;
  if (input.OpenRead(inputFileName)) {
    ErrorMsg("Could not open manager input file '%s'\n", inputFileName.c_str());
    return 1;
  }
  static const char* SEP = " \n\r";
  int ncols = input.GetColumns(SEP);
  while (ncols > -1) {
    if (ncols > 0) {
      if (input.Token(0)[0] != '#') {
        // Expect <system dir>, <description>
        if (ncols > 1) {
          // All columns beyond the first are description
          std::string description = input.Token(1);
          for (int col = 2; col < ncols; col++)
            description.append(" " + input.Token(col));
          Msg("System: %s  Description: '%s'\n", input.Token(0).c_str(), description.c_str());
        }
      }
    }
    ncols = input.GetColumns(SEP);
  }
  input.Close();

  return 0;
}
