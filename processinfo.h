#ifndef PROCESSINFO_H
#define PROCESSINFO_H


#define NANO 1000000000
#define MILLI 1000000


#define Quantum 10
#define NANO 1000000000
#define MILLI 1000000
#define SemName "ponagand"
/* Function declarations */

void sigintHandler(int sig_num);
int setPriority();
int createChildProc();
int getPCBByPid(int pid);
void clearPCB(int location);
void incrementClock();
/************************/

/*Structures to define Process Control Block and Logical Clock */

typedef struct PCB 
{
	float CPU_time;
	float total_time;
	long int burst_time;
	int pid;
	int priority;
	int status;
	
}PCB;

typedef struct Clock 
{
	int	sec;
	long int nano;
	int pid;
	int quantum;
	int count;
	
} Clock;

int setPriority()
{
	if(rand()%100 > 85 )
		return 0;			//High Priority
	else if(rand()%100 >65)
		return 1;		    //Low Priority
	else if(rand()%100 > 55)
		return 2;
	else return 1; 	
}

#endif