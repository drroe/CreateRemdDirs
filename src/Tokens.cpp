#include "Tokens.h"
#include <cstring>
#include "StringRoutines.h"

/** CONSTRUCTOR */
Tokens::Tokens() {}

/** Tokenize line */
int Tokens::Tokenize(std::string const& inputString, const char* SEP) {
  tokens_.clear();
  marked_.clear();
  if (inputString.empty()) 
    return 0;

  // Copy inputString to temp since it is destroyed by tokenize.
  size_t inputStringSize = inputString.size();
  if (inputStringSize < 1) return 0;
  char* tempString = new char[ inputStringSize+1 ];
  inputString.copy( tempString, inputStringSize, 0 );
  tempString[ inputStringSize ] = '\0'; // copy() does not append null 
  // Remove newline char from tempString if present
  if ( tempString[ inputStringSize - 1 ] == '\n' )
    tempString[ inputStringSize - 1 ] = '\0';

  // Begin tokenization
  char* pch = strtok(tempString, SEP);
  if (pch != 0) {
    while (pch != 0) {
      std::string elt(pch);
      //Msg("DEBUG: elt='%s'\n", elt.c_str());
      StringRoutines::RemoveAllWhitespace(elt);
      tokens_.push_back( elt );
      marked_.push_back( false );
      pch = strtok(0, SEP);
    }
  }
  return 0;
}
