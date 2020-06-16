#ifndef PROCESSOR_H
#define PROCESSOR_H
#include <vector>
#include <string>

class Processor {
 public:
  float Utilization();

 private:
 std::vector<std::string> cpu_{};
 float idle_{0.0};
 float nonidle_{0.0};

};

#endif