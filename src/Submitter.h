#ifndef INC_SUBMITTER_H
#define INC_SUBMITTER_H
/// Used to submit jobs
class Submitter {
  public:
    Submitter();
    /// Read options from a file
    int ReadOptions(std::string const&);
  private:
};
#endif
