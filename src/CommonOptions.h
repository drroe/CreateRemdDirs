#ifndef INC_COMMONOPTIONS_H
#define INC_COMMONOPTIONS_H
#include "Option.h"
#include <string>
/// Hold common options (global)
namespace CommonOptions {

/// \return Run script name option
Option<std::string>& Opt_RunScriptName();
/// \return Job ID file name
Option<std::string>& Opt_JobIdFilename();

/// \return True if given name is a recognized submit script name; see Queue::SubmitCmd()
bool IsSubmitScript(std::string const&);
}
#endif
