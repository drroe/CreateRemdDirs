#ifndef INC_CPPTRAJINTERFACE_H
#define INC_CPPTRAJINTERFACE_H
#include <string>
/// Class used to interface with cpptraj via command line
class CpptrajInterface {
  public:
    /// CONSTRUCTOR
    CpptrajInterface();
  private:
    std::string shellCmd(std::string const&) const;
};
#endif
