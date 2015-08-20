#include <cstdlib> // atoi
#include "Submit.h"
#include "Messages.h"
#include "StringRoutines.h"

Submit::~Submit() {
  if (Run_ != 0) delete Run_;
  if (Analyze_ != 0) delete Analyze_;
  if (Archive_ != 0) delete Archive_;
}

int Submit::SubmitRuns(std::string const& top, StrArray const& RunDirs, int start) const
{
  Run_->CalcThreads();
  Run_->Info();
  std::string user = UserName();
  Msg("User: %s\n", user.c_str());
  std::string jobIdFilename(top + "/temp.jobid");
  std::string submitScript( std::string(Run_->SubmitCmd()) + ".sh" );
  std::string submitCommand( std::string(Run_->SubmitCmd()) + " " +
                             submitScript + " > " + jobIdFilename);
  // Set RUNTYPE-specific command line options
  std::string groupfileName("groupfile"); // TODO make these filenames options
  std::string remddimName("remd.dim");
  std::string cmd_opts;
  if (Run_->RunType() == MREMD)
    cmd_opts.assign("-ng $NG -groupfile " + groupfileName + " -remd-file " + remddimName);
  else if (Run_->RunType() == HREMD )
    cmd_opts.assign("-ng $NG -groupfile " + groupfileName + " -rem 3");
  else if (Run_->RunType() == TREMD )
    cmd_opts.assign("-ng $NG -groupfile " + groupfileName + " -rem 1");
  // Create run script for each run directory
  std::string previous_jobid;
  int run_num;
  if (start != -1)
    run_num = start;
  else
    run_num = 0;
  Msg("Submitting %zu runs.\n", RunDirs.size());
  StrArray::const_iterator finaldir = RunDirs.end() - 1;
  for (StrArray::const_iterator rdir = RunDirs.begin(); rdir != RunDirs.end(); ++rdir)
  {
    ChangeDir( top );
    // Check if run directories already contain scripts
    if ( !Run_->OverWrite() && fileExists( *rdir + "/" + submitScript) ) {
      ErrorMsg("Not overwriting (-O) and %s already contains %s\n",
               rdir->c_str(), submitScript.c_str());
      if (Run_->DependType() != NONE) // Exit if dependencies exist
        return 1;
      else
        continue;
    }
    Msg("  %s\n", rdir->c_str());
    ChangeDir( *rdir );
    // Run-specific sanity checks. For MREMD make sure groupfile and remd.dim
    // exist. For HREMD/TREMD make sure groupfile exists. For MD groupfile is
    // used to get command line options only.
    if (Run_->RunType() == TREMD || Run_->RunType() == HREMD) {
      if (CheckExists("groupfile", groupfileName)) return 1;
    } else if (Run_->RunType() == MREMD) {
      if (CheckExists("groupfile", groupfileName)) return 1;
      if (CheckExists("remd.dim", "remd.dim")) return 1;
    } else if (Run_->RunType() == MD) {
      if (!fileExists(groupfileName)) {
        ErrorMsg("groupfile does not exist for '%s'\n", rdir->c_str());
        if (Run_->DependType() != NONE) // Exit if dependencies exist
          return 1;
        else
          continue;
      }
      int nlines = 0;
      TextFile groupfile;
      if (groupfile.OpenRead(groupfileName)) return 1;
      const char* ptr = groupfile.Gets();
      while (ptr != 0) {
        nlines++;
        cmd_opts.assign( ptr );
        ptr = groupfile.Gets();
      }
      groupfile.Close();
      if (nlines > 1)
        cmd_opts.assign("-ng $NG -groupfile " + groupfileName);
    }
    // Set options specific to queuing system, node info, and Amber env.
    TextFile qout;
    if (qout.OpenWrite( submitScript )) return 1;
    if (Run_->QsubHeader(qout, run_num, previous_jobid)) return 1;
    // Set up run command TODO should be in Creation phase
    qout.Printf("\n# Run executable\nTIME0=`date +%%s`\n$MPIRUN $EXEPATH -O %s\n"
                "TIME1=`date +%%s`\n"
                "((TOTAL = $TIME1 - $TIME0))\necho \"$TOTAL seconds.\"\n\n",
                cmd_opts.c_str());
    // Set up script dependency if necessary
    if (Run_->DependType() == SUBMIT && rdir != finaldir) {
      std::string next_dir("../run." + integerToString(run_num+1, 3));
      qout.Printf("%s %s/%s\n", Run_->SubmitCmd(), next_dir.c_str(), submitScript.c_str());
    }
    qout.Printf("exit 0\n");
    qout.Close();
    ChangePermissions( submitScript );
    // Peform job submission if not testing
    if (testing_)
      Msg("Just testing. Skipping script submission.\n");
    else {
      Msg("%s\n", submitCommand.c_str()); 
      if ( system( submitCommand.c_str() ) ) {
        ErrorMsg("Job submission failed.\n");
        return 1;
      }
      // Get job ID of submitted job
      TextFile jobid;
      if (Run_->QueueType() == PBS) {
        if (jobid.OpenRead( jobIdFilename )) return 1;
        const char* ptr = jobid.Gets();
        if (ptr == 0) return 1;
        previous_jobid.assign(ptr);
      } else if (Run_->QueueType() == SLURM) {
        if (jobid.OpenPipe("squeue -u " + user + " --sort=i")) return 1;
        int cols = jobid.GetColumns(" \t");
        if (cols < 1) return 1;
        previous_jobid.assign( jobid.Token(0) );
      } else return 1; // sanity check
      jobid.Close();
      Msg("  Submitted: %s\n", previous_jobid.c_str());
      if (previous_jobid.empty() || previous_jobid == "JOBID") {
        ErrorMsg("Job not submitted.\n");
        return 1;
      }
      if (Run_->DependType() != BATCH) previous_jobid.clear();
    }
    ++run_num;
  }
  Msg("CmdOpts: %s\n", cmd_opts.c_str());

  return 0; 
}

