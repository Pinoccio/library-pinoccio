// DO NOT EDIT, auto-generated from rebuild.js

#include <Scout.h>
#include "Modules.h"

PinoccioModule *ModulesNamed(const char *name)
{
  if(strcmp("hello",name) == 0) return (PinoccioModule*)(new HelloModule());
  if(strcmp("motion",name) == 0) return (PinoccioModule*)(new MotionModule());
  if(strcmp("pixels",name) == 0) return (PinoccioModule*)(new PixelsModule());
  if(strcmp("serialflash",name) == 0) return (PinoccioModule*)(new SerialFlashModule());
  if(strcmp("servo",name) == 0) return (PinoccioModule*)(new ServoModule());
  if(strcmp("wifi",name) == 0) return (PinoccioModule*)(new WifiModule());
  return NULL;
}

void ModulesPrint()
{
  speol("hello");
  speol("motion");
  speol("pixels");
  speol("serialflash");
  speol("servo");
  speol("wifi");
}
