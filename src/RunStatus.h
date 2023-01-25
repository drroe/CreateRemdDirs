#ifndef INC_RUNSTATUS_H
#define INC_RUNSTATUS_H
#include "MdOptions.h"
/// Hold information for an existing run
class RunStatus {
  public:
    /// Various status types - SYNC WITH StatStr_
    enum StatusType { UNKNOWN=0, EMPTY, INCOMPLETE, COMPLETE };

    /// CONSTRUCTOR
    RunStatus();
    /// CONSTRUCTOR - status only
    RunStatus(StatusType);

    /// Print status to stdout
    void PrintStatus() const;
  private:
    static const char* StatStr_[];

    MdOptions runOpts_; ///< Run MD options
    StatusType stat_;   ///< Current run status
    int current_traj_frames_; ///< Current number of frames in trajectory
    int current_n_steps_;     ///< Current number of steps
};
#endif
