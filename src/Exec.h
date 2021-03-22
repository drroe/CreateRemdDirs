#ifndef INC_EXEC_H
#define INC_EXEC_H
#include <string>
class Manager;
class Cols;
/// Abstract base class to hold logic to execute a command.
class Exec {
  public:
    Exec() {}
    // Virtual since inhertied
    virtual ~Exec() {}
    /// Various command return statuses
    enum RetType { OK=0, ERR, QUIT };

    /// Execute command
    virtual RetType Execute(Manager&, Cols&) const = 0;
    /// \return help for command
    virtual std::string Help() const = 0;

    /// Print help for command - takes arguments
    virtual std::string Help(Cols&) const { return Help(); }
};
#endif