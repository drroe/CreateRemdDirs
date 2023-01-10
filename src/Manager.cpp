#include "Manager.h"
#include "Messages.h"
#include "TextFile.h"

using namespace Messages;

/** CONSTRUCTOR */
Manager::Manager() :
  debug_(0),
  activeProjectIdx_(0)
{}

/** Initialize with input file. */
int Manager::InitManager(std::string const& CurrentDir, std::string const& inputFileName) {
  if (inputFileName.empty()) {
    ErrorMsg("No manager input file given.\n");
    return 1;
  }
  topDir_ = CurrentDir;

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
        if ( input.Token(0) == "project" ) {
          // Expect project <name>
          if (ncols < 2) {
            std::string errline;
            for (int col = 1; col < ncols; col++)
              errline.append(" "+input.Token(col));
            ErrorMsg("Not enough columns for 'project': %s\n", errline.c_str());
            return 1;
          }
          // All columns beyond 0 are project name
          std::string description = input.Token(1);
          for (int col = 2; col < ncols; col++)
            description.append(" " + input.Token(col));
          Msg("Project: %s\n", description.c_str());
          projects_.push_back( Project(description) );
        } if ( input.Token(0) == "system" ) {
          // Expect system <system dir>, <description>
          if (ncols < 3) {
            std::string errline;
            for (int col = 1; col < ncols; col++)
              errline.append(" "+input.Token(col));
            ErrorMsg("Not enough columns for 'system': %s\n", errline.c_str());
            return 1;
          }
          // If no Project yet, add default
          if (projects_.empty())
            projects_.push_back( Project() );
          // All columns beyond the first are description
          std::string description = input.Token(2);
          for (int col = 3; col < ncols; col++)
            description.append(" " + input.Token(col));
          Msg("System: %s  Description: '%s'\n", input.Token(1).c_str(), description.c_str());
          projects_.back().AddSystem( System(CurrentDir, input.Token(1), description) );
          if (projects_.back().LastSystem().FindRuns()) return 1;
        }
      }
    }
    ncols = input.GetColumns(SEP);
  }
  input.Close();

  return 0;
}
