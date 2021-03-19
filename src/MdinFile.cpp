#include "MdinFile.h"
#include "TextFile.h"
#include "Messages.h"
#include "Cols.h"
#include "StringRoutines.h"

using namespace Messages;

/** CONSTRUCTOR */
MdinFile::MdinFile() {}

MdinFile::StatType MdinFile::TokenizeLine(TokenArray& Tokens, std::string const& inputString, std::string& namelist)
{
  static const char* separator = ",";
  if (inputString.empty()) 
    return EMPTY_LINE;

  Cols cols;
  if (cols.Split(inputString, separator)) return EMPTY_LINE;

  // Loop over tokens
  for (Cols::const_iterator col = cols.begin(); col != cols.end(); ++col)
  {
      //Msg("DEBUG: elt='%s'\n", elt.c_str());
      if (!col->empty()) {
        if ((*col)[0] == '&') {
          // Namelist beginning or end
          if (*col == "&end") {
            namelist.clear();
            return NAMELIST_END;
          } else {
            namelist.assign(*col);
            return NEW_NAMELIST;
          }
        } else {
          size_t found = col->find_last_of("=");
          if (found == std::string::npos) {
            ErrorMsg("Namelist token does not contain '=': %s\n", col->c_str());
            return ERR;
          }
          std::string varname = col->substr(0, found);
          std::string valname = col->substr(found+1);
          Tokens.push_back( TokenType(varname, valname) );
        }
      }
  }

  return OK;
}

/** Add given line to current namelist. */
int MdinFile::AddToNamelist(std::string const& line) {
  TokenArray Tokens;
  StatType stat = TokenizeLine(Tokens, line, currentNamelist_);

  if (stat == ERR) return 1;

  if (currentNamelist_.empty()) {
    //Msg("DEBUG: Current name list is empty.\n");
    return 0;
  }

  // Get current name list
  NLmap::iterator it = NameLists_.lower_bound( currentNamelist_ );
  if (it == NameLists_.end() || it->first != currentNamelist_ )
  {
    //Msg("DEBUG: New namelist: %s\n", currentNamelist_.c_str());
    it = NameLists_.insert(it, NLpair(currentNamelist_, TokenArray()));
  }
  // Add tokens to current name list
  for (TokenArray::const_iterator tkn = Tokens.begin(); tkn != Tokens.end(); ++tkn)
    it->second.push_back( *tkn );
  return 0;
}

/** Parse input from MDIN file. */
int MdinFile::ParseFile(std::string const& fname) {
  NameLists_.clear();
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
    StringRoutines::RemoveAllWhitespace( line2 );
    if (line2 == "&cntrl") {
      is_full_mdin = true;
      currentNamelist_.assign("&cntrl");
    }
  }

  if (!is_full_mdin) {
    Msg("\tNot a full MDIN, assume only &cntrl namelist variables present.\n");
    currentNamelist_.assign("&cntrl");
    // Need to parse those first two lines
    if (AddToNamelist( line1 )) return 1;
    if (AddToNamelist( line2 )) return 1;
  } else {
    Msg("\tFull MDIN.\n");
    if (currentNamelist_ != "&cntrl")
      Msg("Warning: Current namelist is not &cntrl.\n");
  }

  buffer = MDIN.Gets();
  while ( (buffer != 0) ) {
    if (AddToNamelist( std::string( buffer ) )) return 1;
    buffer = MDIN.Gets();
  }

  MDIN.Close();
  return 0;
}

/** Print namelists and variables to STDOUT. */
void MdinFile::PrintNamelists() const {
  for (NLmap::const_iterator nl = NameLists_.begin(); nl != NameLists_.end(); ++nl)
  {
    Msg("\tMDIN NameList: %s\n", nl->first.c_str());
    for (TokenArray::const_iterator it = nl->second.begin(); it != nl->second.end(); ++it)
      Msg("\t\t%s = %s\n", it->first.c_str(), it->second.c_str());
  }
}

/** \return Value for variable in specified namelist. */
std::string MdinFile::GetNamelistVar(std::string const& namelist, std::string const& varname)
const
{
  std::string valname;
  NLmap::const_iterator it = NameLists_.find( namelist );
  if (it == NameLists_.end()) {
    Msg("Warning: Namelist '%s' not found.\n", namelist.c_str());
    return valname;
  }

  for (TokenArray::const_iterator tkn = it->second.begin(); tkn != it->second.end(); ++tkn)
  {
    if (tkn->first == varname) {
      valname = tkn->second;
      break;
    }
  }
  return valname;
}
