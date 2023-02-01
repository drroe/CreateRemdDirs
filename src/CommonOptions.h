#ifndef INC_COMMONOPTIONS_H
#define INC_COMMONOPTIONS_H
#include "Option.h"
#include <string>
/// Hold common options (global)
namespace CommonOptions {

/// \return Run script name option
Option<std::string>& Opt_RunScriptName();

}
#endif
