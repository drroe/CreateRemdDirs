#include "Commands.h"
#include "Manager.h"
#include "Cols.h"
#include "Cmd.h"
#include "CmdList.h"
#include "Messages.h"
#include <readline.h>
#include <history.h>
#include <cstdarg>
// ----- Commands -----
#include "Exec_Quit.h"

using namespace Messages;

/// Array of command names for tab completion
static std::vector<const char*> names_ = std::vector<const char*>();

/// Master list of commands
static CmdList commands_ = CmdList();

const Cmd EMPTYCMD_ = Cmd();

/** \param oIn Pointer to Exec to add as command.
  * \param nKeys Number of command keywords associated with this command.
  * The remaining arguments are the nKeys command keywords.
  */
static void AddCmd(Exec* oIn, int nKeys, ...) {
  Cmd::Sarray keys;
  va_list args;
  va_start(args, nKeys);
  for (int nk = 0; nk < nKeys; nk++) {
    char* key = va_arg(args, char*);
    keys.push_back( std::string(key) );
  }
  va_end(args);
  commands_.Add( Cmd(oIn, keys) );
  // Store memory addresses of command keys for ReadLine
  for (Cmd::key_iterator key = commands_.Back().keysBegin();
                         key != commands_.Back().keysEnd(); ++key)
    names_.push_back( key->c_str() );
}


/** Initialize all commands. */
void Commands::InitCommands() {
  AddCmd(new Exec_Quit(), 1, "quit");
}

/** Search for command. */
static Cmd const& SearchToken(std::string const& cmdArg) {
  for (CmdList::const_iterator cmd = commands_.begin(); cmd != commands_.end(); ++cmd)
  {
    if ( cmd->KeyMatches( cmdArg ) )
      return *cmd;
  }
  return EMPTYCMD_;
}

/** Process given command. */
Exec::RetType Commands::ProcessCommand(std::string const& inp, Manager& manager) {
  Msg("[%s]\n", inp.c_str());

  Cols line;
  line.Split(inp, " \n\r");

  std::string cmdArg = line.NextColumn();

  // Look for command in command list.
  Cmd const& cmd = SearchToken( cmdArg );

  if (cmd.Empty()) {
    Msg("Warning: Unrecognized command: %s\n", cmdArg.c_str());
    return Exec::ERR;
  }

  // Execute command
  Exec::RetType retVal = cmd.CmdExec()->Execute( manager, line );
  return retVal;
}


/** Command-line prompt for manager mode. */
int Commands::Prompt(Manager& manager) {
  bool getInput = true;
  Exec::RetType ret = Exec::OK;
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
      ret = ProcessCommand( inp, manager );
      free(line);
      if (ret == Exec::QUIT) 
        getInput = false;
      else if (ret == Exec::ERR)
        nerr++;
    }
  }
  return nerr;
}
