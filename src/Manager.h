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
    /// List all current systems.
    int List() const;
  private:
    enum RetType { OK = 0, ERR, QUIT };
    /// Process command
    RetType ProcessCommand(std::string const&);

    typedef std::vector<System> SystemArray;

    SystemArray systems_; ///< Hold all systems in the systems file
    std::string topDir_;          ///< The current (top) working directory
};
#endif
