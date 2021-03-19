#ifndef INC_SIMULATION_H
#define INC_SIMULATION_H
#include <string>
/// Hold information for a single simulation (output, trajectory, restart)
class Simulation {
  public:
    Simulation();

    /// Get simulation information from output, trajectory, and restart files.
    int SetupFromFiles(std::string const&, std::string const&, std::string const&, int);
  private:
    double time_step_;       ///< Simulation time step in ps
    double total_time_;      ///< Total current simulation time in ps
    int expected_nsteps_;    ///< Number of steps simulation should complete.
    int expected_exchanges_; ///< Number of exchanges simulation should complete (-1 for regular MD)
    int traj_write_freq_;    ///< How often (in steps) simulation will write trajectory
    int expected_frames_;    ///< Number of trajectory frames simulation should write
    bool completed_;         ///< True if simulation has completed.
};
#endif
