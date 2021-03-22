#ifndef INC_EXEC_H
#define INC_EXEC_H
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
    virtual RetType Execute(Manager&, Cols&) = 0;
    /// Print help for command
    virtual void Help() const = 0;
    /// Print help for command - takes arguments
    void Help(Cols&) const { Help(); }
};
#endif
