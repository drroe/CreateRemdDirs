#ifndef INC_RUNARRAY_H
#define INC_RUNARRAY_H
#include <vector>
#include "Run.h"
/// Array of Runs
class RunArray {
    typedef std::vector<Run> Rarray;
  public:
    /// CONSTRUCTOR
    RunArray() {}
    /// Const Iterator
    typedef Rarray::const_iterator const_iterator;
    /// Begin
    const_iterator begin() const { return Runs_.begin(); }
    /// End
    const_iterator end()   const { return Runs_.end(); }
    /// Iterator
    typedef Rarray::iterator iterator;
    /// Begin
    iterator begin() { return Runs_.begin(); }
    /// End
    iterator end()   { return Runs_.end(); }
    /// \return Run at specified index
    Run const& operator[](int idx) const { return Runs_[idx]; }
    /// \return modifiable Run at specified index
    Run& Set_Run(int idx) { return Runs_[idx]; }
    /// \return size
    unsigned int size() const { return Runs_.size(); }
    /// \return true if no runs
    bool empty() const { return Runs_.empty(); }
    /// \return last Run
    Run const& back() const { return Runs_.back(); }
    /// \return modifiable last Run
    Run& Set_back() { return Runs_.back(); }
    /// \return first Run
    Run const& front() const { return Runs_.front(); }
    /// Clear runs
    void clear() { Runs_.clear(); }
    /// Add a run
    void AddRun(Run const& r) { Runs_.push_back( r ); }
  private:
    Rarray Runs_;
};
#endif
