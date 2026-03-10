/* PlatformIO build entry — .ino files aren't auto-processed outside src_dir.
 * Guard with PLATFORMIO so Arduino IDE doesn't compile this alongside factory.ino,
 * which would cause duplicate setup()/loop() definitions. */
#ifdef PLATFORMIO
#include "Arduino.h"
#include "factory.ino"
#endif
