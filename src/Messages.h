#ifndef INC_MESSAGES_H
#define INC_MESSAGES_H
namespace Messages {

void ErrorMsg(const char*, ...);
void Msg(const char*, ...);
bool YesNoPrompt(const char*);

}
#endif
