#include <cstdlib> // atoi
#include <cstring> // strncmp
#ifdef HAS_NETCDF
# include <netcdf.h>
# include "NC.h"
#endif
#include "Simulation.h"
#include "Messages.h"
#include "TextFile.h"

using namespace Messages;

/** CONSTRUCTOR */
Simulation::Simulation() :
  time_step_(0),
  total_time_(0),
  expected_nsteps_(0),
  expected_exchanges_(-1),
  traj_write_freq_(0),
  expected_frames_(0),
  actual_frames_(-1),
  completed_(false)
{}

/** Set simulation info from files. */
int Simulation::SetupFromFiles(std::string const& outfilename,
                               std::string const& trajfilename,
                               std::string const& rstfilename,
                               int debug)
{
  // Determine how many frames should be written by the output file.
  // Read the '2. CONTROL DATA FOR THE RUN' section of MDOUT. Look
  // for '5. TIMINGS' to ensure run completed.
  TextFile mdout;
  if (mdout.OpenRead( outfilename )) {
    ErrorMsg("Could not open output file '%s'\n", outfilename.c_str());
    return 1;
  }
  completed_ = false;
  expected_nsteps_ = 0;
  time_step_ = 0;
  total_time_ = 0;
  expected_exchanges_ = -1;
  traj_write_freq_ = 0;
  expected_frames_ = 0;
  actual_frames_ = -1;

  int readInput = 0;
  const char* SEP = " ,=\r\n";
  int ncols = mdout.GetColumns(SEP);
  while (ncols > -1) {
    if (readInput == 0 && ncols > 2) {
      if (mdout.Token(0) == "2." && mdout.Token(1) == "CONTROL")
        readInput = 1;
    } else if (readInput == 1 && ncols > 1) {
      if (mdout.Token(0) == "3." && mdout.Token(1) == "ATOMIC")
        break;
      else {
        for (int col = 0; col != ncols - 1; col++) {
          if (mdout.Token(col) == "nstlim")
            expected_nsteps_ = atoi( mdout.Token(col+1).c_str() );
          else if (mdout.Token(col) == "dt")
            time_step_ = atof( mdout.Token(col+1).c_str() );
          else if (mdout.Token(col) == "numexchg")
            expected_exchanges_ = atoi( mdout.Token(col+1).c_str() );
          else if (mdout.Token(col) == "ntwx")
            traj_write_freq_ = atoi( mdout.Token(col+1).c_str() );
        }
      }
    }
    ncols = mdout.GetColumns(SEP);
  }
  // Scan down to '5. TIMINGS'
  const char* ptr = mdout.Gets();
  while (ptr != 0) {
    if (strncmp("   5.  TIMINGS", ptr, 14)==0) {
      completed_ = true;
      break;
    }
    ptr = mdout.Gets();
  }
  mdout.Close();

  if (completed_) {
    if (debug > 0) Msg("\tRun completed.\n");
  } else {
    if (debug > 0) Msg("\tRun did not complete.\n");
    //*runStat = false;
    //iRunStat = 1;
  }

  if (debug > 0) {
    Msg("\tExpected_Nsteps    = %i\n", expected_nsteps_);
    Msg("\tTime_step          = %g\n", time_step_);
    if (expected_exchanges_ > 0)
      Msg("\tExpected_exchanges = %i\n", expected_exchanges_);
    Msg("\tTraj_write_freq_   = %i\n", traj_write_freq_);
  }

  int numexchg = expected_exchanges_;
  if (numexchg < 1) numexchg = 1;
  total_time_ = ((double)expected_nsteps_ * time_step_) * (double)numexchg;
  if (traj_write_freq_ > 0)
    expected_frames_ = (expected_nsteps_ * numexchg) / traj_write_freq_;
  if (debug > 0) {
    Msg("\tTotal_time         = %g ps\n", total_time_);
    Msg("\tExpected_Frames    = %i\n", expected_frames_);
  }

  // Trajectory check.
# ifdef HAS_NETCDF
  // Get actual number of frames from NetCDF file.
  int ncid = -1;
  if ( NC::CheckErr(nc_open(trajfilename.c_str(), NC_NOWRITE, &ncid)) ) {
    ErrorMsg("Could not open trajectory file '%s'\n", trajfilename.c_str());
    //*runStat = false;
    return 1;
  }
  unsigned int slength; 
  int dimID = NC::GetDimInfo(ncid, "frame", slength);
  if (dimID == -1) return 1;
  actual_frames_ = (int)slength;
  nc_close( ncid );
  if (debug > 0)
    Msg("\tActual Frames: %i\n", actual_frames_);
  // If run did not complete, check restart files if replica.
  if (expected_frames_ != actual_frames_) {
    //++numBadFrameCount;
    //++Nwarnings_;
    //if (badFrameCount != actual_frames_) { // To avoid repeated checkall warnings
      if (debug > 0)
        Msg("Warning: # actual frames %i != # expected frames %i.\n",
            actual_frames_, expected_frames_);
      //badFrameCount = actual_frames_;
    //}
    //if (runType == Run::REMD) check_restarts = true;
  } else {
    if (debug > 0) Msg("\tOK.\n");
  }
# endif /* HAS_NETCDF */
  long int idx = 0;
  Msg("%04li %4i %12g %12i %12i\n", idx, (int)completed_, total_time_,
      actual_frames_, expected_frames_);

  return 0;
}

