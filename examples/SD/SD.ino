/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2014, Pinoccio Inc. All rights reserved.                   *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the MIT License as described in license.txt.         *
\**************************************************************************/

// Example usage
//     > sd.ls("/")
//     14      HELLO.TXT
//     > sd.read("hello.txt")
//     Hello, World!
//
//     > sd.append("foo.txt", "bar\r\n")
//     > sd.read("foo.txt")
//     bar
//
//     > sd.ls("/")
//     14      HELLO.TXT
//     5       FOO.TXT
//     > sd.append("foo.txt", "baz\r\n")
//     > sd.read("foo.txt")
//     bar
//     baz
//

#include <SPI.h>
#include <Wire.h>
#include <Scout.h>
#include <GS.h>
#include <bitlash.h>
#include <lwm.h>
#include <js0n.h>
#include <SD.h>

#include "version.h"

#include <SPI.h>

#define BUFSIZE 32
int ctr = 0;

byte dataToWrite[BUFSIZE];
byte dataWritten[BUFSIZE];

numvar read() {
  if (getarg(0) < 1 || !isstringarg(1)) {
    speol("No filename passed?");
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

numvar append() {
  if (getarg(0) < 1 || !isstringarg(1)) {
    speol("No filename passed?");
    return 0;
  }
  if (getarg(0) < 2) {
    speol("No data to write?");
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

numvar ls() {
  if (getarg(0) < 1 || !isstringarg(1)) {
    speol("No dirname passed?");
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
      Serial.print("DIR");
    else
      Serial.print(entry.size(), DEC);

    Serial.print("\t");
    Serial.println(entry.name());
    entry.close();
  }

  f.close();
  return 1;
}

void setup() {
  Scout.setup(SKETCH_NAME, SKETCH_REVISION, SKETCH_BUILD);
  // Add custom setup code here

  if (!Scout.isLeadScout()) {
    Serial.print("This sketch can only be run on a lead scout");
    return;
  }

  // Initialize SD card with slave-select pin on D8
  if (!SD.begin(8)) {
    Serial.println("No SD card found or initialization failed.");
    return;
  }

  Shell.addFunction("sd.read", read);
  Shell.addFunction("sd.append", append);
  Shell.addFunction("sd.ls", ls);
}

void loop() {
  Scout.loop();
  // Add custom loop code here
}
