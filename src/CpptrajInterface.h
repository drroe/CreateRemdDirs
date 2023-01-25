#ifndef INC_CPPTRAJINTERFACE_H
#define INC_CPPTRAJINTERFACE_H
#include <string>
/// Class used to interface with cpptraj via command line
class CpptrajInterface {
  public:
    /// CONSTRUCTOR
    CpptrajInterface();
    /// \return True if cpptraj is available
    bool Available() const;
    /// \return number of frames from top/traj combo
    int GetTrajFrames(std::string const&, std::string const&) const;
  private:
    std::string shellCmd(std::string const&) const;
};
#endif
