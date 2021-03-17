#ifndef INC_CHECKRUNS_H
#define INC_CHECKRUNS_H
#include <string>
#include "FileRoutines.h" // StrArray
class CheckRuns {
  public:
    CheckRuns();
    /// Check runs in given directory with given subdirectories; optionally only checking first run
    int DoCheck(std::string const&, StrArray const&, bool);
  private:
    enum RunType { UNKNOWN = 0, REMD, SINGLE_MD, MULTI_MD };
#   ifdef HAS_NETCDF
    static int checkNCerr(int);
    static int GetDimInfo(int, const char*, int&);
    static void CompareStrArray(StrArray const&, StrArray const&);
#   endif
    /// \return Extension of given file name.
    static std::string Ext(std::string const&);
};
#endif
