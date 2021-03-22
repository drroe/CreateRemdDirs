#ifndef INC_COMMANDS_H
#define INC_COMMANDS_H
#include <string>
class Manager;
/// Handle interactive prompt commands
namespace Commands {
  enum RetType { OK = 0, ERR, QUIT };
  /// Process command
  RetType ProcessCommand(std::string const&);
  /// Manager command prompt
  int Prompt(Manager&);
}
#endif
