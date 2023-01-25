#ifndef INC_SIMULATION_H
#define INC_SIMULATION_H
#include <string>
/// Hold information for a single simulation (output, trajectory, restart)
class Simulation {
  public:
    Simulation();

    /// Get simulation information from output, trajectory, and restart files.
    int SetupFromFiles(std::string const&, std::string const&, std::string const&, int);

    double Total_Time()      const { return total_time_; }
    //int Expected_Nsteps() const { return expected_nsteps_; }
    int Expected_Exchanges() const { return expected_exchanges_; }
    int Expected_Frames()    const { return expected_frames_; }
    int Actual_Frames()      const { return actual_frames_; }
    bool Completed()         const { return completed_; }
  private:
    double time_step_;       ///< Simulation time step in ps
    double total_time_;      ///< Total current simulation time in ps
    int expected_nsteps_;    ///< Number of steps simulation should complete.
    int expected_exchanges_; ///< Number of exchanges simulation should complete (-1 for regular MD)
    int traj_write_freq_;    ///< How often (in steps) simulation will write trajectory
    int expected_frames_;    ///< Number of trajectory frames simulation should write
    int actual_frames_;      ///< Actual number of frames in the trajectory.
    bool completed_;         ///< True if simulation has completed.
};
#endif
