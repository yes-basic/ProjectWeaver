#include "Arduino.h"
#include "serialCommand.h"

serialCommand::serialCommand(int num)
{
    _num=num;
}
int serialCommand::number()
{
return _num;
}