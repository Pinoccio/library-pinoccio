/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2012-2015, Pinoccio Inc. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD License as described in license.txt.         *
\**************************************************************************/
#include <Arduino.h>
#include <Scout.h>
#include <SPI.h>
#include "SD/SD.h"
#include "sd_mod.h"
#define BUFSIZE 32
int ctr = 0;

byte dataToWrite[BUFSIZE];
byte dataWritten[BUFSIZE];
using namespace pinoccio;

SDModule SDModule::instance;

const __FlashStringHelper *SDModule::name() const {
  return F("sdcard");
}

static numvar append() {
  if (getarg(0) < 1 || !isstringarg(1)) {
    speol("usage: sd.append(\"filename.txt\",\"data to write\")");
    return 0;
  }
  if (getarg(0) < 2) {
    speol("usage: sd.append(\"filename.txt\",\"data to write\")");
    return 0;
  }
  File f = SD.open((const char*)getstringarg(1), O_WRITE|O_CREAT|O_APPEND);

  if (!f) {
    speol("Failed to open file");
    return 0;
  }

  if (isstringarg(2))
    f.write((const char*)getstringarg(2));
  else
    f.print(getarg(2));
  f.close();
  return 1;
}

static numvar read() {
  if (getarg(0) < 1 || !isstringarg(1)) {
    speol("usage: sd.read(\"filename.txt\")");
    return 0;
  }
  File f = SD.open((const char*)getstringarg(1), FILE_READ);
  if (!f) {
    speol("Failed to open file");
    return 0;
  }

  while(f.available())
    spb((char)f.read());
  speol();
  f.close();
  return 1;
}

static numvar ls() {
  if (getarg(0) < 1 || !isstringarg(1)) {
    speol("usage: sd.ls(\"/home\")");
    return 0;
  }

  File f = SD.open((const char*)getstringarg(1));

  if (!f) {
    speol("Failed to open directory");
    return 0;
  }

  if (!f.isDirectory()) {
    speol("Not a directory");
    f.close();
    return 0;
  }

  // Work around a problem where reading or writing a file causes the
  // root directory position to somehow become at the end
  f.rewindDirectory();

  while(true) {
    File entry = f.openNextFile();
    if (!entry)
      break;

    if (entry.isDirectory())
      speol("DIR");
    else
      speol(entry.size(), DEC);

    speol("\t");
    speol(entry.name());
    entry.close();
  }

  f.close();
  return 1;
}

static numvar removeFile() {
  if (getarg(0) < 1 || !isstringarg(1)) {
    speol("usage: sd.rm(\"filename.txt\")");
    return 0;
  }
  SD.remove((char*)getstringarg(1));
  File f = SD.open((const char*)getstringarg(1), FILE_READ);
  if (f) { 
    speol("Failed to remove file");
    return 0;
  }
  f.close();
  return 1;
}

static numvar removeDir() {
  if (getarg(0) < 1 || !isstringarg(1)) {
    speol("usage: sd.rmdir(\"/some_folder\")");
    return 0;
  }
  if (!SD.rmdir((char*)getstringarg(1)))
  {
    speol("Failed to remove folder");
    return 0;
  }
  return 1;
}

static numvar makeDir() {
  if (getarg(0) < 1 || !isstringarg(1)) {
    speol("usage: sd.mkdir(\"/some_folder\")");
    return 0;
  }
  if (!SD.mkdir((char*)getstringarg(1)))
  {
    speol("Failed to create directory");
    return 0;
  }
  return 1;
}

static numvar sdInitialize(void) {
  if (!checkArgs(1, F("usage: sd.init(csPin)"))) {
    return 0;
  }
  if (!SD.begin(getarg(1))) {
    speol("No SD card found or initialization failed.");
    return 0;
  }
}

bool SDModule::enable() {
    Shell.addFunction("sd.init", sdInitialize);
    Shell.addFunction("sd.read", read);
    Shell.addFunction("sd.append", append);
    Shell.addFunction("sd.ls", ls);
    Shell.addFunction("sd.rm", removeFile);
    Shell.addFunction("sd.rmdir", removeDir);
    Shell.addFunction("sd.mkdir", makeDir);

}


void SDModule::loop() { }

