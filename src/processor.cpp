#include "processor.h"
#include "linux_parser.h"

// DONE: Return the aggregate CPU utilization
float Processor::Utilization() {
  float idle = LinuxParser::IdleJiffies();
  float active = LinuxParser::ActiveJiffies();
  float utilization = {0};

  long duration_active{active - active_};
  long duration_idle{idle - idle_};
  long duration{duration_active + duration_idle};
  utilization = static_cast<float>(duration_active) / duration;

  active_ = active;
  idle_ = idle;

  return utilization * 100;
}
