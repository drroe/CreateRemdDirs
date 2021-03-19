#ifndef INC_MANAGER_H
#define INC_MANAGER_H
#include <string>
#include <vector>
#include "System.h"
/// Class to manage runs
class Manager {
  public:
    Manager();
    /// Initialize with current directory and systems file name
    int InitManager(std::string const&, std::string const&);
    /// Manager command prompt
    int Prompt();
  private:
    enum RetType { OK = 0, ERR, QUIT };
    /// Process command
    RetType ProcessCommand(std::string const&);

    std::vector<System> systems_; ///< Hold all systems in the systems file
    std::string topDir_;          ///< The current (top) working directory
};
#endif
