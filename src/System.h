#ifndef INC_SYSTEM_H
#define INC_SYSTEM_H
#include <string>
//class Run;
/// Hold information on runs for a system
class System {
  public:
    System();

    System(std::string const&, std::string const&);

    System(System const&);

    System& operator=(System const&);
  private:
    std::string dirname_;     ///< Directory containin runs for the system
    std::string description_; ///< Description of the system
};
#endif
