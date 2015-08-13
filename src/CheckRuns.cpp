#include <cstdlib>
#include "CheckRuns.h"
#include "StringRoutines.h"
#include "FileRoutines.h"
#include "Messages.h"
#include "TextFile.h"

int CheckRuns(std::string const& TopDir, int start_run, int stop_run) {
  for (int run = start_run; run <= stop_run; run++) {
    if (ChangeDir( TopDir )) return 1;
    std::string dir = "run." + integerToString(run, 3);
    if (!fileExists( dir ))
      Msg("Warning: '%s' does not exist.", dir.c_str());
    else {
      Msg("\t%s", dir.c_str());
      ChangeDir( dir );
      //bool is_md = false;
      // Determine where the output file(s) are.
      StrArray output_files = ExpandToFilenames("OUTPUT/rem.out.*");
      if (output_files.empty())
        output_files = ExpandToFilenames("md.out.*");
      Msg("\t%zu output files.\n", output_files.size());
      for (StrArray::const_iterator fname = output_files.begin();
                                    fname != output_files.end(); ++fname)
      {
        Msg("\t'%s'\n", fname->c_str());
        // Determine how many frames should be written by the output file.
        TextFile mdout;
        if (mdout.OpenRead( *fname )) return 1;
        int readInput = 0;
        int nstlim = 0;
        double dt = 0;
        int numexchg = 0;
        int ntwx = 0;
        int expectedFrames = 0;
        const char* SEP = " ,=";
        int ncols = mdout.GetColumns(SEP);
        while (ncols > -1) {
          if (readInput == 0 && ncols > 2) {
            if (mdout.Token(0) == "2." && mdout.Token(1) == "CONTROL")
              readInput = 1;
          } else if (readInput == 1 && ncols > 1) {
            if (mdout.Token(0) == "3." && mdout.Token(1) == "ATOMIC")
              break;
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
          ncols = mdout.GetColumns(SEP);
        }
        mdout.Close();
        Msg("\tnstlim= %i\n", nstlim);
        Msg("\tdt= %g\n", dt);
        Msg("\tnumexchg= %i\n", numexchg);
        Msg("\tntwx= %i\n", ntwx);
        if (numexchg == 0) numexchg = 1;
        Msg("\tTotal: %g\n", ((double)nstlim * dt) * (double)numexchg);
        expectedFrames = (nstlim * numexchg) / ntwx;
        Msg("\tFrames: %i\n", expectedFrames);
      }
    }
  }
  return 0;
}
