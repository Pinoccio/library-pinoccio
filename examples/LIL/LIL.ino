#include <SPI.h>
#include <Wire.h>
#include <Scout.h>
#include <GS.h>
#include <bitlash.h>
#include <lwm.h>
#include <js0n.h>

extern "C" {
	#include "lil.h"
}

#include "memdebug.h"

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
	lil = lil_new();

	lil_callback(lil, LIL_CALLBACK_WRITE, (lil_callback_proc_t)write);
	Serial.print(">> ");
	lil_register(lil, "mem", (lil_func_proc_t)mem);
}

void loop() {
	static String line;
	static char prev = 0;

	int c = Serial.read();
	if (c >= 0) {
		if (c == '\n' && prev == '\r') {
			// Nothing
		} else if (c == '\r' || c == '\n') {
			Serial.println();
			if (line.length()) {
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
				line = "";
			}
			Serial.print(">> ");
		} else if (c == 0x8) { // backspace
			if (line.length()) {
				line.remove(line.length() - 1);
				Serial.write(0x8);
				Serial.write(' ');
				Serial.write(0x8);
			}
		} else {
			line += (char)c;
			Serial.write(c);
		}
		prev = c;
	}
  Scout.loop();
}
