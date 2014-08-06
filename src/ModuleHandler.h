/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2012-2014, Pinoccio Inc. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD License as described in license.txt.         *
\**************************************************************************/
#ifndef LIB_PINOCCIO_MODULE_HANDLER_H_
#define LIB_PINOCCIO_MODULE_HANDLER_H_

#include "util/LinkedList.h"
#include <modules/PinoccioModule.h>

// Forward declare
class PinoccioModuleInfoBase;

class ModuleHandler {
  public:
    ModuleHandler();

    static void setup();
    static void loop();

    static PinoccioModule *load(char *name);
    static bool loaded(char *name);
    static void list();
    static void loaded();

  protected:
    static LinkedList<PinoccioModule*> loadedModules;
    static const PinoccioModuleInfoBase* moduleInfo;
    friend class PinoccioModuleInfoBase;
};


/**
 * Superclass of all module info classes.
 */
class PinoccioModuleInfoBase {
  public:
    PinoccioModuleInfoBase(const char *name)
      : name(name), next(ModuleHandler::moduleInfo) {
        // Prepend to the moduleInfo linked list
        ModuleHandler::moduleInfo = this;
    }

    // TODO: We'd rather have this a PROGMEM string
    // (__FlashStringHelper), but using F() in global scope (as a
    // constructor argument) isn't allowed. Perhaps we can work around
    // this using a macro to declare an actual PROGMEM variable instead?
    const char * const name;

    // For the ModuleHandler::moduleInfo linked list
    const PinoccioModuleInfoBase * const next;

    // Create a new instance of the module described by this info
    virtual PinoccioModule* load() const = 0;
};

/**
 * Template for the actual module info classes. Each module creates its
 * own class from this template and should create a single static
 * instance of it. E.g., in a .cpp file:
 *
 * static PinoccioModuleInfoBase<HelloModule> helloInfo("modulename");
 */
template<class M>
class PinoccioModuleInfo : public PinoccioModuleInfoBase {
  public:
    explicit PinoccioModuleInfo(const char *name) : PinoccioModuleInfoBase(name) {}

    virtual PinoccioModule* load() const {
      return new M();
    }
};
#endif
