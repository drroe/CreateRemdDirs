#ifndef INC_MDIN_FILE
#define INC_MDIN_FILE
#include <string>
#include <vector>
#include <map>
/// Used to parse input from an Amber MDIN file
class MdinFile {
  public:
    MdinFile();

    /// Pair namelist variable with value
    typedef std::pair<std::string, std::string> TokenType;
    /// Array of namelist variable/value pairs
    typedef std::vector<TokenType> TokenArray;
    /// Map namelist name to token array
    typedef std::map<std::string, TokenArray> NLmap;

    typedef NLmap::const_iterator const_iterator;

    typedef TokenArray::const_iterator token_iterator;

    /// \return const iterator to beginning of namelists
    const_iterator nl_begin() const { return NameLists_.begin(); }
    /// \return const_iterator to end of namelists
    const_iterator nl_end()   const { return NameLists_.end(); }

    /// Parse the given Amber MDIN file into namelists and variables
    int ParseFile(std::string const&);
    /// \return Value for variable in specified namelist if present, empty otherwise.
    std::string GetNamelistVar(std::string const&, std::string const&) const;
  private:
    /// Pair namelist name to token array
    typedef std::pair<std::string, TokenArray> NLpair;

    enum StatType { OK = 0, ERR, EMPTY_LINE, NAMELIST_END, NEW_NAMELIST };

    static StatType TokenizeLine(TokenArray&, std::string const&, std::string&);

    /// Add given line from MDIN to current namelist
    int AddToNamelist(std::string const&);

    std::string currentNamelist_; ///< Current namelist during ParseFile
    NLmap NameLists_;             ///< Hold all namelists
};
#endif
