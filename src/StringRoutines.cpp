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
  std::locale loc;

  std::string::iterator p = line.end();
  --p;
  for (; p != line.begin(); p--)
    if (!isspace( *p, loc) && *p!='\n' && *p!='\r') break;
  size_t lastSpace = (size_t)(p - line.begin()) + 1;
  //mprintf("lastSpace = %zu\n",lastSpace);
  if (lastSpace==1)
    line.clear();
  else
    line.resize( lastSpace );
}

std::string NoTrailingWhitespace(std::string const& line) {
  std::string duplicate(line);
  RemoveTrailingWhitespace(duplicate);
  return duplicate;
}
