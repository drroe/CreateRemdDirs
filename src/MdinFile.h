#ifndef INC_MDIN_FILE
#define INC_MDIN_FILE
#include <string>
/// Used to parse input from an Amber MDIN file
class MdinFile {
  public:
    MdinFile();

    int ParseFile(std::string const&);
};
#endif
