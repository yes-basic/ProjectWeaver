#include "Arduino.h"
#include "serialCommand.h"

serialCommand::serialCommand()
{
inputString.reserve(200);
}
String serialCommand::command(bool flush) {
  if(flush){
    return inputString;
  }
  
  if(inChar=='\n'){
    return "ready";
  }else{
    if(Serial.available()){
      while (Serial.available()) {
        // get the new byte:
        inChar = (char)Serial.read();
        Serial.print(inChar);
        // if the incoming character is a newline, return before
        if (inChar == '\n') {
          return "ready";
        }
        // add it to the inputString:
        inputString += inChar;
        return inputString;
      }
    }else{
      return inputString;
    }
  }
  return "failure";
}
int serialCommand::thing(int num){

}
