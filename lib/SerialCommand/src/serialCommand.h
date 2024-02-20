#ifndef serialCommand_h
#define serialCommand_h

#include "Arduino.h"

class serialCommand
{
  public:
    serialCommand(int num);
    int number();
  private:
  int _num;
};

#endif