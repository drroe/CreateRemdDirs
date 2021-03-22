#include "Commands.h"
#include "Manager.h"
#include "Cols.h"
#include "Cmd.h"
#include "CmdList.h"
#include "Messages.h"
#include <readline.h>
#include <history.h>
#include <cstdarg>
#include <cstdlib> // free, malloc
#include <cstring> // strcpy, strncmp
// ----- Commands -----
#include "Exec_Quit.h"

using namespace Messages;

// ----- For readline tab completion -------------
/// Array of command names for tab completion
static std::vector<const char*> names_ = std::vector<const char*>();

// duplicate_string()
static char* duplicate_string(const char* s) {
  char* r = (char*)malloc(strlen(s) + 1);
  strcpy(r, s);
  return r;
}

// command_generator()
/** Generator function for command completion.  STATE lets us know whether
  * to start from scratch; without any state (i.e. STATE == 0), then we
  * start at the top of the list.
  */
static char* command_generator(const char* text, int state) {
  static int list_index, len;
  const char *name;

  // If this is a new word to complete, initialize now. This includes
  // saving the length of TEXT for efficiency, and initializing the index
  // variable to 0.
  if (!state) {
    list_index = 0;
    len = strlen(text);
  }

  // Return the next name which partially matches from the command list.
  while ( (name = names_[list_index]) != 0 )
  {
    list_index++;
    if (strncmp(name, text, len) == 0)
      return (duplicate_string(name));
  }

  // If no names matched, then return NULL.
  return 0;
}

// command_completion()
static char** command_completion(const char* text, int start, int end) {
  char** matches = 0;
  // If this word is at the start of the line, assume it is a command.
  if (start == 0 || (strncmp(rl_line_buffer, "help ", 5)==0))
    matches = rl_completion_matches(text, command_generator);
  return matches;
}

// -----------------------------------------------
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
  // Tell the completer we want a crack first
  rl_attempted_completion_function = command_completion;
  // Add all commands
  AddCmd(new Exec_Quit(), 1, "quit");
  // Add null ptr to indicate end of command key addresses for readline 
  names_.push_back( 0 );
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