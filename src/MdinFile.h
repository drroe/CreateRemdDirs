#ifndef INC_MDIN_FILE
#define INC_MDIN_FILE
#include <string>
#include <vector>
#include <map>
/// Used to parse input from an Amber MDIN file
class MdinFile {
  public:
    MdinFile();

    int ParseFile(std::string const&);
  private:
    /// Pair namelist variable with value
    typedef std::pair<std::string, std::string> TokenType;
    /// Array of namelist variable/value pairs
    typedef std::vector<TokenType> TokenArray;
    /// Pair namelist name to token array
    typedef std::pair<std::string, TokenArray> NLpair;
    /// Map namelist name to token array
    typedef std::map<std::string, TokenArray> NLmap;

    enum StatType { OK = 0, ERR, EMPTY_LINE, NAMELIST_END, NEW_NAMELIST };

    static StatType TokenizeLine(TokenArray&, std::string const&, std::string&);

    /// Add given line from MDIN to current namelist
    int AddToNamelist(std::string const&);

    std::string currentNamelist_; ///< Current namelist during ParseFile
    NLmap NameLists_;             ///< Hold all namelists
};
#endif
