#include "processor.h"

#include "linux_parser.h"

// TODO: Return the aggregate CPU utilization
float Processor::Utilization() {
  long active_jiffies = LinuxParser::ActiveJiffies();
  long idle_jiffies = LinuxParser::IdleJiffies();
  long prev_total_jiffies = this->prev_active_jiffies_ + this->prev_idle_jiffies_;

  float utilization = (float)(active_jiffies - prev_active_jiffies_) /
                      (idle_jiffies + active_jiffies - prev_total_jiffies);
  
  this->prev_active_jiffies_ = active_jiffies;
  this->prev_idle_jiffies_ = idle_jiffies;
  
  return utilization;
}