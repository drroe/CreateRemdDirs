#ifndef INC_PROJECT_H
#define INC_PROJECT_H
#include <vector>
#include <string>
#include "System.h"
/// Hold Systems pertaining to a single Project
class Project {
  public:
    typedef std::vector<System> SystemArray;
    /// Default project
    Project() : pname_("Default") {}
    /// Project with name
    Project(std::string const& n) : pname_(n) {}
    /// \return Systems
    SystemArray const& Systems() const { return systems_; }
    /// Add system to project
    int AddSystem(System const& s) { systems_.push_back( s ); }
    /// \return Reference to last system
    System& LastSystem() { return systems_.back(); }
    /// \return project name
    const char* name() const { return pname_.c_str(); }
  private:
    SystemArray systems_; ///< Hold all systems pertaining to this Project
    std::string pname_;   ///< Project name
};
#endif
