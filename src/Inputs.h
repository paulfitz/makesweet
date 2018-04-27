#ifndef INPUTS_INC
#define INPUTS_INC

#include "Input.h"

#include <vector>

class Inputs {
private:
  std::vector<Input> data;
public:

  Input& add() {
    data.push_back(Input());
    return data.back();
  }

  const std::vector<Input>& get() const {
    return data;
  }
  
  std::vector<Input>& get_mod() {
    return data;
  }
};

#endif
