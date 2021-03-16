#include "StringRoutines.h"
#include <cmath>
#include <sstream>

// DigitWidth()
/** \return the number of characters necessary to express the given digit. */
int DigitWidth(long int numberIn) {
  if (numberIn == 0L) return 1;
  double numf = (double) numberIn;
  numf = log10( numf );
  ++numf;
  // The cast back to long int implicitly rounds down
  int numi = (int)numf;
  return numi;
}

// integerToString()
std::string integerToString(int i) {
  std::ostringstream oss;
  oss << i;
  return oss.str();
}

// integerToString()
std::string integerToString(int i, int width) {
  std::ostringstream oss;
  oss.fill('0');
  oss.width( width );
  oss << std::right << i;
  return oss.str();
}

std::string doubleToString(double d) {
  std::ostringstream oss;
  oss << d;
  return oss.str();
}

// RemoveTrailingWhitespace()
/// Remove any trailing whitespace from string.
void RemoveTrailingWhitespace(std::string &line) {
  if (line.empty()) return;
  std::locale loc;
  int p = (int)line.size() - 1;
  while (p > -1 && (isspace(line[p],loc) || line[p]=='\n' || line[p]=='\r'))
    --p;
  line.resize(p + 1);
}

std::string NoTrailingWhitespace(std::string const& line) {
  std::string duplicate(line);
  RemoveTrailingWhitespace(duplicate);
  return duplicate;
}

/// Remove all whitespace from string.
void RemoveAllWhitespace(std::string& line) {
  if (line.empty()) return;
  std::string tmp( line );
  line.clear();
  for (std::string::const_iterator it = tmp.begin(); it != tmp.end(); ++it) {
    if (isspace(*it) || *it == '\n' || *it == '\r') continue;
    line += *it;
  }
}
