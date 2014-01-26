Scout Reporting Data
====================

Scouts can return "reports" on demand as well as automatically trigger them when they detect state changes.

Each report is it's own minimal JSON object, they all must be <100 bytes serialized to fit on the RF mesh.

A report always contains a `"_":"***"` where *** is the type of report it is, all of which should be documented below.

### "s" - Scout

This report is sent upon startup.

* "e" - integer (1-255), EEPROM version
* "hv" - integer (1-255), hardware version
* "hf" - integer (1-64k), hardware family id
* "hs" - integer (big), hardware serial id

### "u" - Uptime

* "m" - integer, milliseconds since boot
* "c" - integer, number of commands executed since last boot
* "s" - string, restart method, "watchdog", "pin", "switch", "power"
* "f" - integer, free memory
* "r" - integer, random number

### "tmp" Temperature

* "t" - integer, celcius
* "h" - integer, high since last boot
* "l" - integer, low since last boot

Triggered every change of degree.

### "pwr" - Power

* "p" - integer from 0 to 100, battery percentage charged
* "v" - integer, battery voltage usually 370-420 (3.7V to 4.2V)
* "c" - bool, battery charging state
* "vcc" - bool, is vcc enabled
* "a" - bool, alarm triggered

Power reports are automatically sent every 10% of battery change, and every 1% when below 5.  They are also sent whenever charging is enabled/disabled.

### "led" - LED status

* "l" - array of [1,2,3], the rgb status of the main led colors (0-255 each)
* "t" - [1,2,3] rgb status of the torch colors

### "pin" - Pin Status

* "a" - array of all 8 analog pin values, each array entry is 0-1023, [0,0,0,255,0,42,0,128]
* "d" - array of all 7 digital pin values, each array entry is 0/1 (LOW/HIGH), [0,0,0,1,0,1,0]

This report is sent upon any pin change (by command or by hardware) and no more often than once every 10 seconds.

### "bps" - Backpacks List

A list of all attached backpacks is sent as one report

* "a" - array of all attached backpacks, ["wifi","env"]

This report is sent on startup, and whenever a backpack is added/removed.

### "bp" - Backpack Details

Each backpack attached generates it's own report (two backpacks on one scout would be two reports). These reports are sent on startup and whenever a backpack is added/removed.

* "b" - name of backpack, "wifi" or "env", etc
* "v" - version of backpack, float

There may be [more values](https://docs.google.com/document/d/1SVDNBB62NCAtVEWtao9h0Uc8jENlW3GP6svxdSTk-xc/edit#heading=h.9jwcjy4ekhf) based on each backpack (power usage, etc). 

* `"b":"wifi"`
  * "v" - version 
  * "c" - connection status, true/false
  * "n" - network (max 16 char)

### "rf" - RF Mesh

* "id" - device id, integer
* "p" - pan id, integer
* "c" - channel id, integer
* "tx" - transmit power, string
* "r" - data rate, string
* "t" - routing table size, integer

