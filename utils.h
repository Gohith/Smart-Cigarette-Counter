#pragma once
#include <stdlib.h>

#define min2ms(X) (X)*60*1000

double fRand(double fMin, double fMax)
{
    double f = (double)rand() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}

int iRand(int min, int max)
{
  return (rand() % (max - min + 1)) + min;
}

double fluctuate(double value, double amount)
{
  return value * fRand(1 - amount, 1 + amount);
}

void toPrettyTime(unsigned long input_ms, int& ms, int& s, int& m, int& h)
{
  ms = input_ms;

  s = ms / 1000;
  ms %= 1000;

  m = s / 60;
  s %= 60;

  h = m / 60;
  m %= 60;
}
