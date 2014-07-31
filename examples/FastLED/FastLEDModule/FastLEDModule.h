#ifndef FAST_LED_MODULE_H_
#define FAST_LED_MODULE_H_

class FastLEDModule : public PinoccioModule {

  public:
    void setup();
    void loop();
    void setHue(int);

};

#endif