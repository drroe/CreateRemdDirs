#include <cstdlib> // atoi
#include "Submit.h"
#include "Messages.h"
#include "StringRoutines.h"

using namespace Messages;
using namespace StringRoutines;
using namespace FileRoutines;

Submit::~Submit() {
  if (Run_ != 0) delete Run_;
  if (Analyze_ != 0) delete Analyze_;
  if (Archive_ != 0) delete Archive_;
}

void Submit::OptHelp() {
  Msg("Queue job submission input file variables:\n"
      "  INPUT_FILE <file>  : Read additional options from <file>.\n"
      "  JOBNAME <name>     : Job name in queueing system (required).\n"
      "  NODES <#>          : Number of nodes needed.\n"
      "  PPN <#>            : Processors per node needed.\n"
      "  THREADS <#>        : Number of threads needed. Calcd from NODES * PPN if not specified\n"
      "  AMBERHOME <dir>    : Directory containing AMBER installation.\n"
      "  PROGRAM <name>     : Name of binary to run (required).\n"
      "  QSUB <arg>         : Queue type {PBS | SBATCH (slurm)}\n"
      "  WALLTIME <arg>     : Wall time needed.\n"
      "  NODEARGS <arg>     : Any additonal -l node arguments (PBS only)\n"
      "  MPIRUN <command>   : Command used to execute parallel run. Can use\n"
      "                       $NODES, $THREADS, $PPN (will be set by script).\n"
      "  MODULEFILE <file>  : File containing extra commands to run (only last one loaded used)\n"
      "  COMMANDFILE <file> : File containing any additional commands to be run.\n"
      "  COMMAND <command>  : Additional command to run (can specify multiple).\n"
      "  ACCOUNT <name>     : Account name\n"
      "  EMAIL <email>      : User email address\n"
      "  QUEUE <name>       : Queue name\n"
      "  SERIAL {0|1}       : If set to 1 run in serial, no MPIRUN needed.\n"
      "  DEPEND <arg>       : Job dependencies. BATCH=Use batch system (default),\n"
      "                       SUBMIT=Execute next script at end of previous, or NONE.\n"
      "  FLAG <flag>        : Any additional queue flags.\n\n");
}

int Submit::SubmitRuns(std::string const& TopDir, StrArray const& RunDirs, int start, bool overwrite,
                       std::string const& prev_jobidIn)
