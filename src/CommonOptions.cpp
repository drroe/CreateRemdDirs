#include "CommonOptions.h"

static Option<std::string> opt_runscriptname_ = Option<std::string>("RunMD.sh");

Option<std::string>& CommonOptions::Opt_RunScriptName() { return opt_runscriptname_; }

static Option<std::string> opt_jobidfilename_ = Option<std::string>("temp.jobid");

Option<std::string>& CommonOptions::Opt_JobIdFilename() { return opt_jobidfilename_; }

/** \return true if a recognized submit script name.
  * Keep in sync with Queue::SubmitCmd()
  */
bool CommonOptions::IsSubmitScript(std::string const& name) {
  if (name == "run.sh") return true;
  if (name == "sbatch.sh") return true;
  if (name == "qsub.sh") return true;
  return false;
}
