#ifndef INC_FILEROUTINES_H
#define INC_FILEROUTINES_H
#include <string>
#include <vector>
namespace FileRoutines {
/// Expand any tildes in the given filename
std::string tildeExpansion(std::string const&);
/// Array of strings
typedef std::vector<std::string> StrArray;
/// Expand the given wildcard expression to an array of file names; optionally print warnings.
StrArray ExpandToFilenames(std::string const&, bool);
/// Expand the given wildcard expression to an array of file names; print all warnings.
StrArray ExpandToFilenames(std::string const&);
/// \return True if the file can be opened
bool fileExists(std::string const&);
/// \return 0 if file (with specified description) exists.
int CheckExists(const char*, std::string const&);
/// \return 1 if the given file is actually a directory
int IsDirectory(std::string const&);
/// Create a directory with given name
int Mkdir(std::string const&);
/// Add '../' if given path is not an absolute path
std::string add_path_prefix(std::string const&);
/// \return the current directory
std::string GetWorkingDir();
/// Change to the specified directory
int ChangeDir(std::string const&);
/// Change given file permissions to 775, i.e. make the file executable
int ChangePermissions(std::string const&);
/// \return The current user name
std::string UserName();
/// \return portion of file name after final '.'
std::string Extension(std::string const&);
/// \return Base file name
std::string Basename(std::string const&);
}
#endif
