#include "Commands.h"
#include "Manager.h"
#include "Cols.h"
#include "Messages.h"
#include <readline.h>
#include <history.h>

using namespace Messages;

/** Process given command. */
Commands::RetType Commands::ProcessCommand(std::string const& inp) {
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
int Commands::Prompt(Manager& manager) {
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
