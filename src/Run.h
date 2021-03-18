#ifndef INC_RUN_H
#define INC_RUN_H
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
  private:
    Type type_;
};
#endif
