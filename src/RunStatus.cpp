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
  "Ready",
  "Queued",
  "Running",
  "Incomplete",
  "Complete"
};

/** Print status to stdout (no newline). */
void RunStatus::PrintStatus(std::string const& jobid) const {
  if (stat_ == EMPTY || stat_ == PENDING || stat_ == READY || stat_ == IN_QUEUE)
    Msg("%10s", StatStr_[stat_]);
  else {
    // DEBUG
    double frac_steps;
    if (runOpts_.N_Steps().IsSet() && runOpts_.N_Steps().Val() > 0)
      frac_steps = (double)current_n_steps_ / (double)runOpts_.N_Steps().Val();
    else
      frac_steps = 0;
    double pct_steps = frac_steps * 100.0;
    //runOpts_.PrintOpts(false, -1, -1);
    if (current_traj_frames_ > -1)
      Msg("%10s Pct=%6.2f%% Frames=%10i of %10i",
          StatStr_[stat_],
          //runOpts_.Total_Time(),
          pct_steps,
          current_traj_frames_,
          runOpts_.Expected_Frames());
    else
      Msg("%10s Pct=%6.2f%% Frames=%10s of %10i",
          StatStr_[stat_],
          //runOpts_.Total_Time(),
          pct_steps,
          "Unknown",
          runOpts_.Expected_Frames());
    if (ns_per_day_.IsSet())
      Msg(" %g ns/day", ns_per_day_.Val());
    if (!jobid.empty())
      Msg(" Job=%s\n", jobid.c_str());
  }
}

/** \return Current # traj frames. */
unsigned int RunStatus::CurrentTrajFrames() const {
  if (current_traj_frames_ > -1)
    return (unsigned int)current_traj_frames_;
  return 0;
}