int Submit::SubmitAnalysis(std::string const& top) {
  if (Analyze_ == 0) {
    ErrorMsg("No ANALYSIS_FILE set.\n");
    return 1;
  }
  Analyze_->SetRunType( ANALYSIS );
  Analyze_->CalcThreads();
  Analyze_->Info();
  return Analyze_->Submit( top );
}

int Submit::ReadOptions(std::string const& fn) {
  if (Run_ != 0) {
    ErrorMsg("Only one queue input allowed.\n");
    return 1;
  }
  Run_ = new QueueOpts();
  if (ReadOptions( fn, *Run_ )) return 1;
  if (Run_->Check()) return 1;
  if (Analyze_ != 0 && Analyze_->Check()) return 1;
  if (Archive_ != 0 && Archive_->Check()) return 1;
  return 0;
}

int Submit::ReadOptions(std::string const& fnameIn, QueueOpts& Qopt) {
  n_input_read_++;
  if (n_input_read_ > 100) {
    ErrorMsg("# of input files read > 100; possible infinite recursion.\n");
    return 1;
  }

  Msg("  Reading queue options from '%s'\n", fnameIn.c_str());
  std::string fname = tildeExpansion( fnameIn );
  if (CheckExists( "Queue options", fname )) return 1;
  TextFile infile;
  if (infile.OpenRead( fname )) return 1;
  const char* ptr = infile.Gets();
  while (ptr != 0) {
    // Format is <NAME> <OPTIONS>
    std::string line( ptr );
    size_t found = line.find_first_of(" ");
    if (found == std::string::npos) {
      ErrorMsg("malformed option: %s\n", ptr);
      return 1;
    }
    // Find first non-whitespace character
    size_t found1 = found;
    while (found1 < line.size() && line[found1] == ' ') ++found1;
    // Remove any newline chars and trailing whitespace
    std::string Args = NoTrailingWhitespace( line.substr(found1) );
    // Reduce line to option only
    line.resize(found);
    //Msg("Opt: '%s'   Args: '%s'\n", line.c_str(), Args.c_str());

    // Process options
    if (line == "ANALYZE_FILE") {
      if (Analyze_ != 0) {
        ErrorMsg("Only one ANALYZE_FILE allowed.\n");
        return 1;
      }
      Analyze_ = new QueueOpts(); // TODO Copy of existing?
      if (ReadOptions( Args, *Analyze_ )) return 1;
    } else if (line == "ARCHIVE_FILE") {
      if (Archive_ != 0) {
        ErrorMsg("Only one ARCHIVE_FILE allowed.\n");
        return 1;
      }
      Archive_ = new QueueOpts();
      if (ReadOptions( Args, *Archive_ )) return 1;
    } else if (line == "INPUT_FILE") {
      // Try to prevent recursion.
      std::string fn = tildeExpansion( Args );
      if (fn == fname) {
        ErrorMsg("An input file may not read from itself (%s).\n", Args.c_str());
        return 1;
      }
      if (ReadOptions( Args, Qopt )) return 1;
    } else {
      if (Qopt.ProcessOption(line, Args)) return 1;
    }
    
    ptr = infile.Gets();
  }
  infile.Close();

  return 0;
}
  

