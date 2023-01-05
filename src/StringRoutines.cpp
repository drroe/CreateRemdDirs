#include "StringRoutines.h"
#include "Messages.h"
#include <cmath>
#include <sstream>

// DigitWidth()
/** \return the number of characters necessary to express the given digit. */
int StringRoutines::DigitWidth(long int numberIn) {
  if (numberIn == 0L) return 1;
  double numf = (double) numberIn;
  numf = log10( numf );
  ++numf;
  // The cast back to long int implicitly rounds down
  int numi = (int)numf;
  return numi;
}

// integerToString()
std::string StringRoutines::integerToString(int i) {
  std::ostringstream oss;
  oss << i;
  return oss.str();
}

// integerToString()
std::string StringRoutines::integerToString(int i, int width) {
  std::ostringstream oss;
  oss.fill('0');
  oss.width( width );
  oss << std::right << i;
  return oss.str();
}

// validInteger()
bool StringRoutines::validInteger(std::string const &argument) {
  if (argument.empty()) return false;
  std::string::const_iterator c;
  if (argument[0]=='-' || argument[0]=='+') {
    c = argument.begin()+1;
    if (c == argument.end()) return false;
  } else
    c = argument.begin();
  for (; c != argument.end(); ++c)
    if (!isdigit(*c)) return false;
  return true;
}

// convertToInteger()
/** Convert the input string to an integer. */
int StringRoutines::convertToInteger(std::string const &s) {
  std::istringstream iss(s);
  long int i;
  iss >> i;
  if (iss.fail()) {
    Messages::ErrorMsg("Could not convert '%s' to integer.\n", s.c_str());
    return 0;
  }
    //throw BadConversion("convertToInteger(\"" + s + "\")");
  return (int)i;
}


std::string StringRoutines::doubleToString(double d) {
  std::ostringstream oss;
  oss << d;
  return oss.str();
}

// RemoveTrailingWhitespace()
/// Remove any trailing whitespace from string.
void StringRoutines::RemoveTrailingWhitespace(std::string &line) {
  if (line.empty()) return;
  std::locale loc;
  int p = (int)line.size() - 1;
  while (p > -1 && (isspace(line[p],loc) || line[p]=='\n' || line[p]=='\r'))
    --p;
  line.resize(p + 1);
}

std::string StringRoutines::NoTrailingWhitespace(std::string const& line) {
  std::string duplicate(line);
  RemoveTrailingWhitespace(duplicate);
  return duplicate;
}

/// Remove all whitespace from string.
void StringRoutines::RemoveAllWhitespace(std::string& line) {
  if (line.empty()) return;
  std::string tmp( line );
  line.clear();
  for (std::string::const_iterator it = tmp.begin(); it != tmp.end(); ++it) {
    if (isspace(*it) || *it == '\n' || *it == '\r') continue;
    line += *it;
  }
}
