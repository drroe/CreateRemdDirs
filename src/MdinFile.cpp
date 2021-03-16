#include <cstring>
#include "MdinFile.h"
#include "TextFile.h"
#include "Messages.h"
#include "StringRoutines.h"

/** CONSTRUCTOR */
MdinFile::MdinFile() {}

MdinFile::TokenArray MdinFile::TokenizeLine(std::string const& inputString) {
  static const char* separator = ",";
  TokenArray Tokens;
  if (inputString.empty()) 
    return Tokens;

  typedef std::vector<std::string> Sarray;
  Sarray Pairs;

  // Copy inputString to temp since it is destroyed by tokenize.
  size_t inputStringSize = inputString.size();
  if (inputStringSize < 1) return Tokens;
  char* tempString = new char[ inputStringSize+1 ];
  inputString.copy( tempString, inputStringSize, 0 );
  tempString[ inputStringSize ] = '\0'; // copy() does not append null 
  // Remove newline char from tempString if present
  if ( tempString[ inputStringSize - 1 ] == '\n' )
    tempString[ inputStringSize - 1 ] = '\0';

  // Begin tokenization
  char* pch = strtok(tempString, separator);
  if (pch != 0) {
    while (pch != 0) {
      Pairs.push_back( std::string(pch) );
      pch = strtok(0, separator);
    }
  }

  Msg("DEBUG: Tokens:\n");
  for (Sarray::const_iterator it = Pairs.begin(); it != Pairs.end(); ++it)
    Msg("\t%s\n", it->c_str());

  return Tokens;
}

/** Parse input from MDIN file. */
int MdinFile::ParseFile(std::string const& fname) {
  TextFile MDIN;

  if (MDIN.OpenRead(fname)) {
    ErrorMsg("Could not open MDIN file '%s'\n", fname.c_str());
    return 1;
  }
  // Read the first two lines. If the second line contains &cntrl, assume 
  // we have been given a full MDIN file. If not, assume the file only
  // contains entries that belong in the &cntrl namelist.
  bool is_full_mdin = false;
  const char* buffer = MDIN.Gets();
  if (buffer == 0) {
    ErrorMsg("Nothing in MDIN file '%s'\n", fname.c_str());
    return 1;
  }
  std::string line1(buffer);
  std::string line2;
  buffer = MDIN.Gets();
  if (buffer != 0) {
    line2 = std::string(buffer);
    RemoveAllWhitespace( line2 );
    if (line2 == "&cntrl")
      is_full_mdin = true;
  }
  

  //while ( (buffer = MDIN.Gets()) != 0) {
  return 0;
}