const
{
  Run_->Info();
  std::string user = NoTrailingWhitespace( UserName() );
  Msg("User: %s\n", user.c_str());
  std::string jobIdFilename(TopDir + "/temp.jobid");
  std::string submitScript( std::string(Run_->SubmitCmd()) + ".sh" );
  std::string submitCommand( std::string(Run_->SubmitCmd()) + " " +
                             submitScript + " > " + jobIdFilename);
  std::string runScriptName("RunMD.sh");
  // Create run script for each run directory
  if (!prev_jobidIn.empty())
    Msg("First submission will depend on job id %s\n", prev_jobidIn.c_str());
  std::string previous_jobid = prev_jobidIn;
  int run_num;
  if (start != -1)
    run_num = start;
  else
    run_num = 0;
  Msg("Submitting %zu runs.\n", RunDirs.size());
  StrArray::const_iterator finaldir = RunDirs.end() - 1;
  for (StrArray::const_iterator rdir = RunDirs.begin(); rdir != RunDirs.end(); ++rdir)
  {
    ChangeDir( TopDir );
    // Check if run directories already contain scripts
    if ( !overwrite && fileExists( *rdir + "/" + submitScript) ) {
      ErrorMsg("Not overwriting (-O) and %s already contains %s\n",
               rdir->c_str(), submitScript.c_str());
      if (Run_->DependType() != NONE) // Exit if dependencies exist
        return 1;
      else
        continue;
    }
    Msg("  %s\n", rdir->c_str());
    ChangeDir( *rdir );
    // Ensure runscript exists.
    if (CheckExists("run script", runScriptName)) return 1;
    // Set options specific to queuing system, node info, and Amber env.
    TextFile qout;
    if (qout.OpenWrite( submitScript )) return 1;
    if (Run_->QsubHeader(qout, run_num, previous_jobid, "")) return 1;
    // Set up command to execute run script
    qout.Printf("\n# Run executable\n./%s\n\n", runScriptName.c_str());
    // Set up script dependency if necessary
    if (Run_->DependType() == SUBMIT && rdir != finaldir) {
      std::string next_dir("../" + *(rdir+1));
      qout.Printf("cd %s && %s %s\n", next_dir.c_str(), Run_->SubmitCmd(), submitScript.c_str());
    }
    qout.Printf("exit 0\n");
    qout.Close();
    ChangePermissions( submitScript );
    // Peform job submission if not testing
    if (testing_)
      Msg("Just testing. Skipping script submission.\n");
    else if (Run_->DependType() == SUBMIT && rdir != RunDirs.begin())
      Msg("Job will be submitted when previous job completes.\n");
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
        // -i inidcates reverse sort
        if (jobid.OpenPipe("squeue -u " + user + " --sort=-i")) return 1;
        const char* ptr = jobid.Gets();     // Header with JOBID
        if (ptr == 0) return 1;
        int cols = jobid.GetColumns(" \t"); // Should be last submitted job
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

  return 0; 
}

int Submit::SubmitAnalysis(std::string const& TopDir, int start, int stop, bool overwrite) const
{
  if (Analyze_ == 0) {
    ErrorMsg("No ANALYSIS_FILE set.\n");
    return 1;
  }
  Analyze_->Info();
  ChangeDir( TopDir );
  // Check that analysis directory, input, and script exist.
  std::string suffix(integerToString(start) + "." + integerToString(stop));
  std::string CPPDIR("Analyze." + suffix);
  if (CheckExists("analysis input directory", CPPDIR)) return 1;
  std::string inputName("batch.cpptraj.in"); // TODO make option
  if (CheckExists("analysis input file", CPPDIR + "/" + inputName)) return 1;
  std::string scriptName("RunAnalysis.sh"); // TODO make option
  if (CheckExists("analysis script", CPPDIR + "/" + scriptName)) return 1;

  // Set options specific to queuing system, node info, and Amber env.
  std::string qName( std::string(Analyze_->SubmitCmd()) + ".sh" );
  std::string qNamePath( CPPDIR + "/" + qName );
  if (!overwrite && fileExists( qNamePath )) {
    ErrorMsg("Not overwriting existing script %s\n", qNamePath.c_str());
    return 1;
  }
  TextFile qout;
  if (qout.OpenWrite( qNamePath )) return 1;
  if (Analyze_->QsubHeader(qout, -1, std::string(), "proc." + suffix + ".")) return 1;
  qout.Printf("\n# Run script\n./%s\nexit $?\n", scriptName.c_str());
  qout.Close();
  ChangePermissions( qNamePath );
  // Submit job
  if (testing_)
    Msg("Just testing; not submitting analysis job.\n");
  else {
    ChangeDir( CPPDIR );
    std::string submitCommand( std::string(Analyze_->SubmitCmd()) + " " + qName );
    if ( system( submitCommand.c_str() ) ) {
      ErrorMsg("Analysis job submission failed.\n");
      return 1;
    }
  }
  return 0;
}

int Submit::SubmitArchive(std::string const& TopDir, int start, int stop, bool overwrite) const
{
  if (Archive_ == 0) {
    ErrorMsg("No ARCHIVE_FILE set.\n");
    return 1;
  }
  Archive_->Info();
  ChangeDir( TopDir );
  // TODO: Require or have option for dependency on analysis job.
  // Check that archive dir, input, and run script exist
  std::string suffix(integerToString(start) + "." + integerToString(stop));
  std::string ARDIR("Archive." + suffix);
  if (CheckExists("archive input directory", ARDIR)) return 1;
  std::string scriptName("RunArchive." + suffix + ".sh");
  if (CheckExists("archive run script", scriptName)) return 1;
  StrArray ar1_files = ExpandToFilenames(ARDIR + "/ar1.*.in");
  StrArray ar2_files = ExpandToFilenames(ARDIR + "/ar2.*.in");
  if (ar2_files.empty()) {
    ErrorMsg("No archive input found in %s\n", ARDIR.c_str());
    return 1;
  }
  if (!ar1_files.empty() && ar2_files.size() != ar1_files.size()) {
    ErrorMsg("Number of full archive input files %zu != number of stripped archive inputs %zu\n",
             ar1_files.size(), ar2_files.size());
    return 1;
  }

  // Set options specific to queuing system, node info, and Amber env.
  std::string qName("archive." + std::string(Archive_->SubmitCmd()) + "." + suffix + ".sh");
  if (!overwrite && fileExists( qName )) {
    ErrorMsg("Not overwriting existing script %s\n", qName.c_str());
    return 1;
  }
  TextFile qout;
  if (qout.OpenWrite( qName )) return 1;
  if (Archive_->QsubHeader(qout, -1, std::string(), "ar." + suffix + ".")) return 1;
  qout.Printf("\n# Run script\n./%s\nexit $?\n", scriptName.c_str());
  qout.Close();
  ChangePermissions( qName );
  // Submit job
  if (testing_)
    Msg("Just testing; not submitting archive job.\n");
  else {
    std::string submitCommand( std::string(Archive_->SubmitCmd()) + " " + qName );
    if ( system( submitCommand.c_str() ) ) {
      ErrorMsg("Archive job submission failed.\n");
      return 1;
    }
  }

  return 0;
}

int Submit::ReadOptions(std::string const& fn) {
  Msg("  Reading queue options from '%s'\n", fn.c_str());
  n_input_read_ = 0;
  if (Run_ == 0) Run_ = new QueueOpts();
  if (ReadOptions( fn, *Run_ )) return 1;
  return 0;
}

int Submit::CheckOptions() {
  if (Run_ == 0) return 1;
  if (Run_->Check()) return 1;
  Run_->CalcThreads();
  if (Analyze_ != 0) {
    if (Analyze_->Check()) return 1;
    Analyze_->CalcThreads();
  }
  if (Archive_ != 0) {
    if (Archive_->Check()) return 1;
    Archive_->CalcThreads();
  }
  return 0;
}

int Submit::ReadOptions(std::string const& fnameIn, QueueOpts& Qopt) {
  n_input_read_++;
  if (n_input_read_ > 100) {
    ErrorMsg("# of input files read > 100; possible infinite recursion.\n");
    return 1;
  }

  if (debug_ > 0)
    Msg("  Reading queue options from '%s'\n", fnameIn.c_str());
  std::string fname = tildeExpansion( fnameIn );
  if (CheckExists( "Queue options", fname )) return 1;
  TextFile infile;
  TextFile::OptArray Options = infile.GetOptionsArray(fname, debug_);
  if (Options.empty()) return 1;
  for (TextFile::OptArray::const_iterator opair = Options.begin(); opair != Options.end(); ++opair)
  {
    std::string const& line = opair->first;
    std::string const& Args = opair->second;
    if (debug_ > 0)
      Msg("Opt: '%s'   Args: '%s'\n", line.c_str(), Args.c_str());

    // Process options
    if (line == "ANALYZE_FILE") {
      if (Analyze_ != 0) {
        ErrorMsg("Only one ANALYZE_FILE allowed.\n");
        return 1;
      }
      Analyze_ = new QueueOpts(Qopt);
      if (ReadOptions( Args, *Analyze_ )) return 1;
    } else if (line == "ARCHIVE_FILE") {
      if (Archive_ != 0) {
        ErrorMsg("Only one ARCHIVE_FILE allowed.\n");
        return 1;
      }
      Archive_ = new QueueOpts(Qopt);
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
  }

  return 0;
}
  

// =============================================================================
Submit::QueueOpts::QueueOpts() :
  nodes_(0),
  ppn_(0),
  threads_(0),
  queueType_(PBS),
  isSerial_(false),
  dependType_(BATCH)
{}

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
  else if (OPT == "PPN"    ) ppn_ = atoi( VAR.c_str() );
  else if (OPT == "THREADS") threads_ = atoi( VAR.c_str() );
  else if (OPT == "RUNTYPE") {
    ErrorMsg("RUNTYPE is obsolete. Please remove.\n");
    return 1;
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
  else if (OPT == "WALLTIME"   ) walltime_ = VAR;
  else if (OPT == "NODEARGS"   ) nodeargs_ = VAR;
  else if (OPT == "MPIRUN"     ) mpirun_ = VAR;
  else if (OPT == "MODULEFILE" ) modfileName_ = VAR;
  else if (OPT == "COMMANDFILE") {
    if ( CheckExists( "Additional commands file", VAR ) ) return 1;
    Msg("  Reading commands from file %s\n", VAR.c_str());
    TextFile cmdfile;
    if (cmdfile.OpenRead( tildeExpansion(VAR) )) return 1;
    const char* ptr = cmdfile.Gets();
    while (ptr != 0) {
      additionalCommands_.append( std::string(ptr) );
      ptr = cmdfile.Gets();
    }
    cmdfile.Close();
  }
  else if (OPT == "COMMAND") {
    additionalCommands_.append( VAR );
    additionalCommands_.append("\n");
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
  if (queueType_ == PBS) {
    if (nodes_ < 1) {
      ErrorMsg("Less than 1 node specified (use NODES option).\n");
      return 1;
    }
  } else if (queueType_ == SLURM) {
    if (nodes_ < 1 && threads_ < 1) {
      ErrorMsg("Must specify either NODES or THREADS.\n");
      return 1;
    }
  }
  return 0;
}

void Submit::QueueOpts::Info() const {
  Msg("\n---=== Job Submission ===---\n");
  Msg("  JOBNAME   : %s\n", job_name_.c_str());
  if (nodes_ > 0  ) Msg("  NODES     : %i\n", nodes_);
  if (ppn_ > 0    ) Msg("  PPN       : %i\n", ppn_);
  if (threads_ > 0) Msg("  THREADS   : %i\n", threads_);
  if (!amberhome_.empty()) Msg("  AMBERHOME : %s\n", amberhome_.c_str());
  Msg("  PROGRAM   : %s\n", program_.c_str());
  Msg("  QSUB      : %s\n", QueueTypeStr[queueType_]);
  if (!walltime_.empty())    Msg("  WALLTIME  : %s\n", walltime_.c_str());
  if (!mpirun_.empty())      Msg("  MPIRUN    : %s\n", mpirun_.c_str());
  if (!nodeargs_.empty())    Msg("  NODEARGS  : %s\n", nodeargs_.c_str());
  if (!account_.empty())     Msg("  ACCOUNT   : %s\n", account_.c_str());
  if (!email_.empty())       Msg("  EMAIL     : %s\n", email_.c_str());
  if (!queueName_.empty())   Msg("  QUEUE     : %s\n", queueName_.c_str());
  if (!modfileName_.empty()) Msg("  MODULEFILE: %s\n", modfileName_.c_str());
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

int Submit::QueueOpts::QsubHeader(TextFile& qout, int run_num, std::string const& jobID,
                                  std::string const& namePrefix)
{
  std::string job_title, previous_job;
  if (run_num > -1)
    job_title = namePrefix + job_name_ + "." + integerToString(run_num);
  else
    job_title = namePrefix + job_name_;
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
    qout.Printf("#!/bin/bash\n#SBATCH -J %s\n", job_title.c_str());
    if (nodes_ > 0) qout.Printf("#SBATCH -N %i\n", nodes_);
    qout.Printf("#SBATCH -t %s\n", walltime_.c_str());
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
  // Set thread info
  if (ppn_ > 0) qout.Printf("PPN=%i\n", ppn_);
  if (nodes_ > 0) qout.Printf("NODES=%i\n", nodes_);
  if (threads_ > 0) qout.Printf("THREADS=%i\n", threads_);
  // If AMBERHOME is set, set the EXE path
  if (!amberhome_.empty()) {
    qout.Printf("export AMBERHOME=%s\n", amberhome_.c_str());
    if (fileExists(amberhome_ + "/amber.sh"))
      qout.Printf("source $AMBERHOME/amber.sh\n");
    else
      qout.Printf("export PATH=$AMBERHOME/bin:$PATH\n"); // TODO LD_LIBRARY_PATH?
    // Check if program exists
    std::string exepath = amberhome_ + "/bin/" + program_;
    if (CheckExists("Full program path", exepath)) return 1;
    qout.Printf("export EXEPATH=%s\nls -l $EXEPATH\n", exepath.c_str());
  }
  // Add any module file commands
  if (!modfileName_.empty()) {
    Msg("  Reading module file %s\n", modfileName_.c_str());
    TextFile modFile;
    if (modFile.OpenRead( tildeExpansion(modfileName_) )) return 1;
    const char* ptr = modFile.Gets();
    while (ptr != 0) {
      qout.Printf("%s", ptr);
      ptr = modFile.Gets();
    }
    modFile.Close();
  }
  // Add any additional input
  if (!additionalCommands_.empty())
    qout.Printf("\n%s\n\n", additionalCommands_.c_str());
  qout.Printf("export MPIRUN=\"%s\"\n", mpirun_.c_str()); // TODO Combine with EXEPATH
  // Set EXE path here if AMBERHOME not set
  if (amberhome_.empty())
    qout.Printf("export EXEPATH=`which %s`\nls -l $EXEPATH\n", program_.c_str());

  return 0;
}
