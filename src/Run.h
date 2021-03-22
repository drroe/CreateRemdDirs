#ifndef INC_RUN_H
#define INC_RUN_H
#include <string>
#include "FileRoutines.h" // StrArray
/// Abstract base class for a run
class Run {
  public:
    enum Type { UNKNOWN = 0, REMD, MULTI_MD, SINGLE_MD };
    /// CONSTRUCTOR
    Run() : type_(UNKNOWN) {}
    /// Virtual because inherited
    virtual ~Run() {}

    virtual void RunInfo() const = 0;

    Run(Type t) : type_(t) {}
    /// \return Description of given type
    static const char* typeStr(Type);
    /// \return Detected run type base on output files in the run directory.
    static Type DetectType(FileRoutines::StrArray&);
    /// Set up run based on given output file array
    int SetupRun(std::string const&, FileRoutines::StrArray const&);
  protected:
    /// Every run type needs an internal setup from given output file array
    virtual int InternalSetup(FileRoutines::StrArray const&) = 0;
    /// \return setup directory
    std::string const& SetupDirName() const { return setupDir_; }
    /// \return run dir name
    std::string const& RunDirName() const { return rundir_; }
  private:
    Type type_;
    std::string rundir_;   ///< Run directory. May be relative.
    std::string setupDir_; ///< Directory in which SetupRun gets invoked. Should be absolute run dir.
};
#endif
