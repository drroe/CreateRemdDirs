#ifndef INC_CMD_H
#define INC_CMD_H
#include <vector>
#include <string>
class Exec;
/// Hold command object and associated keywords
/** NOTE: This class does NOT contain a destructor because otherwise object_
  *       might be freed when copying or assigning. Currently CmdList is
  *       responsible for freeing object_ memory.
  */
class Cmd {
    typedef std::vector< std::string > Sarray;
  public:
    /// CONSTRUCTOR
    Cmd() : object_(0) {}
    /// CONSTRUCTOR - take object pointer and keywords
    Cmd(Exec* obj, Sarray keys) : object_(obj), keywords_(keys) {}
    /// Free Exec
    void Clear();
  private:
    Exec* object_;
    Sarray keywords_;
};
#endif
