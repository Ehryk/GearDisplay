/*
Analog Comparator
Ehryk Menze
*/

#include <Arduino.h>

#ifndef Comparator_h
#define Comparator_h

class Comparator
{
  public:
    Comparator();
    Comparator(int inputCount);
	Comparator(int inputCount, int method);
  private:
    int analogPins[16];
	int baseline;
	int tolerance;
};

#endif