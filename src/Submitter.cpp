#include "Submitter.h"
#include "Messages.h"
#include "FileRoutines.h"
#include "TextFile.h"
#include "StringRoutines.h"
#include "CommonOptions.h"

using namespace Messages;
using namespace FileRoutines;
using namespace StringRoutines;

/** CONSTRUCTOR */
Submitter::Submitter() :
  debug_(0),
  nodes_(0),
  procs_(0),
  dependType_(NO_DEPENDS)
{}

/** KEEP IN SYNC WITH DependType */
const char* Submitter::DependTypeStr_[] = {
  "BATCH", "SUBMIT", "NONE"
};

/** Set debug level */
void Submitter::SetDebug(int debugIn) {
  debug_ = debugIn;
}

/** Print help to stdout. */
void Submitter::OptHelp() {
  Msg("Queue job submission input file variables:\n"
      "  INPUT_FILE <file>          : Read additional options from <file>.\n"
      "  JOBNAME <name>             : Job name in queueing system (required).\n"
      "  NODES <#>                  : Number of nodes needed.\n"
      "  PROCS <#>                  : Number of processes needed.\n"
      "                               Determined from NODES * PPN if not specified.\n"
      "  PROGRAM <name>             : Name of binary to run (required).\n"
      "  MPIRUN <command>           : Command used to execute parallel run. Can use\n"
      "                               $NODES, $PROCS, $PPN (will be set by script).\n"
      "  ACCOUNT <name>             : Account name\n"
      "  EMAIL <email>              : User email address\n"
      "  DEPEND {BATCH|SUBMIT|NONE} : Job dependencies. BATCH=Use batch system (default),\n"
      "                               SUBMIT=Execute next script at end of previous, or NONE.\n"
     );
  Queue::OptHelp();
  Msg("\n");
}

/** Parse option from a file.
  * \return 1 if option was parsed, 0 if ignored, -1 if error.
  */
int Submitter::ParseFileOption( OptArray::OptPair const& opair ) {
  std::string const& OPT = opair.first;
  std::string const& VAR = opair.second;
  //if (debug_ > 0) // FIXME
    Msg("    Option: %s  Variable: %s\n", OPT.c_str(), VAR.c_str());
  if (OPT == "INPUT_FILE") {
    Msg("Warning: INPUT_FILE is only processed when read from a Submit options file.\n");
  } else if (OPT == "JOBNAME") {
    job_name_ = VAR;
  } else if (OPT == "NODES") {
    nodes_ = convertToInteger( VAR );
  } else if (OPT == "PROCS") {
    // NOTE: Previously was threads
    procs_ = convertToInteger( VAR );
  } else if (OPT == "PROGRAM") {
    program_ = VAR;
  } else if (OPT == "WALLTIME") {
    walltime_ = VAR;
  } else if (OPT == "ACCOUNT") {
    account_ = VAR;
  } else if (OPT == "EMAIL") {
    email_ = VAR;
  } else if (OPT == "MPIRUN") {
    mpirun_ = VAR;
  } else if (OPT == "DEPEND") {
    if (VAR == "BATCH")
      dependType_ = BATCH;
    else if (VAR == "SUBMIT")
      dependType_ = SUBMIT;
    else if (VAR == "NONE")
      dependType_ = NO_DEPENDS;
    else {
      ErrorMsg("Unrecognized variable %s for option %s\n", VAR.c_str(), OPT.c_str());
      return -1;
    }
  } else {
    return 0;
  }
  return 1;
}