// =============================================================================
Submit::QueueOpts::QueueOpts() :
  nodes_(0),
  ng_(0),
  ppn_(0),
  threads_(0),
  runType_(MD),
  overWrite_(false),
  queueType_(PBS),
  isSerial_(false),
  dependType_(BATCH)
{}

const char* Submit::QueueOpts::RunTypeStr[] = {
  "MD", "TREMD", "HREMD", "MREMD", "ANALYSIS", "ARCHIVE"
};

const char* Submit::QueueOpts::QueueTypeStr[] = {
  "PBS", "SBATCH"
};

const char* Submit::QueueOpts::DependTypeStr[] = {
  "BATCH", "SUBMIT", "NONE"
};

const char* Submit::QueueOpts::SubmitCmdStr[] = {
  "qsub", "sbatch"
};

static inline int RetrieveOpt(const char** Str, int end, std::string const& VAR) {
  for (int i = 0; i != end; i++)
    if ( VAR.compare( Str[i] )==0 )
      return i;
  return end;
}

int Submit::QueueOpts::ProcessOption(std::string const& OPT, std::string const& VAR) {
  //Msg("Processing '%s' '%s'\n", OPT.c_str(), VAR.c_str()); // DEBUG

  if      (OPT == "JOBNAME") job_name_ = VAR;
  else if (OPT == "NODES"  ) nodes_ = atoi( VAR.c_str() );
  else if (OPT == "NG"     ) ng_ = atoi( VAR.c_str() );
  else if (OPT == "PPN"    ) ppn_ = atoi( VAR.c_str() );
  else if (OPT == "THREADS") threads_ = atoi( VAR.c_str() );
  else if (OPT == "RUNTYPE") {
    runType_ = (RUNTYPE)RetrieveOpt(RunTypeStr, NO_RUN, VAR);
    if (runType_ == NO_RUN) {
      ErrorMsg("Unrecognized run type: %s\n", VAR.c_str());
      return 1;
    }
  }
  else if (OPT == "AMBERHOME") {
    if ( CheckExists("AMBERHOME", VAR) ) return 1;
    amberhome_ = tildeExpansion( VAR );
  }
  else if (OPT == "PROGRAM"  ) program_ = VAR;
  else if (OPT == "QSUB"     ) {
    queueType_ = (QUEUETYPE) RetrieveOpt(QueueTypeStr, NO_QUEUE, VAR);
    if (queueType_ == NO_QUEUE) {
      ErrorMsg("Unrecognized QSUB: %s\n", VAR.c_str());
      return 1;
    }
  }
  else if (OPT == "WALLTIME"  ) walltime_ = VAR;
  else if (OPT == "NODEARGS"  ) nodeargs_ = VAR;
  else if (OPT == "MPIRUN"    ) mpirun_ = VAR;
  else if (OPT == "MODULEFILE") {
    if ( CheckExists( "Module file", VAR ) ) return 1;
    TextFile modfile;
    if (modfile.OpenRead( tildeExpansion(VAR) )) return 1;
    const char* ptr = modfile.Gets();
    while (ptr != 0) {
      additionalCommands_.append( std::string(ptr) );
      ptr = modfile.Gets();
    }
    modfile.Close();
  }
  else if (OPT == "ACCOUNT") account_ = VAR;
  else if (OPT == "EMAIL"  ) email_ = VAR;
  else if (OPT == "QUEUE"  ) queueName_ = VAR;
  else if (OPT == "SERIAL" ) isSerial_ = (bool)atoi( VAR.c_str() );
  else if (OPT == "DEPEND" ) {
    dependType_ = (DEPENDTYPE) RetrieveOpt(DependTypeStr, NO_DEP, VAR);
    if (dependType_ == NO_DEP) {
      ErrorMsg("Unrecognized DEPEND: %s\n", VAR.c_str());
      return 1;
    }
  }
/*  else if (OPT == "CHAIN"  ) {
    int ival = atoi( VAR.c_str() );
    if (ival == 1) dependType_ = SUBMIT;
  }
  else if (OPT == "NO_DEPEND") {
    if (atoi( VAR.c_str()) == 1)
      dependType_ = NO_DEPEND;
  }*/ 
  else if (OPT == "FLAG"     ) Flags_.push_back( VAR );
  else {
    ErrorMsg("Unrecognized option '%s' in input file.\n", OPT.c_str());
    return 1;
  }
  return 0;
}

