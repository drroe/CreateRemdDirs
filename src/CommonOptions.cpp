#include "CommonOptions.h"

static Option<std::string> opt_runscriptname_ = Option<std::string>("RunMD.sh");

Option<std::string>& CommonOptions::Opt_RunScriptName() { return opt_runscriptname_; }
