#ifndef EXAMPLE_MODULE_H_
#define EXAMPLE_MODULE_H_

class ExampleModule : public PinoccioModule {

  public:
    void setup();
    void loop();
    const char *name();
    
    // any custom methods
    uint8_t foo();

};

#endif