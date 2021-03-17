#ifndef INC_CHECKRUNS_H
#define INC_CHECKRUNS_H
#include <string>
#include "FileRoutines.h" // StrArray
class CheckRuns {
  public:
    CheckRuns();
    /// Check runs in given directory with given subdirectories; optionally only checking first run
    int DoCheck(std::string const&, StrArray const&, bool) const;
  private:
    enum RunType { UNKNOWN = 0, REMD, SINGLE_MD, MULTI_MD };
#   ifdef HAS_NETCDF
    static int checkNCerr(int);
    static int GetDimInfo(int, const char*, int&);
#   endif
    static void CompareStrArray(StrArray const&, StrArray const&);
    /// \return Extension of given file name.
    static std::string Ext(std::string const&);
    /// Check REMD restarts
    static int CheckRemdRestarts(StrArray const&);
};
#endif
