#include "Cols.h"
#include <cstring>
#include "StringRoutines.h"
#include "Messages.h"

using namespace StringRoutines;
using namespace Messages;

/** CONSTRUCTOR */
Cols::Cols() {}

/** Columnize line */
int Cols::Split(std::string const& inputString, const char* SEP) {
  columns_.clear();
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
      RemoveAllWhitespace(elt);
      columns_.push_back( elt );
      marked_.push_back( false );
      pch = strtok(0, SEP);
    }
  }
  delete[] tempString;
  return 0;
}

/** \return Unmarked column next to unmarked key. */
std::string Cols::GetKey(std::string const& key) {
  for (unsigned int idx = 0; idx != columns_.size(); idx++) {
    if (!marked_[idx] && key == columns_[idx]) {
      marked_[idx] = true;
      idx++;
      if (idx < columns_.size()) {
        marked_[idx] = true;
        return columns_[idx];
      } else {
        break;
      }
    }
  }
  return std::string("");
}

/** \return true if key is present, mark it. */
bool Cols::HasKey(std::string const& key) {
  for (unsigned int idx = 0; idx != columns_.size(); idx++) {
    if (!marked_[idx] && key == columns_[idx]) {
      marked_[idx] = true;
      return true;
    }
  }
  return false;
}

/** \param ival Set to integer value of unmarked column next to unmarked key (or defaultVal).
  * \return 1 if key was not a valid integer, 0 otherwise.
  */
int Cols::GetKeyInteger(int& ival, std::string const& key, int defaultVal) {
  std::string arg = GetKey( key );
  if (!arg.empty()) {
    if (validInteger(arg)) {
      ival = convertToInteger( arg );
      return 0;
    } else {
      ErrorMsg("%s is not a valid integer.\n", arg.c_str());
      return 1;
    }
  }
  ival = defaultVal;
  return 0;
}

/** \return Next unmarked string. */
std::string Cols::NextColumn() {
  for (unsigned int idx = 0; idx != columns_.size(); idx++) {
    if (!marked_[idx]) {
      marked_[idx] = true;
      return columns_[idx];
    }
  }
  return std::string("");
}
