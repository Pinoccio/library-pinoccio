// TODO: this should be batch generated from a shell script based on the dir names or something :)

// this file is included in ModuleHandler.cpp to create any module on demand by name
#include <Scout.h>

// make sure to update Modules.h for includes too
#include "Modules.h"

PinoccioModule *ModulesNamed(const char *name)
{
  if(strcmp("hello",name) == 0) return (PinoccioModule*)(new HelloModule());
  if(strcmp("servo",name) == 0) return (PinoccioModule*)(new ServoModule());
  if(strcmp("wifi",name) == 0) return (PinoccioModule*)(new WifiModule());

  return NULL;
}

// TODO, combine these in a more automated way somehow?
void ModulesPrint()
{
  speol("hello");
  speol("servo");
  speol("wifi");
}

