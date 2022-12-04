#include "linux_parser.h"

#include <bits/stdc++.h>
#include <dirent.h>
#include <unistd.h>

#include <sstream>
#include <string>
#include <vector>

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
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

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel, version;
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

// Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  string line;
  string key;
  string value;
  std::string::size_type size;
  long mem_total, mem_free, buffers, cached, reclaimable, shmem = 0;

  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        try {
          if (key == "MemTotal:") {
            mem_total = std::stol(value, &size, 10);
          } else if (key == "MemFree:") {
            mem_free = std::stol(value, &size, 10);
          } else if (key == "Buffers:") {
            buffers = std::stol(value, &size, 10);
          } else if (key == "Cached:") {
            cached = std::stol(value, &size, 10);
          } else if (key == "SReclaimable:") {
            reclaimable = std::stol(value, &size, 10);
          } else if (key == "Shmem:") {
            shmem = std::stol(value, &size, 10);
          }
        }
        // catch invalid_argument exception.
        catch (const std::invalid_argument) {
          continue;
        }
      }
    }
    filestream.close();
  }
  return (float)(mem_total - mem_free -
                 (buffers + cached + reclaimable - shmem)) /
         mem_total;
}

//  Read and return the system uptime
long LinuxParser::UpTime() {
  string uptime, line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> uptime;
  }
  stream.close();
  return std::stol(uptime);
}

// Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  long uptime = LinuxParser::UpTime();
  return uptime * sysconf(_SC_CLK_TCK);
}

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid) {
  string line, key, payload;
  long active_jiffies = 0;
  long utime, stime, cutime, cstime, to_be_skipped;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    int counter{0};
    while (counter++ < 13) {
      linestream >> to_be_skipped;
    }
    linestream >> utime >> stime >> cutime >> cstime;
    active_jiffies = (utime + stime + cutime + cstime) / sysconf(_SC_CLK_TCK);
    stream.close();
    return active_jiffies;
  }
  stream.close();
  return active_jiffies;
}

// Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  auto jiffies  = LinuxParser::CpuUtilization();
  return std::stol(jiffies[CPUStates::kUser_]) + std::stol(jiffies[CPUStates::kNice_]) +
        std::stol(jiffies[CPUStates::kSystem_]) + std::stol(jiffies[CPUStates::kIRQ_]) +
        std::stol(jiffies[CPUStates::kSoftIRQ_]) + std::stol(jiffies[CPUStates::kSteal_]);    
}

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  auto jiffies  = LinuxParser::CpuUtilization();
  return std::stol(jiffies[CPUStates::kIdle_]) + std::stol(jiffies[CPUStates::kIOwait_]);  
}

// Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  vector<string> cpu_util;
  string line, key, token;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> key;
    if (key == "cpu") {
      while (linestream >> token) {
        cpu_util.push_back(token);
      }
    }
  }
  stream.close();
  return cpu_util;
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  string line, key, payload;
  int total_processes;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> payload) {
        if (key == "processes") {
          total_processes = std::stoi(payload);
          return total_processes;
        }
      }
    }
    filestream.close();
  }
  return total_processes;
}

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  string line, key, payload;
  int running_processes{0};
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> payload) {
        if (key == "procs_running") {
          running_processes = std::stoi(payload);
          return running_processes;
        }
      }
    }
    filestream.close();
  }
  return running_processes;
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kCmdlineFilename);
  if (stream.is_open()) {
    string line;
    std::getline(stream, line);
    stream.close();
    return line;
  }
  stream.close();
  return string();
}

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid) {
  string line;
  string key;
  string value;
  std::ifstream filestream(kProcDirectory + std::to_string(pid) +
                           kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "VmSize:") {
          filestream.close();
          return value;
        }
      }
    }
  }
  filestream.close();
  return value;
}

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid) {
  string line;
  string key;
  string value;
  std::ifstream filestream(kProcDirectory + std::to_string(pid) +
                           kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "Uid:") {
          filestream.close();
          return value;
        }
      }
    }
  }
  filestream.close();
  return value;
}

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid) {
  string uid_reference = LinuxParser::Uid(pid);
  string line, user, _toSkip, uid;
  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> user >> _toSkip >> uid) {
        if (uid == uid_reference) {
          filestream.close();
          return user;
        }
      }
    }
  }
  filestream.close();
  return user;
}

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid) {
  string line;
  long uptime, to_be_skipped;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    int counter{0};
    while (counter++ < 22) {
      linestream >> to_be_skipped;
    }
    linestream >> uptime;
    stream.close();
    return uptime / sysconf(_SC_CLK_TCK);
  }
  stream.close();
  return uptime;
}