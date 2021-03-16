#ifndef INC_STRINGROUTINES_H
#define INC_STRINGROUTINES_H
#include <string>
// Functions for creating fixed-width digit strings.
int DigitWidth(long int);
std::string integerToString(int);
std::string integerToString(int,int);
std::string doubleToString(double);
/// Remove any trailing whitespace from string.
void RemoveTrailingWhitespace(std::string &);
/// \return string stripped of trailing whitespace.
std::string NoTrailingWhitespace(std::string const&);
/// Remove all whitespace from a string
void RemoveAllWhitespace(std::string&);
#endif
