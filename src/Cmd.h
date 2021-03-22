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
  public:
    /// Array type for holding list of command keywords
    typedef std::vector< std::string > Sarray;
    /// CONSTRUCTOR
    Cmd() : object_(0) {}
    /// CONSTRUCTOR - take object pointer and keywords
    Cmd(Exec* obj, Sarray keys) : object_(obj), keywords_(keys) {}
    /// Free Exec
    void Clear();
    /// Iterator for command keywords.
    typedef Sarray::const_iterator key_iterator;
    /// \return iterator to beginning of command keywords.
    key_iterator keysBegin() const { return keywords_.begin(); }
    /// \return iterator to end of command keywords.
    key_iterator keysEnd()   const { return keywords_.end();   }
    /// \return true if given key matches any of this commands keywords.
    bool KeyMatches(const char*) const;
  private:
    Exec* object_;
    Sarray keywords_;
};
#endif
