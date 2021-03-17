#include <cmath>
#include <cstdlib>
#ifdef HAS_NETCDF
# include "netcdf.h"
#endif
#include "CheckRuns.h"
#include "Messages.h"
#include "TextFile.h"

/** CONSTRUCTOR */
CheckRuns::CheckRuns() {}

#ifdef HAS_NETCDF
int CheckRuns::checkNCerr(int ncerr) {
  if ( ncerr != NC_NOERR ) {
    ErrorMsg("NETCDF: %s\n", nc_strerror(ncerr));
    return 1;
  }
  return 0;
}

int CheckRuns::GetDimInfo(int ncid, const char* attribute, int& length) {
  int dimID;
  size_t slength = 0;
  length = 0;
  // Get dimid 
  if ( checkNCerr(nc_inq_dimid(ncid, attribute, &dimID)) ) {
    ErrorMsg("Getting dimID for attribute %s\n", attribute);
    return -1;
  }
  // get Dim length 
  if ( checkNCerr(nc_inq_dimlen(ncid, dimID, &slength)) ) {
    ErrorMsg("Getting length for attribute %s\n",attribute);
    return -1;
  }
  length = (int) slength;
  return dimID;
}
#endif

std::string CheckRuns::Ext(std::string const& name) {
  size_t found = name.find_last_of(".");
  if (found == std::string::npos)
    return std::string("");
  return  name.substr(found);
}

/** Given two arrays of file names of different size try to determine
  * where they differ based on extension.
  */
void CheckRuns::CompareStrArray(StrArray const& a1, StrArray const& a2) {
  unsigned int max = (unsigned int)std::min(a1.size(), a2.size());
  unsigned int idx = 0;
  for (idx = 0; idx != max; idx++) {
    std::string e1 = Ext(a1[idx]);
    std::string e2 = Ext(a2[idx]);
    if (e1 != e2) {
      ErrorMsg("Differs at '%s', '%s'\n", a1[idx].c_str(), a2[idx].c_str());
      return;
    }
  }
  if (idx < a1.size())
    ErrorMsg("Differs at '%s'\n", a1[idx].c_str());
  if (idx < a2.size())
    ErrorMsg("Differs at '%s'\n", a2[idx].c_str());
}

