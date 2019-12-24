#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <vector>

class Processor {
 public:
  float Utilization();

 private:
  long active_{0};
  long idle_{0};
};

#endif
