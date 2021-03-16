#ifndef INC_MDIN_FILE
#define INC_MDIN_FILE
#include <string>
#include <vector>
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

    static TokenArray TokenizeLine(std::string const&);
};
#endif
