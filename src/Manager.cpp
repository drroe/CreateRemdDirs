#include "Manager.h"
#include "Messages.h"
#include "TextFile.h"
#include "Cols.h"
#include <cstdlib> // free
#include <readline.h>
#include <history.h>

using namespace Messages;

/** CONSTRUCTOR */
Manager::Manager() {}

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
        if ( input.Token(0) == "system" ) {
          // Expect system <system dir>, <description>
          if (ncols < 3) {
            std::string errline;
            for (int col = 1; col < ncols; col++)
              errline.append(" "+input.Token(col));
            ErrorMsg("Not enough columns for 'system': %s\n", errline.c_str());
            return 1;
          }
          // All columns beyond the first are description
          std::string description = input.Token(2);
          for (int col = 3; col < ncols; col++)
            description.append(" " + input.Token(col));
          Msg("System: %s  Description: '%s'\n", input.Token(1).c_str(), description.c_str());
          systems_.push_back( System(CurrentDir, input.Token(1), description) );
          if (systems_.back().FindRuns()) return 1;
        }
      }
    }
    ncols = input.GetColumns(SEP);
  }
  input.Close();

  return 0;
}

/** Process given command. */
Manager::RetType Manager::ProcessCommand(std::string const& inp) {
  Msg("[%s]\n", inp.c_str());

  Cols line;
  line.Split(inp, " \n\r");

  std::string cmd = line.NextColumn();
  if (cmd == "q" || cmd == "quit")
    return QUIT;
  else {
    Msg("Warning: Unrecognized command: %s\n", cmd.c_str());
    return ERR;
  }
  return OK;
}

/** Command-line prompt for manager mode. */
int Manager::Prompt() {
  bool getInput = true;
  RetType ret = OK;
  int nerr = 0;
  while (getInput) {
    char* line = readline("> ");
    if (line == 0)
      // EOF
      getInput=false;
    else {
      std::string inp( line );
      // Add line to history TODO avoid blank lines
      add_history( inp.c_str() );
      ret = ProcessCommand( inp );
      free(line);
      if (ret == QUIT) 
        getInput = false;
      else if (ret == ERR)
        nerr++;
    }
  }
  return nerr;
}
