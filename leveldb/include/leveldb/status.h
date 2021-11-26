#ifndef STATUS_H_
#define STATUS_H_

#include "slice.h"

class Status {
private:
  // OK status has a NULL state. Otherwise, state is a new[] array of the following from:
  // state_[0..3] = length of message
  // state_[4] = code
  // state_[5..] = message
  const char* state_;

  enum Code {

  };
};

#endif // STATUS_H_
