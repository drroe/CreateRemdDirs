#ifndef INC_PROJECT_H
#define INC_PROJECT_H
#include <vector>
#include <string>
#include "System.h"
/// Hold Systems pertaining to a single Project
class Project {
  public:
    typedef std::vector<System> SystemArray;
    /// CONSTRUCTOR - Default project
    Project() : pname_("Default"), activeSystemIdx_(0) {}
    /// CONSTRUCTOR - Project with name
    Project(std::string const& n) : pname_(n), activeSystemIdx_(0) {}
    /// \return Systems
    SystemArray const& Systems() const { return systems_; }
    /// Add system to project
    void AddSystem(System const& s) { systems_.push_back( s ); }
    /// \return Reference to last system
    System& LastSystem() { return systems_.back(); }
    /// \return project name
    const char* name() const { return pname_.c_str(); }
    /// \return index of active system
    int ActiveSystemIdx() const { return activeSystemIdx_; }
    /// \return Active system
    System& ActiveSystem() { return systems_[activeSystemIdx_]; }
    /// Set active system index
    void SetActiveSystem(int idx) { activeSystemIdx_ = idx; }
  private:
    SystemArray systems_; ///< Hold all systems pertaining to this Project
    std::string pname_;   ///< Project name
    int activeSystemIdx_; ///< The index of the active system in Systems array
};
#endif
