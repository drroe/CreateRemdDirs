#ifndef INC_NC_H
#define INC_NC_H
/// Namespace for netcdf-related functions
namespace NC {

/// \return 1 and print error message if given netcdf status is an error
int CheckErr(int);

/// \return Dimension ID for ncid attribute or -1 on error; also set dim length
int GetDimInfo(int, const char*, unsigned int&);

}
#endif
