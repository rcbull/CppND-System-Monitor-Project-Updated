#include <unistd.h>
#include <cstddef>
#include <set>
#include <string>
#include <vector>

#include "linux_parser.h"
#include "process.h"
#include "processor.h"
#include "system.h"

using std::set;
using std::size_t;
using std::string;
using std::vector;

Processor& System::Cpu() { return cpu_; }

vector<Process>& System::Processes() {
  vector<int> pids{LinuxParser::Pids()};
  set<int> PIDs;
  for (Process& process : processes_) {
    PIDs.insert(process.Pid());
  }

  // Emplace all new processes
  for (int pid : pids) {
    if (PIDs.find(pid) == PIDs.end()) processes_.emplace_back(pid);
  }
  std::sort(processes_.begin(), processes_.end());
  return processes_;
}

// Return the system's kernel identifier (string)
std::string System::Kernel() const { return LinuxParser::Kernel(); }

// Return the system's memory utilization
float System::MemoryUtilization() const {
  return LinuxParser::MemoryUtilization();
}

// Return the operating system name
std::string System::OperatingSystem() const {
  return LinuxParser::OperatingSystem();
}

// Return the number of processes actively running on the system
int System::RunningProcesses() const { return LinuxParser::RunningProcesses(); }

// Return the total number of processes on the system
int System::TotalProcesses() const { return LinuxParser::TotalProcesses(); }

// Return the number of seconds since the system started running
long int System::UpTime() const { return LinuxParser::UpTime(); }