int Submit::QueueOpts::Check() const {
  if (job_name_.empty()) {
    ErrorMsg("No job name\n");
    return 1;
  }
  if (program_.empty()) { // TODO check AMBERHOME ?
    ErrorMsg("PROGRAM not specified.\n");
    return 1;
  }
  if (!isSerial_ && mpirun_.empty()) {
    ErrorMsg("MPI run command MPIRUN not set.\n");
    return 1;
  }
  return 0;
}

int Submit::QueueOpts::Submit(std::string const& WorkDir) const {
  // Get user name from whoami command
  std::string user = UserName();
  Msg("User: %s\n", user.c_str());
  return 0;
}

void Submit::QueueOpts::Info() const {
  Msg("\n---=== Job Submission ===---\n");
  Msg("  RUNTYPE   : %s\n", RunTypeStr[runType_]);
  Msg("  JOBNAME   : %s\n", job_name_.c_str());
  if (nodes_ > 0  ) Msg("  NODES     : %i\n", nodes_);
  if (ng_ > 0     ) Msg("  NG        : %i\n", ng_);
  if (ppn_ > 0    ) Msg("  PPN       : %i\n", ppn_);
  if (threads_ > 0) Msg("  THREADS   : %i\n", threads_);
  if (!amberhome_.empty()) Msg("  AMBERHOME : %s\n", amberhome_.c_str());
  Msg("  PROGRAM   : %s\n", program_.c_str());
  Msg("  QSUB      : %s\n", QueueTypeStr[queueType_]);
  if (!walltime_.empty())  Msg("  WALLTIME  : %s\n", walltime_.c_str());
  if (!mpirun_.empty())    Msg("  MPIRUN    : %s\n", mpirun_.c_str());
  if (!nodeargs_.empty())  Msg("  NODEARGS  : %s\n", nodeargs_.c_str());
  if (!account_.empty())   Msg("  ACCOUNT   : %s\n", account_.c_str());
  if (!email_.empty())     Msg("  EMAIL     : %s\n", email_.c_str());
  if (!queueName_.empty()) Msg("  QUEUE     : %s\n", queueName_.c_str());
  Msg("  DEPEND    : %s\n", DependTypeStr[dependType_]);
}

void Submit::QueueOpts::CalcThreads() {
  if (threads_ < 1) {
    if (nodes_ > 0 || ppn_ > 0) {
      int N = 1, P = 1;
      if (nodes_ > 0) N = nodes_;
      if (ppn_ > 0) P = ppn_;
      threads_ = N * P;
    }
  }
  if (threads_ < 1)
    Msg("Warning: Less than 1 thread specified.\n");
}

