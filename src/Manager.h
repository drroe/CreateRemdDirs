#ifndef INC_MANAGER_H
#define INC_MANAGER_H
#include <string>
#include <vector>
#include "Project.h"
/// Class to manage runs in Systems
class Manager {
  public:
    typedef std::vector<Project> ProjectArray;

    Manager();
    /// Initialize with current directory and systems file name
    int InitManager(std::string const&, std::string const&);

    ProjectArray const& Projects() const { return projects_; }
    /// \return Top directory name
    const char* topDirName() const { return topDir_.c_str(); }
  private:
    enum RetType { OK = 0, ERR, QUIT };
    /// Process command
    RetType ProcessCommand(std::string const&);

    ProjectArray projects_; ///< Hold all Projects from the systems file
    std::string topDir_;    ///< The current (top) working directory
};
#endif
