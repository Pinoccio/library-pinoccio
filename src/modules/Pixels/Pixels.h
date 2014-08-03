#ifndef PIXELS_MODULE_H_
#define PIXELS_MODULE_H_

class PixelsModule : public PinoccioModule {

  public:
    void setup();
    void loop();
    const char *name();

};

#endif