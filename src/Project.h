#ifndef INC_PROJECT_H
#define INC_PROJECT_H
#include <vector>
#include "System.h"
/// Hold Systems pertaining to a single Project
class Project {
  public:
    typedef std::vector<System> SystemArray;

    Project();

    int AddSystem(System const& s) { systems_.push_back( s ); }

    SystemArray const& Systems() const { return systems_; }
  private:
    SystemArray systems_; ///< Hold all systems pertaining to this Project
};
#endif
