#ifndef INC_SYSTEM_H
#define INC_SYSTEM_H
#include <string>
//class Run;
/// Hold information on runs for a system
class System {
  public:
    /// CONSTRUCTOR
    System();
    /// CONSTRUCTOR - top directory, run directory, description
    System(std::string const&, std::string const&, std::string const&);
    /// COPY CONSTRUCTOR
    System(System const&);
    /// ASSIGNMENT
    System& operator=(System const&);

    /// Find runs in dirname_
    int FindRuns();
  private:
    std::string topDir_;      ///< Top level directory
    std::string dirname_;     ///< Directory containing runs for the system
    std::string description_; ///< Description of the system
};
#endif
