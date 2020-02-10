#define RANDOMSCHED 1 
#define LINUXSCHED 2

unsigned int schedflag;
void setschedclass(int sched_class);

int getschedclass();
