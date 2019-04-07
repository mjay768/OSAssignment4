
#ifndef PROCESSINFO_H
#define PROCESSINFO_H


#define NANO 1000000000
#define MILLI 1000000
//Function Declarations

void sigintHandler(int sig_num);
int createChildProc();
void advanceClock();
void clearPCB(int loc);
void initclock();
void writedebug(char *errortext);
int getPcbByPid(int pid);
void time_increment();
int genRandom();
int isCompleted(int loc);

//----------------------//

typedef struct PCB 
{
	float CPU_time;
	float total_time;
	long int last_burst;
	int pid;
	int priority;
	int status;
	int pclass;
	int pqueue;
	
}PCB;

typedef struct Clock 
{
	int	sec;
	long int nsec;
	int pid;
	int quantum;
	int count;
	
} Clock;

int getPriority()
{
	if(rand()%100 > 85 )
		return 1;			//High Priority
	else 
		return 0;		    //Low Priority	
}




#endif