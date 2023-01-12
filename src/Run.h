#ifndef INC_RUN_H
#define INC_RUN_H
#include <string>
#include "FileRoutines.h" // StrArray
class Creator;
/// Abstract base class for a run
class Run {
  public:
    enum Type { UNKNOWN = 0, REMD, MULTI_MD, SINGLE_MD };
    /// CONSTRUCTOR - type unknown
    Run() : type_(UNKNOWN), debug_(0) {}
    /// CONSTRUCTOR - set type
    Run(Type t) : type_(t), debug_(0) {}
    /// COPY CONSTRUCTOR
    Run(const Run& rhs) : type_(rhs.type_), rundir_(rhs.rundir_), setupDir_(rhs.setupDir_), debug_(rhs.debug_) {}
    /// ASSIGNMENT
    Run& operator=(const Run& rhs) {
      if (&rhs == this) return *this;
      type_ = rhs.type_;
      rundir_ = rhs.rundir_;
      setupDir_ = rhs.setupDir_;
      debug_ = rhs.debug_;
      return *this;
    }
    /// Set debug level
    void SetDebug(int d) { debug_ = d; }
    // -------------------------------------------
    /// Virtual because inherited
    virtual ~Run() {}
    /// Print info about run to stdout
    virtual void RunInfo() const = 0;
    /// \return Copy of run
    virtual Run* Copy() const = 0;
    /// Create run directory
    virtual int CreateRunDir(Creator const&, int, int, std::string const&) const = 0;
    // -------------------------------------------
    /// \return Description of given type
    static const char* typeStr(Type);
    /// \return Current debug level
    int Debug() const { return debug_; }

    /// \return Detected run type base on output files in the run directory.
    static Type DetectType(FileRoutines::StrArray&);
    /// Set up run based on given output file array
    int SetupRun(std::string const&, FileRoutines::StrArray const&);
    /// Set up run just using run dir name (run not yet complete)
    int SetupRun(std::string const&);
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
    int debug_; ///< Debug level
};
#endif
