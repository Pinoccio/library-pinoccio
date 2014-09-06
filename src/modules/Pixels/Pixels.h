#ifndef PIXELS_MODULE_H_
#define PIXELS_MODULE_H_

#include "../Module.h"

namespace pinoccio {
  class PixelsModule : public Module {

    public:
      bool enable();
      const __FlashStringHelper *name() const;
      void loop();
      static PixelsModule instance;

    private:
      using Module::Module;
  };
}

#endif