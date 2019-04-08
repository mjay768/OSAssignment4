#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <semaphore.h> 
#define NANOSEC 1000000000
#define MILLISEC 1000

#include "processinfo.h"
#include "processqueue.h"

/* typedef struct PCB 
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
	
} Clock; */
Clock *clk;
PCB *pcb;
int r,seed =24;
int getRandomNumber()
{
//	srand(seed++);
	r =rand()%100;
	if(r <= 50)
		return 1;			//50% - All quantum is used
	else if(r <= 60 )
		return 0;			//10% - Terminate the process
	else if(r <= 75)
		return 2;			//15% - Prcess to wait for I/O
	else
		return 3;			//25% - Use a percentage of defined quantum

}
int getPCBbyPID(int pid)
{
	int i;

	for(i  = 0; i < 18; i++ )
		if(pcb[i].pid == pid)
			return i;
		
	return -1;
}

int isCompleted(int loc)
{
	if(pcb[loc].CPU_time > 0.05)
	{
		srand(seed++);
		if(rand()%100 > 30)
			return 1;
		else
			return 0;
	}
}

 int main(int argc,char *argv[])
 {
	 
	 
	 int pcbid = atoi(argv[1]);
	 int clkid = atoi(argv[2]);
	 int i,pid;
	
	 char *c = argv[3];
	 sem_t *sem = sem_open(c, 0);
	 pcb = shmat(pcbid, NULL, 0);
	 clk = shmat(clkid, NULL, 0);
	 clk -> count +=1; 
	 pid = getpid(); 
	 srand(pid);
	 int ch;
	 int r=0,s=0,p = -1;
	 float wait,_time,quantum;
	 int StartSec = 0,EndSec = 0, dSec = 0;
	 long int StartNano = 0, EndNano = 0, dNano =0;
	 StartSec = clk -> sec;
	 StartNano = clk -> nano;
	while(1)
	{
		if(pid == clk -> pid)
		{
			wait = 0,r=0,_time=0.0,s=0, quantum = 0.0;
			ch = getRandomNumber();
			switch(ch)
			{
				case 0:
						break;
				case 1:
						wait = (float)(clk -> quantum) / MILLISEC;
						
						//wait(wait);
						break;
				case 2:	
						r = rand()%5;
						s = rand()%1000;
						_time = (float)s / 1000;
						wait = (float)r + _time;
						//wait(wait);
						break;
				case 3:

						r = rand()%100;
						_time = (float)r / 100;
						quantum = (float)(clk -> quantum) / MILLISEC ;
						wait = (float) quantum  * _time;
						//wait(wait);
						break;
				default:
						fprintf(stderr,"User Process Error Occurred\n");
						exit(0);
					
			}
	
			if((p = getPCBbyPID(pid)) >= 0)
			{
			
				pcb[p].burst_time = wait * NANOSEC;
				pcb[p].CPU_time += wait;
				if(isCompleted(p) == 1)
				{
					
					break;
				}
			}
			else
			{
				fprintf(stderr,"Child processed caused error- PID: %d\n",getpid());
				exit(0);
			}
			clk -> pid = -1;
			
			sem_post(sem);
			
		}
	}
			EndSec = clk -> sec;
			EndNano = clk -> nano;
			dSec = EndSec - StartSec;
			dNano = EndNano - StartNano;
			_time = (float)dNano / NANOSEC;
			pcb[getPCBbyPID(pid)].total_time = (float) dSec + _time + wait;
			pcb[p].status = 1;
			sem_post(sem);
			
	shmdt(pcb);
	shmdt(clk);
	 return 0;
 }
