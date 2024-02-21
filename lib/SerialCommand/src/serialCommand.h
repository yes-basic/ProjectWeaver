#ifndef serialCommand_h
#define serialCommand_h

#include "Arduino.h"

class serialCommand
{
  public:
    serialCommand();
    String command(bool flush);
    int thing(int num);
  private:
    String inputString;
    char inChar;
};

#endif