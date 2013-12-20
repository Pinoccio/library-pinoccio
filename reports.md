Scout Reporting Data
====================

Scouts can return "reports" on demand as well as automatically trigger them when they detect state changes.

Each report is it's own minimal JSON object, they all must be <100 bytes serialized to fit on the RF mesh.

A report always contains a `"t":"***"` where *** is the type of report it is, all of which should be documented below.

### "s" - Scout

This report is sent upon startup.

* "e" - EEPROM version
* "hv" - hardware version
* "hf" - hardware family id
* "hs" - hardware serial id

### "u" - Uptime

* "m" - integer, milliseconds since boot
* "c" - integer, number of commands executed
* "r" - integer, hardware watchdog resets

### "pwr" - Power

* "p" - number from 0 to 100, battery percentage charged
* "v" - floating point number, ex 4.1, battery voltage
* "c" - bool, battery charging state
* "vcc" - bool, is vcc enabled

Power reports are automatically sent every 10% of battery change, and every 1% when below 5.  They are also sent whenever charging is enabled/disabled.

### "led" - LED status

* "l" - array of [1,2,3], the rgb status of the main led colors
* "t" - [1,2,3] rgb status of the torch colors

### "pin" - Pin Status

* "a" - array of all analog pin values, each array entry is 0-255, [0,0,0,255,0,42,0,128]
* "d" - array of all digital pin values, each array entry is 0/1 (LOW/HIGH), [0,0,0,1,0,1,0,1]

This report is sent upon any pin change (by command or by hardware) and no more often than once every 10 seconds.

### "bps" - Backpacks List

A list of all attached backpacks is sent as one report

* "a" - array of all attached backpacks, ["wifi","env"]

This report is sent on startup, and whenever a backpack is added/removed.

### "bp" - Backpack Details

Each backpack attached generates it's own report (two backpacks on one scout would be two reports).

* "b" - name of backpack, "wifi" or "env", etc
* "v" - version of backpack, float

There may be more values based on each backpack (power usage, etc). These reports are sent on startup and whenever a backpack is added/removed.

### "rf" - RF Mesh

* "id" - device id, integer
* "p" - pan id, integer
* "c" - channel id, integer
* "tx" - transmit power, string

### "rr" - RF Routing

* "f" - fixed
* "m" - multicast
* "s" - score
* "d" - dest address
* "h" - next hop address
* "r" - rank
* "l" - link quality indicator

