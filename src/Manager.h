#ifndef INC_MANAGER_H
#define INC_MANAGER_H
#include <string>
#include <vector>
#include "System.h"
/// Class to manage runs
class Manager {
  public:
    Manager();

    int InitManager(std::string const&);
  private:
    std::vector<System> systems_;
};
#endif