void Submit::QueueOpts::AdditionalFlags(TextFile& qout) const {
  for (Sarray::const_iterator flag = Flags_.begin(); flag != Flags_.end(); ++flag)
    qout.Printf("#%s %s\n", QueueTypeStr[queueType_], flag->c_str());
}

int Submit::QueueOpts::QsubHeader(TextFile& qout, int run_num, std::string const& jobID)
{
  std::string job_title, previous_job;
  if (run_num > -1)
    job_title = job_name_ + "." + integerToString(run_num);
  else
    job_title = job_name_;
  if (dependType_ == BATCH )
    previous_job = jobID;
  // Queue specific options.
  // ----- PBS -----------------------------------
  if (queueType_ == PBS) {
    std::string resources("nodes=" + integerToString(nodes_));
    if (ppn_ > 0)
      resources.append(":ppn=" + integerToString(ppn_));
    resources.append(nodeargs_);
    qout.Printf("#PBS -S /bin/bash\n#PBS -l walltime=%s,%s\n#PBS -N %s\n#PBS -j oe\n",
                walltime_.c_str(), resources.c_str(), job_title.c_str());
    if (!email_.empty()) qout.Printf("#PBS -m abe\n#PBS -M %s\n", email_.c_str()); 
    if (!account_.empty()) qout.Printf("#PBS -A %s\n", account_.c_str());
    if (!previous_job.empty()) qout.Printf("#PBS -W depend=afterok:%s\n", previous_job.c_str());  
    if (!queueName_.empty()) qout.Printf("#PBS -q %s\n", queueName_.c_str());
    AdditionalFlags( qout );
    qout.Printf("\ncd $PBS_O_WORKDIR\n\n");
  }
  // ----- SLURM ---------------------------------
  else if (queueType_ == SLURM) {
    qout.Printf("#!/bin/bash\n#SBATCH -J %s\n#SBATCH -N %i\n#SBATCH -t %s\n",
                job_title.c_str(), nodes_, walltime_.c_str()); 
    if (threads_ > 0) qout.Printf("#SBATCH -n %i\n", threads_);
    if (!email_.empty())
      qout.Printf("#SBATCH --mail-user=%s\n#SBATCH --mail-type=all\n", email_.c_str());
    if (!account_.empty())
      qout.Printf("#SBATCH -A %s\n", account_.c_str());
    if (!previous_job.empty()) qout.Printf("#SBATCH -d afterok:%s\n", previous_job.c_str());
    if (!queueName_.empty()) qout.Printf("#SBATCH -p %s\n", queueName_.c_str());
    AdditionalFlags( qout );
    qout.Printf("\necho \"JobID: $SLURM_JOB_ID\"\necho \"NodeList: $SLURM_NODELIST\"\n"
                "cd $SLURM_SUBMIT_DIR\n\n");
  }
  // Set thread info, Amber environment
  if (ppn_ > 0) qout.Printf("PPN=%i\n", ppn_);
  if (nodes_ > 0) qout.Printf("NODES=%i\n", nodes_);
  if (threads_ > 0) qout.Printf("THREADS=%i\n", threads_);
  if (!amberhome_.empty()) {
    qout.Printf("export AMBERHOME=%s\n", amberhome_.c_str());
    if (fileExists(amberhome_ + "/amber.sh"))
      qout.Printf("source $AMBERHOME/amber.sh\n");
    else
      qout.Printf("export PATH=$AMBERHOME/bin:$PATH\n"); // TODO LD_LIBRARY_PATH?
    // Check if program exists
    std::string exepath = amberhome_ + "/bin/" + program_;
    if (CheckExists("Full program path", exepath)) return 1;
    qout.Printf("EXEPATH=%s\nls -l $EXEPATH\n", exepath.c_str());
  } else
    qout.Printf("EXEPATH=%s\nls -l `which $EXEPATH`\n", program_.c_str());
  // Add any additional input
  if (!additionalCommands_.empty())
    qout.Printf("\n%s\n\n", additionalCommands_.c_str());
  qout.Printf("MPIRUN=\"%s\"\n", mpirun_.c_str()); // TODO Combine with EXEPATH

  return 0;
}
