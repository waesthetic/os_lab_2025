#include "modulo.h"

static uint64_t AddModulo(uint64_t a, uint64_t b, uint64_t mod) {
  if (a >= mod - b) {
    return a - (mod - b);
  }

  return a + b;
}

uint64_t MultModulo(uint64_t a, uint64_t b, uint64_t mod) {
  uint64_t result = 0;
  a = a % mod;

  while (b > 0) {
    if (b % 2 == 1) {
      result = AddModulo(result, a, mod);
    }

    a = AddModulo(a, a, mod);
    b /= 2;
  }

  return result;
}