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

    Run(Type t) : type_(t) {}
    /// \return Description of given type
    static const char* typeStr(Type);
    /// \return Detected run type base on output files in the run directory.
    static Type DetectType(FileRoutines::StrArray&);

    void SetRunDir(std::string const& r) { rundir_ = r; }
  private:
    Type type_;
    std::string rundir_; ///< Run directory
};
#endif
