#ifndef INC_RUNINDEX_H
#define INC_RUNINDEX_H
/// Used to hold system, project, and run index.
class RunIndex {
  public:
    RunIndex(int p, int s, int r) : pidx_(p), sidx_(s), ridx_(r) {}

    int Pidx() const { return pidx_; }
    int Sidx() const { return sidx_; }
    int Ridx() const { return ridx_; }

    bool operator<(RunIndex const& rhs) const {
      if (pidx_ == rhs.pidx_) {
        if (sidx_ == rhs.sidx_) {
          return (ridx_ < rhs.ridx_);
        } else {
          return (sidx_ < rhs.sidx_);
        }
      } else {
        return (pidx_ < rhs.pidx_);
      }
    }
  private:
    int pidx_; ///< Project index
    int sidx_; ///< System index
    int ridx_; ///< Run index
};
#endif
