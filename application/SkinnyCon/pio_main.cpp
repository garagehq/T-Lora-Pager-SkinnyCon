/* PlatformIO build entry — .ino files aren't auto-processed outside src_dir.
 *
 * Guarded with PLATFORMIO so Arduino IDE (which compiles all .cpp in the
 * sketch directory) sees an empty translation unit and doesn't produce
 * duplicate setup()/loop() definitions.
 *
 * PlatformIO defines PLATFORMIO automatically. Arduino IDE does not. */
#ifdef PLATFORMIO
#include "Arduino.h"
#include "SkinnyCon.ino"
#endif
