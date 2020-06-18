#include "system.h"

#include <unistd.h>

#include <cstddef>
#include <set>
#include <string>
#include <vector>

#include "format.h"
#include "linux_parser.h"
#include "process.h"
#include "processor.h"

using std::set;
using std::size_t;
using std::string;
using std::vector;

// System's CPU
Processor& System::Cpu() { return cpu_; }

// System's processes as a vector
vector<Process>& System::Processes() {
  const vector<int>& pids = LinuxParser::Pids();
  for (int i : pids) {
    Process process(i);
    processes_.push_back(process);
  }
  return processes_;
}

// System's kernel identifier
std::string System::Kernel() const { return LinuxParser::Kernel(); }

// System's memory utilization
float System::MemoryUtilization() const {
  return LinuxParser::MemoryUtilization();
}

// Operating system name
std::string System::OperatingSystem() const {
  return LinuxParser::OperatingSystem();
}

// Number of processes actively running on the system
int System::RunningProcesses() const { return LinuxParser::RunningProcesses(); }

// Total number of processes on the system
int System::TotalProcesses() const { return LinuxParser::TotalProcesses(); }

// Number of seconds since the system started running
long int System::UpTime() const { return LinuxParser::UpTime(); }