#ifndef INC_SYSTEM_H
#define INC_SYSTEM_H
#include <string>
#include <vector>
#include "Creator.h"
#include "Submitter.h"
class Run;
/// Hold information on runs for a system
class System {
  public:
    typedef std::vector<Run*> RunArray;
    /// CONSTRUCTOR
    System();
    /// DESTRUCTOR
    ~System();
    /// CONSTRUCTOR - top directory, run directory, description
    System(std::string const&, std::string const&, std::string const&);
    /// COPY CONSTRUCTOR
    System(System const&);
    /// ASSIGNMENT
    System& operator=(System const&);

    /// Find runs in dirname_
    int FindRuns();
    /// Set debug level
    void SetDebug(int);

    /// Set up run creator
    int SetupRunCreator(std::string const& c, bool m) { return creator_.Setup(c, m); }
    /// Print run creator info
    void RunCreatorInfo() const { creator_.Info(); }
    /// Create runs
    int CreateRunDirectories(int, int, bool);

    /// Print system info
    void PrintInfo() const;
    /// \return Run Array
    RunArray const& Runs() const { return Runs_; }
    /// Change to system directory
    int ChangeToSystemDir() const;
    /// \return System dir name
    //std::string const& SystemDirName() const { return dirname_; }
  private:
    typedef std::vector<std::string> Sarray;
    /// Clear all runs
    void clearRuns();
    /// Create run directory names from start to stop with current digit width
    Sarray createRunDirNames(int, int) const;

    RunArray Runs_;           ///< Array of runs for the system
    std::string topDir_;      ///< Top level directory
    std::string dirname_;     ///< Directory containing runs for the system
    std::string description_; ///< Description of the system
    std::string createOptsFilename_; ///< File name for creator options
    std::string submitOptsFilename_; ///< File name for submitter options
    std::string runDirPrefix_;       ///< Run directory prefix
    int runDirExtWidth_;             ///< Width of run directory numerical suffix
    Creator creator_;                ///< For creating runs
    Submitter submitter_;            ///< For submitting runs
};
#endif
