# CreateRemdDirs
Automatically generate input for running M-REMD simulations (and more) in Amber.

About CreateRemdDirs
====================
CreateRemdDirs can be used to automatically generate input for running REMD, 
H-REMD, M-REMD, and MD simulations that will be chained together (split up into
several runs). The general idea is to facilitate long simulation runs on e.g.
HPC resources. Currently CreateRemdDirs requires NetCDF libraries to perform
checks on Amber trajectories/restarts.

## Installation
Typically it is enough to run `./configure gnu` on most systems. The job check
mode requires NetCDF libraries. If you've already got an Amber installation
built with NetCDF it will be enough to `./configure --with-netcdf=$AMBERHOME gnu`.
Alternatively you can `./configure -no-netcdf gnu` to build without NetCDF.

## Usage
CreateRemdDirs has 3 modes: input Creation, job Submission, job Checking. There
are also 3 types of jobs: Runs, Analysis (--analyze), and Archiving (--archive).
Analysis is currently very basic and just consists of stripping, imaging, and 
sorting (if REMD). Archiving currently consists of placing the stripped/sorted
trajectories into a TAR/GZIP archive (along with any fully solvated trajectories
specified). The default mode is to Create input for Runs. A typical workflow might
consist of the following:
- Create run input.
- Submit run job.
- (After run completion) Check run.
- Create analysis/archive input.
- Submit analysis/archive job.

Help is available via the command line flags '-h' or '--help'. Help on options for
various input files is available via the command line flag '--full-help'.

# Author
-Daniel R. Roe

Disclaimer and Copyright
========================
CreateRemdDirs is Copyright (c) 2015 Daniel R. Roe.
The software is provided AS IS with no warranty.
CreateRemdDirs is free software and is licensed under the GPL v2 license (see LICENSE for details).

# Brief Example for REMD
CreateRemdDirs uses an input file (by default named remd.opts) that describes
how a run should be set up. For example, a 2D M-REMD run (temperature + Hamiltonian)
might look like so:
```
DIMENSION   Temperatures.dat
DIMENSION   Hamiltonians.dat
NSTLIM      500
DT          0.002
NUMEXCHG    100
TEMPERATURE 300.0
MDIN_FILE   pme.remd.in
FULLARCHIVE 0
```

## Creating Input
Basic usage is `CreateRemdDirs -b <start run #> -e <end run #>`. If only one run is desired
only `-b` need be specified. Runs are set up in subdirectories named run.XXX, where XXX is a
3 digit number corresponding to the run number. Input coordinates can be either specified
in the remd.opts file or on the command line with `-c` (path must be relative to inside
the run.XXX directories). Each run after the first is automatically set up to continue from
the previous run. Run #0 is special in that it will not be set up as a restart (irest = 0).

The MDIN_FILE should contain Amber MD input flags that belong in the &cntrl namelist. The
following flags are set from the remd.opts file and/or automatically: imin (always 0), 
nstlim, dt, irest, ntx, ig, numexchg, temp0, tempi. All other necessary flags should be
in MDIN_FILE. For example:
```
    ntwx = 5000, ioutfm = 1, ntwr = 100000, ntxo = 2, ntpr = 5000,
    iwrap = 1, nscm = 1000, 
    ntc = 2, ntf = 2, ntb = 1, cut = 8.0,
    ntt = 3, gamma_ln = 1, 
    ntp = 0,
```
Note that it is recommended that trajectory and restart files be written in NetCDF format
(ioutfm=1 and ntxo=2 respectively).

Each REMD run directory will contain a groupfile, input files (in an INPUT subdirectory),
a run script (RunMD.sh), and a remd.dim file for M-REMD runs. Output will also be into
subdirectories, namely INFO for mdinfo files, LOG for log files (PMEMD only),
OUTPUT for mdout files, RST for restart files, and TRAJ for trajectory files. An AMD
directory will be created for aMD output files. 

## Dimensions
The DIMENSION files describe what each dimension looks like. CreateRemdDirs currently
supports Temperature, Hamiltonian (Topology), and accelerated MD (aMD) dihedral boost.
A dimension file has one header line which describes what kind of dimension it is followed by lines
 describing how the dimension is set up. For example, a Temperature dimension looks like so:
```
#Temperature
277.00
281.30
285.70
290.20
```

A Hamiltonian dimension looks like so:
```
#Hamiltonian
../../AltDFC.01.PagF.TIP3P.ff14SB.parm7
../../AltDFC.02.PagF.TIP3P.ff14SB.parm7
```
Note that the paths in a Hamiltonian dimension need to be either absolute or relative to
the run directories, i.e. if you 'ls' the file from inside a run.XXX directory it will
show up. 

An aMD dihedral boost dimension looks like so:
```
#amd_dihedral
0.0 0.0
50.0 112.0000000000
```
where the first number corresponds to alpha and the second to the threshhold.

## Job Submission
CreateRemdDirs can automatically generate and submit run scripts for PBS and SLURM
using options defined in an input file (default 'qsub.opts'). An example looks like
so:
```
JOBNAME test
NODES 16
PPN 8
WALLTIME 24:00:00
EMAIL invalid@fake.com
ACCOUNT testaccount
PROGRAM pmemd
QSUB PBS 
MPIRUN mpiexec -n $THREADS
AMBERHOME /home/droe/Amber/GIT/amber
ANALYZE_FILE analyze.opts
ARCHIVE_FILE analyze.opts
```
System-wide options can be put into '~/default.qsub.opts', which will always be read.
At least JOBNAME and PROGRAM must be specified. The type of queuing system can be
changed simply by changing QSUB from PBS to SBATCH.

Jobs are submitted via the '--submit' command flag, e.g. `CreateRemdDirs -b 0 -e 1 --submit`
will submit the already-created input for run.000 and run.001. By default each subsequent
job depends on the previous job via batch system holds - this behavior can be changed with
the DEPEND input variable. Run input creation and job submission can also be accomplished
in one step via the '-s' flag, e.g. `CreateRemdDirs -b 0 -e 1 -s`.

## Job Check
This requires CreateRemdDirs to have been compiled with NetCDF and trajectories and restart
files are written in NetCDF format (ioutfm=1 and ntxo=2 respectively). Once a job has completed
runs can be checked via the '--check' flag, e.g. `CreateRemdDirs -b 0 -e 1 --check`. This will
check to see if the trajectory length matches what is expected based on input from 
existing output files. If the trajectory is short, restart times are checked to make sure they
are the same. Note that by default for speed only the first replica is checked; all replicas
can be checked by using the '--checkall' command line flag.
