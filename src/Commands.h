#ifndef INC_COMMANDS_H
#define INC_COMMANDS_H
#include <string>
#include "Exec.h"
class Manager;
/// Handle interactive prompt commands
namespace Commands {
  /// Initialize all commands
  void InitCommands();
  /// Process command
  Exec::RetType ProcessCommand(std::string const&, Manager&);
  /// Manager command prompt
  int Prompt(Manager&);
}
#endif
