#ifndef INC_SUBMITTER_H
#define INC_SUBMITTER_H
#include "FileRoutines.h" // StrArray
class QueueOpts;
/// Class used to submit jobs via a queuing system.
class Submitter {
  public:
    Submitter() : Run_(0), Analyze_(0), Archive_(0), n_input_read_(0), debug_(0), testing_(false) {}
   ~Submitter();

   static void OptHelp();
   int ReadOptions(std::string const&);
   int CheckOptions();
   int SubmitRuns(std::string const&, FileRoutines::StrArray const&, int, bool, std::string const&) const;
   int SubmitAnalysis(std::string const&, int, int, bool) const;
   int SubmitArchive(std::string const&, int, int, bool) const;
   void SetTesting(bool t) { testing_ = t; }
   void SetDebug(int d)    { debug_ = d;   }
  private:
    int ReadOptions(std::string const&, QueueOpts&);

    QueueOpts *Run_;     ///< Run queue options
    QueueOpts *Analyze_; ///< Analysis queue options
    QueueOpts *Archive_; ///< Archive queue options
    int n_input_read_;   ///< # of times ReadOptions has been called.
    int debug_;
    bool testing_;       ///< If true do not actually submit scripts.
};
#endif
