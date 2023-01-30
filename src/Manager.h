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
    /// \return true if no projects
    bool NoProjects() const { return projects_.empty(); }
    /// \return Top directory name
    std::string const& TopDirName() const { return topDir_; }
    /// \return Top directory name
    const char* topDirName() const { return topDir_.c_str(); }
    /// \return Global debug level
    int Debug() const { return debug_; }
    /// \return index of the active project
    int ActiveProjectIdx() const { return activeProjectIdx_; }
    /// Change to active system directory
    int ChangeToActiveSystemDir() const;
    /// \return True if there is an active project and system
    bool HasActiveProjectSystem() const;

    /// \return the active project
    Project& ActiveProject() { return projects_[activeProjectIdx_]; }
    /// \return the active system of active project
    System& ActiveProjectSystem() { return projects_[activeProjectIdx_].ActiveSystem(); }
    /// Modifiable project
    Project& Set_Project(int idx) { return projects_[idx]; }

    /// Set global debug level
    void SetDebug(int);
    /// Set active project and system indices
    int SetActiveProjectSystem(int, int);
  private:
    enum RetType { OK = 0, ERR, QUIT };

    ProjectArray projects_; ///< Hold all Projects from the systems file
    std::string topDir_;    ///< The current (top) working directory
    int debug_;             ///< Global debug level
    int activeProjectIdx_;  ///< The index of the active project in Project array
};
#endif
