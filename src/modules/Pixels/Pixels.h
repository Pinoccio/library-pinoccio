#ifndef PIXELS_MODULE_H_
#define PIXELS_MODULE_H_

#include "../Module.h"

namespace pinoccio {
  class PixelsModule : public Module {
    public:
      bool load();
      void loop();
      const __FlashStringHelper *name() const;

    // Ensure there is always exactly one instance by declaring it here
    // and making our constructor private
    private:
      using Module::Module;
    public:
      static PixelsModule instance;
  };
} // namespace pinoccio

#endif
