// TODO: this should be batch generated from a shell script based on the dir names or something :)

// this file is included in ModuleHandler.cpp to create any module on demand by name
#include "PinoccioModule.h"

// make sure to update Modules.cpp too
#include "Servo/Servo.h"
#include "Hello/Hello.h"
#include "Wifi/Wifi.h"


PinoccioModule *ModulesNamed(const char *name);
void ModulesPrint();
