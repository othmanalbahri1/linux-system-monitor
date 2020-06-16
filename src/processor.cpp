#include "processor.h"
#include "linux_parser.h"
#include <vector>
#include <string>

using std::vector;
using std::string;
using std::stof;

/*
  Aggregate CPU utilization
  idle = kIdle_ + kIOwait_
  nonidle = kUser_ + kNice_ + kSystem_ + kIRQ_ + kSoftIRQ_ + kSteal_
  Utilization = nonidle / (nonidle + idle)
*/
float Processor::Utilization() {
    cpu_ = LinuxParser::CpuUtilization();

    idle_ = stof(cpu_[LinuxParser::kIdle_]) + stof(cpu_[LinuxParser::kIOwait_]);
    
    nonidle_ = stof(cpu_[LinuxParser::kUser_]) + stof(cpu_[LinuxParser::kNice_]) +
      stof(cpu_[LinuxParser::kSystem_]) + stof(cpu_[LinuxParser::kIRQ_]) +
      stof(cpu_[LinuxParser::kSoftIRQ_]) + stof(cpu_[LinuxParser::kSteal_]);

    return nonidle_ / (nonidle_ + idle_);
}