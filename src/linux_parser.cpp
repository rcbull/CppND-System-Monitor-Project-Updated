#include <dirent.h>
#include <unistd.h>
#include <fstream>
#include <string>
#include <vector>

// #include <experimental/filesystem>
// namespace fs = std::experimental::filesystem;

#include "linux_parser.h"

using std::getline;
using std::ifstream;
using std::istringstream;
using std::stof;
using std::string;
using std::to_string;
using std::vector;

// An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  string key_name = "PRETTY_NAME";
  ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == key_name) {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    getline(stream, line);
    istringstream linestream(line);
    linestream >> os >> version >> kernel;  // fix parse to show kernel version
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // validate if is this a directory
    if (file->d_type == DT_DIR) {
      // is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);  // convert string to ing
        pids.push_back(pid);       // add in vector
      }
    }
  }
  closedir(directory);
  return pids;
}

float LinuxParser::MemoryUtilization() {
  string line, key, value;
  string key_mem_total = "MemTotal:";
  string key_mem_free = "MemFree:";
  float total_mem;
  float free_mem;
  ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      istringstream linestream(line);
      linestream >> key >> value;
      if (key == key_mem_total) {
        total_mem = stol(value);
      } else if (key == key_mem_free) {
        free_mem = stol(value);
      }

      if (total_mem > 0 && free_mem > 0) {
        return ((total_mem - free_mem) / total_mem) * 100;
      }
    }
  }
  return 0.0;
}

// Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { return UpTime() * sysconf(_SC_CLK_TCK); }

// Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) {
  long jiffies{0};
  vector<string> values;
  ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);
  if (filestream.is_open()) {
    string line, str;
    getline(filestream, line);
    istringstream linestream(line);
    while (linestream >> str) {
      values.push_back(str);
    }
    if (values.size() > 21) {
      jiffies = std::stol(values[13]) + std::stol(values[14]) +
                std::stol(values[15]) + std::stol(values[16]);
    }
  }
  return jiffies;
}

// Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  long activeJiffies{0};
  vector<string> cpu_utililization = LinuxParser::CpuUtilization();
  for (int i = kUser_; i <= kGuestNice_; i++) {
    activeJiffies += std::stol(cpu_utililization[i]);
  }
  activeJiffies -= std::stol(cpu_utililization[kIdle_]) +
                   std::stol(cpu_utililization[kIOwait_]);
  return activeJiffies;
}

// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  vector<string> cpu_utililization = LinuxParser::CpuUtilization();
  return std::stol(cpu_utililization[kIdle_]) +
         std::stol(cpu_utililization[kIOwait_]);
}

// Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  vector<string> values;
  string line, value;
  ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      istringstream linestream(line);
      while (linestream >> value) {
        if (value == "cpu") {
          while (linestream >> value) {
            values.push_back(value);
          }
          return values;
        }
      }
    }
  }
  return values;
}

int LinuxParser::TotalProcesses() {
  string line, key, value;
  string key_name = "processes";
  ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (getline(stream, line)) {
      istringstream stream(line);
      while (stream >> key >> value) {
        if (key == key_name) {
          return stoi(value);
        }
      }
    }
  }
  return 0;
}

int LinuxParser::RunningProcesses() {
  string line, key, value;
  string key_name = "procs_running";
  ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (getline(stream, line)) {
      istringstream stream(line);
      while (stream >> key >> value) {
        if (key == key_name) {
          return stoi(value);
        }
      }
    }
  }
  return 0;
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  ifstream filestream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  if (filestream.is_open()) {
    string line;
    getline(filestream, line);
    int length = line.length();
    if (length > 60) {
      length = 60;
    }
    return line.substr(
        0, length);  // handles command name to display without line break
  }
  return string();
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
  string line, key;
  string key_name = "VmSize:";
  ifstream stream(LinuxParser::kProcDirectory + to_string(pid) +
                  LinuxParser::kStatusFilename);
  if (stream.is_open()) {
    while (stream >> key) {
      if (key == "VmSize:") {
        if (stream >> key) return to_string(stoi(key) / 1024);
      }
    }
  }
  return string("0");
}

string LinuxParser::Uid(int pid) {
  string line, key;
  string key_name = "Uid:";
  ifstream stream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (stream.is_open()) {
    while (stream >> key) {
      if (key == key_name) {
        if (stream >> key) {
          return key;
        }
      }
    }
  }
  return string("");
}

// Read and return the user associated with a process
string LinuxParser::User(int pid) {
  ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    string line;
    while (getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      istringstream linestream(line);
      string value, name, x;
      while (linestream >> name >> x >> value) {
        if (value == Uid(pid)) {
          return name;
        }
      }
    }
  }
  return "0";
}

long LinuxParser::UpTime() {
  string line;
  long uptime;
  ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    getline(stream, line);
    istringstream stream(line);
    if (stream >> uptime) {
      return uptime;
    }
  }
  return 0;
}

long LinuxParser::UpTime(int pid) {
  ifstream filestream(kProcDirectory + to_string(pid) +
                      kStatFilename);  // see cat /proc/stat and count lines
  if (filestream.is_open()) {
    string line, value;
    while (getline(filestream, line)) {
      int arr_counter = 0;
      istringstream linestream(line);
      while (linestream >> value) {
        if (arr_counter == 13) {  // 13 = uptime
          long uptime = stol(value) / sysconf(_SC_CLK_TCK);
          return uptime;
        }
        arr_counter++;
      }
    }
  }
  return 0;
}
