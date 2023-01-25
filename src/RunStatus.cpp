#include "RunStatus.h"
#include "Messages.h"

using namespace Messages;

/** CONSTRUCTOR */
RunStatus::RunStatus() :
  stat_(UNKNOWN),
  current_traj_frames_(-1),
  current_n_steps_(-1)
{}

/** CONSTRUCTOR - Status only */
RunStatus::RunStatus(StatusType t) :
  stat_(t),
  current_traj_frames_(0),
  current_n_steps_(0)
{}

/** CORRESPONDS TO StatusType */
const char* RunStatus::StatStr_[] = {
  "Unknown",
  "Empty",
  "Pending",
  "Incomplete",
  "Complete"
};

/** Print status to stdout (no newline). */
void RunStatus::PrintStatus() const {
  if (stat_ == EMPTY || stat_ == PENDING)
    Msg("%10s", StatStr_[stat_]);
  else
    Msg("%10s Time=%12.4g Frames=%10i Expected=%10i",
        StatStr_[stat_],
        runOpts_.Total_Time(),
        current_traj_frames_,
        runOpts_.Expected_Frames());
}