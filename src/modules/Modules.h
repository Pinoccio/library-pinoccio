// this file is included in ModuleHandler.cpp to create any module on demand by name
#include "PinoccioModule.h"

// make sure to update Modules.cpp too
#include "Servo/Servo.h"
#include "Hello/Hello.h"


PinoccioModule *ModulesNamed(const char *name);
void ModulesPrint();