/** Read queue/job options from a file. */
int Submitter::ReadOptions(std::string const& input_file) {
  // TODO clear previous options?
  // Read options from input file
  if (CheckExists("Submit options file", input_file)) return 1;
  std::string fname = tildeExpansion( input_file );
  Msg("Reading Submit options from file: %s\n", fname.c_str());
  TextFile infile;
  OptArray Options = infile.GetOptionsArray(fname, debug_);
  if (Options.empty()) return 1;
  for (OptArray::const_iterator opair = Options.begin(); opair != Options.end(); ++opair)
  {
    if (opair->first == "INPUT_FILE") {
      // Try to prevent recursion
      std::string fn = tildeExpansion( opair->second );
      if (fn == fname) {
        ErrorMsg("An input file may not read from itself (%s)\n", opair->second.c_str());
        return 1;
      }
      if (ReadOptions( fn )) return 1;
    } else {
      int ret = ParseFileOption( *opair );
      if (ret == -1) {
        ErrorMsg("Could not parse option '%s' = '%s'\n", opair->first.c_str(), opair->second.c_str());
        return 1;
      } else if (ret == 0) {
        // See if this is a local queue option
        ret = localQueue_.ParseOption(opair->first, opair->second);
        if (ret == -1) {
          ErrorMsg("Could not parse queue option '%s' = '%s'\n", opair->first.c_str(), opair->second.c_str());

          return 1;
        } else if (ret == 0) {
          Msg("Warning: Ignoring unrecognized Submit option '%s' = '%s'\n", opair->first.c_str(), opair->second.c_str());
        }
      }
    }
  } // END loop over file options

  //if (setupSubmitter()) {
  //  Msg("Warning: Invalid of missing options in file '%s'\n", input_file.c_str());
  //}
  return 0;
}

/** Check that submitter is valid. */
int Submitter::CheckSubmitter() {
  int errcount = 0;
  // Set user if needed
  if (user_.empty())
    user_ = NoTrailingWhitespace( UserName() );
  // Check required options
  if (job_name_.empty()) {
    ErrorMsg("No JOBNAME specified.\n");
    errcount++;
  }
  if (program_.empty()) {
    ErrorMsg("No PROGRAM specified.\n");
    errcount++;
  }
  // Check queue
  if (!localQueue_.IsValid()) {
    ErrorMsg("Invalid queue.\n");
    errcount++;
  }

  return errcount;
}

/** Print options to stdout. */
void Submitter::Info() const {
  Msg("Submitter options:\n");
  Msg(  "  JOBNAME   : %s\n", job_name_.c_str());
  Msg(  "  USER      : %s\n", user_.c_str());
  Msg(  "  DEPEND    : %s\n", DependTypeStr_[dependType_]);
  Msg(  "  PROGRAM   : %s\n", program_.c_str());
  if (!mpirun_.empty())
    Msg("  MPIRUN    : %s\n", mpirun_.empty());
  if (nodes_ > 0)
    Msg("  NODES     : %i\n", nodes_);
  if (procs_ > 0)
    Msg("  PROCS     : %i\n", procs_);
  if (!walltime_.empty())
    Msg("  WALLTIME  : %s\n", walltime_.c_str());
  if (!email_.empty())
    Msg("  EMAIL     : %s\n", email_.c_str());
  if (!account_.empty())
    Msg("  ACCOUNT   : %s\n", account_.c_str());
  localQueue_.Info();
}

/** Write queue header */
int Submitter::writeHeader(TextFile& qout, int run_num, std::string const& prev_jobidIn, int myprocs) const {
  std::string job_title = job_name_ + "." + integerToString(run_num);
  std::string previous_job;
  if (dependType_ == BATCH)
    previous_job = prev_jobidIn;

  // Queue-specific options
// ----- PBS -----------------------------------
  if (localQueue_.QueueType() == Queue::PBS) {
    std::string resources("nodes=" + integerToString(nodes_));
    if (localQueue_.PPN() > 0)
      resources.append(":ppn=" + integerToString(localQueue_.PPN()));
    //resources.append(nodeargs_);
    qout.Printf("#PBS -S /bin/bash\n#PBS -l walltime=%s,%s\n#PBS -N %s\n#PBS -j oe\n",
                walltime_.c_str(), resources.c_str(), job_title.c_str());
    if (!email_.empty()) qout.Printf("#PBS -m abe\n#PBS -M %s\n", email_.c_str()); 
    if (!account_.empty()) qout.Printf("#PBS -A %s\n", account_.c_str());
    if (!previous_job.empty()) qout.Printf("#PBS -W depend=afterok:%s\n", previous_job.c_str());  
    if (!localQueue_.Name().empty()) qout.Printf("#PBS -q %s\n", localQueue_.Name().c_str());
    localQueue_.AdditionalFlags( qout );
    qout.Printf("\ncd $PBS_O_WORKDIR\n\n");
  }
  // ----- SLURM ---------------------------------
  else if (localQueue_.QueueType() == Queue::SLURM) {
    qout.Printf("#!/bin/bash\n#SBATCH -J %s\n", job_title.c_str());
    if (nodes_ > 0) qout.Printf("#SBATCH -N %i\n", nodes_);
    qout.Printf("#SBATCH -t %s\n", walltime_.c_str());
    if (myprocs > 0) qout.Printf("#SBATCH -n %i\n", myprocs);
    if (!email_.empty())
      qout.Printf("#SBATCH --mail-user=%s\n#SBATCH --mail-type=all\n", email_.c_str());
    if (!account_.empty())
      qout.Printf("#SBATCH -A %s\n", account_.c_str());
    if (!previous_job.empty()) qout.Printf("#SBATCH -d afterok:%s\n", previous_job.c_str());
    if (!localQueue_.Name().empty()) qout.Printf("#SBATCH -p %s\n", localQueue_.Name().c_str());
    localQueue_.AdditionalFlags( qout );
    qout.Printf("\necho \"JobID: $SLURM_JOB_ID\"\necho \"NodeList: $SLURM_NODELIST\"\n"
                "cd $SLURM_SUBMIT_DIR\n\n");
  } else {
    qout.Printf("#!/bin/bash\n# %s\n\n", job_title.c_str());
  }
 
  return 0;
} 