/** Check trajectories and output files for specified runs. */
int CheckRuns::DoCheck(std::string const& TopDir, StrArray const& RunDirs, bool firstOnly) {
  if (firstOnly)
    Msg("Checking only first output/traj for all runs.\n");
  else
    Msg("Checking all output/traj for all runs.\n");
  int debug = 0;
  int Nwarnings = 0;
  std::vector<bool> run_is_ok(RunDirs.size(), false);
  // Loop over all run directories
  std::vector<bool>::iterator runStat = run_is_ok.begin();
  for (StrArray::const_iterator rdir = RunDirs.begin(); rdir != RunDirs.end(); ++rdir, ++runStat)
  {
    if (ChangeDir( TopDir )) {
      ErrorMsg("Could not change to top directory '%s'\n", TopDir.c_str());
      return 1;
    }
    if (!fileExists( *rdir ))
      Msg("Warning: '%s' does not exist.\n", rdir->c_str());
    else {
      *runStat = true;
      Msg("  %s:", rdir->c_str());
      if (ChangeDir( *rdir )) {
        ErrorMsg("Could not change to run directory '%s'\n", rdir->c_str());
        return 1;
      }
      // Try to determine the run type based on the output files.
      RunType runType = UNKNOWN;
      // REMD? 
      StrArray output_files = ExpandToFilenames("OUTPUT/rem.out.*", false);
      if (!output_files.empty()) {
        runType = REMD;
      } else {
        // Not REMD. Multiple MD?
        output_files = ExpandToFilenames("md.out.*", false);
        if (!output_files.empty()) {
          runType = MULTI_MD;
        } else {
          if (fileExists("md.out")) {
            output_files.push_back("md.out");
            runType = SINGLE_MD;
          }
        }
      }
      Msg(" %zu output files.\n", output_files.size());
      if (output_files.empty() || runType == UNKNOWN) {
        ErrorMsg("Output file(s) not found.\n");
        *runStat = false;
        continue;
      }
      // Determine where the trajectory files are.
      StrArray traj_files;
      if (runType == REMD)
        traj_files = ExpandToFilenames("TRAJ/rem.crd.*", false);
      else if (runType == MULTI_MD)
        traj_files = ExpandToFilenames("md.nc.*", false);
      else if (fileExists("mdcrd.nc"))
        traj_files.push_back("mdcrd.nc");
      else
        ErrorMsg("Trajectory file(s) not found.");
       
      if (traj_files.size() != output_files.size()) {
        ErrorMsg("Number of output files %zu != # of traj files %zu.\n",
                 output_files.size(), traj_files.size());
        CompareStrArray( output_files, traj_files );
        *runStat = false;
        continue;
      }
      // Loop over output and trajectory files.
#     ifdef HAS_NETCDF
      int badFrameCount = -1;
#     endif
      int numBadFrameCount = 0;
      bool check_restarts = false;
      StrArray::const_iterator tname = traj_files.begin();
      for (StrArray::const_iterator fname = output_files.begin();
                                    fname != output_files.end();
                                  ++fname, ++tname)
      {
        if (debug > 0) Msg("    '%s'\n", fname->c_str());
        // Determine how many frames should be written by the output file.
        // Read the '2. CONTROL DATA FOR THE RUN' section of MDOUT. Look
        // for '5. TIMINGS' to ensure run completed.
        TextFile mdout;
        if (mdout.OpenRead( *fname )) {
          ErrorMsg("Could not open output file '%s'\n", fname->c_str());
          return 1;
        }
        bool end_mdout_reached = false;
        int readInput = 0;
        int nstlim = 0;
        double dt = 0;
        int numexchg = 0;
        int ntwx = 0;
        int expectedFrames = 0;
        const char* SEP = " ,=\r\n";
        int ncols = mdout.GetColumns(SEP);
        while (ncols > -1) {
          if (readInput == 0 && ncols > 2) {
            if (mdout.Token(0) == "2." && mdout.Token(1) == "CONTROL")
              readInput = 1;
          } else if (readInput == 1 && ncols > 1) {
            if (mdout.Token(0) == "3." && mdout.Token(1) == "ATOMIC")
              readInput = 2;
            else {
              for (int col = 0; col != ncols - 1; col++) {
                if (mdout.Token(col) == "nstlim")
                  nstlim = atoi( mdout.Token(col+1).c_str() );
                else if (mdout.Token(col) == "dt")
                  dt = atof( mdout.Token(col+1).c_str() );
                else if (mdout.Token(col) == "numexchg")
                  numexchg = atoi( mdout.Token(col+1).c_str() );
                else if (mdout.Token(col) == "ntwx")
                  ntwx = atoi( mdout.Token(col+1).c_str() );
              }
            }
          } else if (readInput == 2 && ncols > 1) {
            if (mdout.Token(0) == "5." && mdout.Token(1) == "TIMINGS") {
              end_mdout_reached = true;
              break;
            }
          }
          ncols = mdout.GetColumns(SEP);
        }
        mdout.Close();
        if (end_mdout_reached)
          Msg("\tRun completed.\n");
        else {
          Msg("\tRun did not complete.\n");
          *runStat = false;
        }
        Msg("\tnstlim= %i\n", nstlim);
        Msg("\tdt= %g\n", dt);
        if (runType == REMD) Msg("\tnumexchg= %i\n", numexchg);
        Msg("\tntwx= %i\n", ntwx);
        if (numexchg == 0) numexchg = 1;
        double totalTime = ((double)nstlim * dt) * (double)numexchg;
        expectedFrames = (nstlim * numexchg) / ntwx;
        //if (debug > 0) {
          Msg("\tTotal time: %g ps\n", totalTime);
          Msg("\tExpected Frames: %i\n", expectedFrames);
        //}

        // Trajectory check.
#       ifdef HAS_NETCDF
        // Get actual number of frames from NetCDF file.
        int ncid = -1;
        if ( checkNCerr(nc_open(tname->c_str(), NC_NOWRITE, &ncid)) ) {
          ErrorMsg("Could not open trajectory file '%s'\n", tname->c_str());
          *runStat = false;
          continue;
        }
        int dimID;
        size_t slength = 0;
        if ( checkNCerr(nc_inq_dimid(ncid, "frame", &dimID))  ) return 1;
        if ( checkNCerr(nc_inq_dimlen(ncid, dimID, &slength)) ) return 1;
        int actualFrames = (int)slength;
        nc_close( ncid );
        //if (debug > 0)
          Msg("\tActual Frames: %i\n", actualFrames);
        // If run did not complete, check restart files if replica.
        if (expectedFrames != actualFrames) {
          ++numBadFrameCount;
          ++Nwarnings;
          if (badFrameCount != actualFrames) { // To avoid repeated checkall warnings
            Msg("Warning: # actual frames %i != # expected frames %i.\n",
                actualFrames, expectedFrames);
            badFrameCount = actualFrames;
          }
          if (runType == REMD) check_restarts = true;
        } else {
          if (debug > 0) Msg("\tOK.\n");
        }
#       else
        Msg("Compiled without NetCDF; skipping trajectory check.\n");
#       endif /* HAS_NETCDF */
        if (firstOnly) break;
      } // END loop over output files for run
      if (numBadFrameCount > 0) {
        Msg("Warning: Frame count did not match for %i replicas.\n", numBadFrameCount);
        *runStat = false;
      }
      // Check restarts for REMD run if any OUTPUT/TRAJ files were bad.
      if (check_restarts) {
        StrArray restart_files = ExpandToFilenames("RST/*.rst7");
        if (restart_files.empty())
          restart_files = ExpandToFilenames("RST/*.ncrst");
        if (restart_files.size() != output_files.size()) {
          ErrorMsg("Number of restart files %zu != # output files %zu\n",
                   restart_files.size(), output_files.size());
          CompareStrArray( restart_files, output_files );
          *runStat = false;
          continue;
        }
#       ifdef HAS_NETCDF
        double rst_time0 = 0.0;
        for (StrArray::const_iterator rfile = restart_files.begin();
                                      rfile != restart_files.end(); ++rfile)
        {
          int ncid = -1, timeVID = -1;
          double rsttime = -1.0;
          if ( checkNCerr(nc_open(rfile->c_str(), NC_NOWRITE, &ncid)) ) return 1; // TODO Ascii
          if ( checkNCerr(nc_inq_varid(ncid, "time", &timeVID)      ) ) return 1;
          if ( checkNCerr(nc_get_var_double(ncid, timeVID, &rsttime)) ) return 1;
          if (rfile == restart_files.begin()) {
            rst_time0 = rsttime;
            Msg("\tInitial restart time: %g\n", rst_time0);
          } else if ( fabs(rst_time0 - rsttime) > 0.00000000000001 ) {
            ErrorMsg("File '%s' time %g does not match initial restart time %g\n",
                     rfile->c_str(), rsttime, rst_time0);
            *runStat = false;
            continue;
          }
          // Check first 2 coordinates
          int natom;
          int atomDID = GetDimInfo(ncid, "atom", natom);
          if (atomDID < 0) return 1;
          if (natom > 1) {
            size_t start[2], count[2];
            int coordVID = -1;
            if ( checkNCerr(nc_inq_varid(ncid, "coordinates", &coordVID)) ) return 1;
            start[0] = 0;
            start[1] = 0;
            count[0] = 2; // Only 2 atoms
            count[1] = 3;
            double Coords[6]; // Hold first 2 coord sets
            if ( checkNCerr(nc_get_vara_double(ncid, coordVID, start, count, Coords)) )
              return 1;
            // Calculate distance
            double dx = Coords[0] - Coords[3];
            double dy = Coords[1] - Coords[4];
            double dz = Coords[2] - Coords[5];
            double dist2 = (dx * dx) + (dy * dy) + (dz * dz);
            if (dist2 < 0.0001) {
              ErrorMsg("First two coordinates in restart '%s' overlap. Probable corruption.\n",
                        rfile->c_str());
              *runStat = false;
              continue;
            }
            // Get box info if present
            int cellVID = -1;
            if ( nc_inq_varid(ncid, "cell_lengths", &cellVID) == NC_NOERR ) {
              count[0] = 3;
              count[1] = 0;
              if ( checkNCerr(nc_get_vara_double(ncid, cellVID, start, count, Coords)) )
                return 1;
              // Calc max distance allowed by box
              double box2 = (Coords[0]*Coords[0]) + (Coords[1]*Coords[1]) + (Coords[2]*Coords[2]);
              if (dist2 > box2) {
                ErrorMsg("First two coordinates distance > box size in restart '%s'."
                         " Probable corruption.\n", rfile->c_str());
                *runStat = false;
                continue;
              }
            }
          }
          nc_close( ncid );
        } // END loop over restart files
#       else
        Msg("Compiled without NetCDF. Skipping restart check.\n");
#       endif /* HAS_NETCDF */
      } // END check restarts
    } // END run directory exists
  } // END loop over runs
  unsigned int n_bad_runs = 0;
  for (std::vector<bool>::const_iterator it = run_is_ok.begin(); it != run_is_ok.end(); ++it)
    if (!(*it))
      ++n_bad_runs;
  if (n_bad_runs > 0) {
    ErrorMsg("%u of %zu runs had problems.\n", n_bad_runs, RunDirs.size());
    return 1;
  }
  if (Nwarnings == 0)
    Msg("  All checks OK.\n");
  else
    Msg("  Runs seem OK, but some warnings were encountered.\n");

  return 0;
}
