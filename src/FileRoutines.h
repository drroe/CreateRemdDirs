#ifndef INC_FILEROUTINES_H
#define INC_FILEROUTINES_H
#include <string>
#include <vector>
std::string tildeExpansion(std::string const&);
typedef std::vector<std::string> StrArray;
StrArray ExpandToFilenames(std::string const&);
bool fileExists(std::string const&);
int CheckExists(const char*, std::string const&);
int Mkdir(std::string const&);
std::string GetWorkingDir();
int ChangeDir(std::string const&);
#endif