/** Submit job, set job id */
int Submitter::SubmitJob(std::string& jobid, std::string const& prev_jobidIn, int run_num, std::string const& next_dir) const {
  // Ensure the MD run script exists
  std::string runScriptName = CommonOptions::Opt_RunScriptName().Val();
  if (!fileExists( runScriptName )) {
    ErrorMsg("Run script %s not found.\n");
    return 1;
  }
  // Calculate procs if needed
  int myprocs = 0;
  if (procs_ > 0)
    myprocs = procs_;
  else {
    if (nodes_ > 0 || localQueue_.PPN() > 0) {
      int iN = 1;
      int iP = 1;
      if (nodes_ > 0) iN = nodes_;
      if (localQueue_.PPN() > 0) iP = localQueue_.PPN();
      myprocs = iN * iP;
    }
  }
  if (localQueue_.QueueType() != Queue::NO_QUEUE && myprocs < 1) {
    Msg("Warning: Less than 1 process specified.\n");
  }
  // Create submit script name
  std::string submitScript( localQueue_.SubmitCmd() + ".sh" );
  Msg("DEBUG: submit script: %s\n", submitScript.c_str());
  if (fileExists( submitScript ))
    Msg("Warning: Overwriting %s\n", submitScript.c_str());
  // Write the run script
  TextFile qout;
  if (qout.OpenWrite( submitScript )) return 1;
  if (writeHeader(qout, run_num, prev_jobidIn, myprocs)) return 1;
  // Write additional commands
  if (!localQueue_.AdditionalCommands().empty()) {
    for (Queue::Sarray::const_iterator it = localQueue_.AdditionalCommands().begin();
                                       it != localQueue_.AdditionalCommands().end(); ++it)
      qout.Printf("%s\n", it->c_str());
    qout.Printf("\n");
  }
  // Export EXEPATH and MPIRUN
  qout.Printf("export EXEPATH=%s\n", program_.c_str());
  if (!mpirun_.empty()) {
    if (localQueue_.PPN() > 0) qout.Printf("PPN=%i\n", localQueue_.PPN());
    if (nodes_ > 0) qout.Printf("NODES=%i\n", nodes_);
    if (myprocs > 0) qout.Printf("PROCS=%i\n", myprocs);
    qout.Printf("export MPIRUN=\"%s\"\n", mpirun_.c_str());
  }
  // Command to run MD script
  qout.Printf("\n# Run executable\n./%s\n\n", runScriptName.c_str());
  // Set up dependency if necessary
  if (dependType_ == SUBMIT && !next_dir.empty()) {
      qout.Printf("cd ../%s && %s %s\n", next_dir.c_str(), localQueue_.SubmitCmd().c_str(), submitScript.c_str());
  }
  qout.Printf("exit 0\n"); // TODO capture run script error stat
  
  qout.Close();
  return 0;
}
