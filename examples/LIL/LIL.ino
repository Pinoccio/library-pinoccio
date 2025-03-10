#include <SPI.h>
#include <Wire.h>
#include <Scout.h>
#include <GS.h>
#include <bitlash.h>
#include <lwm.h>
#include <js0n.h>
#include <util/memdebug.h>

extern "C" {
  #include "lil.h"
}


lil_t lil;

// Forward declare to prevent Arduino IDE from adding one above the
// include...
void write(lil_t l, const char* s);
void write(lil_t l, const char* s) {
  Serial.write(s);
}


lil_value_t mem(lil_t lil, size_t argc, lil_value_t* argv);
lil_value_t mem(lil_t lil, size_t argc, lil_value_t* argv) {
  showMemory();
  return NULL;
}


void setup() {
  Scout.setup("LIL", "custom", 42);
  Shell.disableShell();
  lil = lil_new();

  lil_callback(lil, LIL_CALLBACK_WRITE, (lil_callback_proc_t)write);
  lil_register(lil, "mem", (lil_func_proc_t)mem);
}

void run_lil(const String& line) {
  lil_value_t ret = lil_parse(lil, line.c_str(), line.length(), 1);
  const char *msg;
  size_t pos;
  if (lil_error(lil, &msg, &pos)) {
    Serial.print("Error: ");
    Serial.println(msg);
  } else {
    Serial.println(lil_to_string(ret));
  }
  lil_free_value(ret);
}

static StringBuffer serialIncoming;
static String prevCommand;
static char lastc = 0;
static bool esc_sequence = false;

void loop() {
  while (Serial.available()) {
    char c = Serial.read();

    if (c == '\n' && lastc == '\r') {
      // Ignore the \n in \r\n to prevent interpreting \r\n as two
      // newlines. Note that we cannot just ignore newlines when the
      // buffer is empty, since that doesn't allow forcing a new
      // prompt by sending a newline (when the terminal is messed up
      // by debug output for example).
    } if (c == '\r' || c == '\n') {
      Serial.println();
      if (serialIncoming.length()) {
        run_lil(serialIncoming);
        prevCommand = serialIncoming;
        serialIncoming = (char*)NULL;
      }
      Shell.prompt();
    } else if (c == '\b') {
      if (serialIncoming.length()) {
        // Erase last character (backspace only moves the cursor back,
        // so print a space to actually erase)
        Serial.write("\b \b");
        serialIncoming.remove(serialIncoming.length() - 1);
      } else {
        // Nothing to erase, send a bell
        Serial.write('\a');
      }
    } else if (c == '\x1b') {
      esc_sequence = true;
    } else if (esc_sequence && lastc == '\x1b') {
      // Ignore the first character after the escape
    } else if (esc_sequence && lastc != '\x1b') {
      if (lastc == '[' && c == 'A') { // ESC[A == arrow up
        // Erase existing command
        Serial.write('\r');
        for (size_t i = 0; i < serialIncoming.length(); ++i)
          Serial.write(' ');
        Serial.write('\r');
        // Load and show previous command
        Shell.prompt();
        serialIncoming = prevCommand;
        Serial.print(serialIncoming);
      }
      esc_sequence = false;
    } else {
      Serial.write(c); // echo everything back
      serialIncoming += c;
    }
    lastc = c;
  }
  Scout.loop();
}
