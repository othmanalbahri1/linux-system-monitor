#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// Operating system
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// Kernel info
string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// system memory utilization 
float LinuxParser::MemoryUtilization() {
  string line, label, MemTotal, MemFree, Buffers;
  float adj;
  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      if (line.find("MemTotal:") != string::npos) {
        std::istringstream linestream(line);
        linestream >> label >> MemTotal;
      }
      else if (line.find("MemFree:") != std::string::npos) {
        std::istringstream linestream(line);
        linestream >> label >> MemFree;
      }
      else if (line.find("Buffers:") != std::string::npos) {
        std::istringstream linestream(line);
        linestream >> label >> Buffers;
      }
    }
  }
  adj = stof(MemTotal) - stof(Buffers);
  return 1.0 - (stof(MemFree) / adj);
}

// System uptime
long LinuxParser::UpTime() {
  string line, str_uptime;
  std::ifstream filestream(kProcDirectory + kUptimeFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> str_uptime;
  }
  return stol(str_uptime);
}

// Total jiffies for the system
long LinuxParser::Jiffies() {
  return UpTime() * sysconf(_SC_CLK_TCK);
}

// Active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) {
  string line;
  long jiffies;
  std::ifstream filestream(kProcDirectory + to_string(pid));
  if (filestream.is_open()) {
    std::getline(filestream, line);
    // split line into a vector
    std::istringstream linestream(line);
    std::istream_iterator<string> start(linestream);
    std::istream_iterator<string> end;
    std::vector<string> stov(start, end);
    jiffies = stol(stov[13]) + stol(stov[14])
        + stol(stov[15]) + stol(stov[16]);
  }
  return jiffies;
}

// Active jiffies for the system
long LinuxParser::ActiveJiffies() {
  string line, num;
  long jiffies;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    line.erase (line.begin(), line.begin()+5);
    std::istringstream linestream(line);
    while (linestream >> num) {
        jiffies += std::stol(num);
    }
  }
  return jiffies;
}

// Idle jiffies for the system
long LinuxParser::IdleJiffies() {
  string line;
  long jiffies;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    line.erase (line.begin(), line.begin()+5);
    // split line into a vector
    std::istringstream linestream(line);
    std::istream_iterator<string> start(linestream);
    std::istream_iterator<string> end;
    std::vector<string> cpu(start, end);
    jiffies = std::stol(cpu[kIdle_]) + std::stol(cpu[kIOwait_]);
  }
  return jiffies;
}

// CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  string line, num;
  vector<string> cpu;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    line.erase (line.begin(), line.begin()+5);
    // split line into a vector
    std::istringstream linestream(line);
    while (linestream >> num) {
      cpu.push_back(num);
    }
  }
  return cpu;
}

// Total number of processes
int LinuxParser::TotalProcesses() {
  string line, key, value;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      if (line.find("processes") != string::npos) {
        std::istringstream linestream(line);
        linestream >> key >> value;
      }
    }
  }
  return stoi(value);
}

// Number of running processes
int LinuxParser::RunningProcesses() {
  string line, key, value;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      if (line.find("procs_running") != string::npos) {
        std::istringstream linestream(line);
        linestream >> key >> value;
      }
    }
  }
  return stoi(value);
}

// Process CPU utilization
float LinuxParser::CpuUtilization(int pid) {
  string line, num;
  vector<string> stat;
  float cpu, total_time, seconds;

  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    // split line into a vector
    std::istringstream linestream(line);
    while (linestream >> num) {
      stat.push_back(num);
    }
  }
  total_time = (stof(stat[13]) + stof(stat[14]) + 
      stof(stat[15]) + stof(stat[16])) / sysconf(_SC_CLK_TCK);
  seconds = UpTime() - (stof(stat[21]) / sysconf(_SC_CLK_TCK));
  cpu = 100 * (total_time/seconds);
  return cpu;
}

// Command associated with a process
string LinuxParser::Command(int pid) {
  string cmd;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  if (filestream.is_open()) { std::getline(filestream, cmd); }
  return cmd;
}

// Memory used by a process
// TODO: convert to MB
string LinuxParser::Ram(int pid) {
  string line, VmSize, label;
  std::ifstream filestream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      if (line.find("VmSize:") != string::npos) {
        std::istringstream linestream(line);
        linestream >> label >> VmSize;
      }
    }
  }
  return VmSize;
}

// User ID associated with a process
string LinuxParser::Uid(int pid) {
  string line, label, uid;
  std::ifstream filestream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      if (line.find("Uid:") != string::npos) {
        std::istringstream linestream(line);
        linestream >> label >> uid;
      }
    }
  }
  return uid;
}

// User associated with a process
string LinuxParser::User(int pid) {
  string ptrn, line, user;
  ptrn = string(":x:").append(Uid(pid));
  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      if (line.find(ptrn) != string::npos) {
        std::replace(line.begin(), line.end(), ':', ' ');
        std::istringstream linestream(line);
        linestream >> user;
      }
    }
  }
  return user;
}

// Uptime of a process
long LinuxParser::UpTime(int pid) {
  string line;
  long uptime;
  std::ifstream filestream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    // split line into a vector
    std::istringstream linestream(line);
    std::istream_iterator<string> start(linestream);
    std::istream_iterator<string> end;
    std::vector<string> stat(start, end);
    uptime = std::stol(stat[21]) / sysconf(_SC_CLK_TCK); // in seconds
  }
  return uptime;
}
