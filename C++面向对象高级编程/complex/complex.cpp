#include "complex.h"

inline complex &complex::operator+=(const complex &r) {
  return _doapl(this, r);
}
