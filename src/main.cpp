#include <HomeStatusDisplay.h>

static const char* IDENTIFIER = "HomeStatusDisplay";

HomeStatusDisplay display;

void setup() { 
  display.begin(IDENTIFIER);
}

void loop() {     
  display.work();
}

