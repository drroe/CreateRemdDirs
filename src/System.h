#ifndef INC_SYSTEM_H
#define INC_SYSTEM_H
#include <string>
#include <vector>
#include "Creator.h"
#include "Submitter.h"
#include "MdInterface.h"
#include "RunArray.h"
class QueueArray;
/// Hold information on runs for a system
class System {
  public:
    /// CONSTRUCTOR
    System();
    /// CONSTRUCTOR - top directory, run directory, description
    System(std::string const&, std::string const&, std::string const&);
    /// COPY CONSTRUCTOR
    System(System const&);
    /// ASSIGNMENT
    System& operator=(System const&);

    /// Find runs in dirname_
    int FindRuns(QueueArray&);
    /// Refresh current runs (true = verbose)
    int RefreshCurrentRuns(bool);
    /// Refresh only the specified run
    int RefreshSpecifiedRun(int);
    /// Set debug level
    void SetDebug(int);

    /// Create runs
    int CreateRunDirectories(int, int, bool);
    /// Submit runs
    int SubmitRunDirectories(int, int, bool, std::string const&, bool);
    /// Parse a single option
    int ParseOption(std::string const&, std::string const&);
    /// Write current options to files
    int WriteSystemOptions();

    /// Print system summary
    void PrintSummary() const;
    /// Print system info
    void PrintInfo() const;
    /// \return true if all options are ok
    bool CheckAllOptions() const;
    /// \return Run Array
    RunArray const& Runs() const { return Runs_; }
    /// Change to system directory
    int ChangeToSystemDir() const;
    /// \return true if System options need to be written
    bool NeedsSave() const { return (c_needs_save_ || s_needs_save_); }
    /// \return System dir name
    //std::string const& SystemDirName() const { return dirname_; }
  private:
    typedef std::vector<std::string> Sarray;
    /// Clear all runs
    void clearRuns();
    /// Find existing run index if present
    int findRunIdx(int) const;
    /// Read MdPackage specific MD input options
    int read_mdpackage_mdin();

    int debug_; 
    RunArray Runs_;                  ///< Array of runs for the system
    std::string topDir_;             ///< Top level directory
    std::string dirname_;            ///< Directory containing runs for the system
    std::string description_;        ///< Description of the system
    std::string createOptsFilename_; ///< File name for creator options
    std::string submitOptsFilename_; ///< File name for submitter options
    std::string runDirPrefix_;       ///< Run directory prefix
    int runDirExtWidth_;             ///< Width of run directory numerical suffix
    Creator creator_;                ///< For creating runs
    Submitter submitter_;            ///< For submitting runs
    MdInterface mdInterface_;        ///< Use to interface with different MD packages
    bool c_needs_save_;              ///< True if Creator options have changed and System needs save
    bool s_needs_save_;              ///< True if Submitter options have changed and System needs save
};
#endif
