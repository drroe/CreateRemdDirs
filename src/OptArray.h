#ifndef INC_OPTARRAY_H
#define INC_OPTARRAY_H
#include <string>
#include <utility> // pair
#include <vector>
/// Hold array of options of style <OPTION> = <VALUE>
class OptArray {
  public:
    typedef std::pair<std::string, std::string> OptPair;
    typedef std::vector<OptPair> OptPairArray;

    /// CONSTRUCTOR
    OptArray() {}
    /// Add option value pair
    void AddOpt( OptPair const& sp ) { options_.push_back( sp ); }
    /// Iterator
    OptPairArray::const_iterator const_iterator;
    /// Iterator to beginning
    const_iterator begin() const { return options_.begin(); }
    /// Iterator to end
    const_iterator end() const { return options_.end(); }
  private:
    OptPairArray options_;
};
#endif
