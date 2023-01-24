#ifndef INC_RUN_H
#define INC_RUN_H
#include <string>
/// Hold information for a single run
class Run {
  public:
    /// CONSTRUCTOR
    Run();
    /// \return Current debug level
    int Debug() const { return debug_; }
    /// \return Run index
    int RunIndex() const { return idx_; }
    /// \return run dir name
    std::string const& RunDirName() const { return rundir_; }

    
  private:
    std::string rundir_;   ///< Run directory. May be relative.
    std::string setupDir_; ///< Directory in which SetupRun gets invoked. Should be absolute run dir

    int idx_;              ///< Run index, based on directory name extension
    int debug_;            ///< Debug level
};
#endif
