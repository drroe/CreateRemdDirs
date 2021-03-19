#ifndef INC_TOKENS_H
#define INC_TOKENS_H
#include <string>
#include <vector>
/// Class for breaking a string up into an array of strings using given separators.
class Tokens {
  public:
    Tokens();
    /// Tokenize the given line according to given separators.
    int Tokenize(std::string const&, const char*);
    /// \return Token at specified position
    std::string const& operator[](unsigned int i) const { return tokens_[i]; }
    /// \return current number of tokens
    unsigned int Ntokens() const { return tokens_.size(); }
    /// \return Unmarked Token next to specified unmarked key.
    std::string GetToken(const char*);
    /// \return Next unmarked token.
    std::string NextToken();
  private:
    typedef std::vector<std::string> Sarray;
    Sarray tokens_;
    typedef std::vector<bool> Barray;
    Barray marked_;
};
#endif
