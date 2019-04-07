#include<stdio.h>
#include<stdlib.h>
#include<sys/ipc.h>
#include<unistd.h>
#include<sys/ipc.h>
#include<semaphore.h>
#include<sys/shm.h>
#include<signal.h>
#include<string.h>

#include "processqueue.h"
#include "processinfo.h"
#define NANO 1000000000
#define MILLI 1000000

PCB *pcb;
Clock *clk;
int r;
int seed = 24;
int main(int *argc, char *argv[])
{
    printf("\nI am child Process %d", getpid());
    int pcbid = atoi(argv[1]);
    printf("\npcbid from child %d",pcbid);
    printf("\n attached PCB");
    int clockid = atoi(argv[2]);
    printf("\n Clk id from child %d",clockid);
    printf("\nAttached Clock");
    //char *semname = argv[3];
    int startsec=0,endsec=0,diffsec=0,choice,pid;
    long int startn=0,endn=0,diffn=0;
     
    sem_t *sem = sem_open("ponagand",0);

    pcb = shmat(pcbid,NULL,0);
    clk = shmat(clockid,NULL,0);
    clk -> count +=1;
    pid = getpid();
    srand(pid);
    startsec = clk->sec;
    startn = clk->nsec;
    int r,s,proc=-1,p;
    float wait,_time,quantum;
    //printf("\n%d",getPcbByPid());

    while((1))
    {
        if(pid == clk->pid)
        {
            r=0,s=0,wait = 0,_time = 0.0, quantum = 0.0;
            choice = genRandom();
            switch(choice)
            {
                case 0:
                    break;
                case 1:
                    wait = (float)(clk->quantum) /MILLI;
                    break;
                case 2:
                    r = rand()%5;
                    s = rand()%1000;
                    _time = (float)s / 1000;
                    wait = (float)r + _time;
                    break;
                case 3:
                    p = rand()%(100-1);
                    quantum = (float)(clk->quantum)/MILLI;
                    wait = (float)(p*quantum)/100;
                    break;
                default:
                fprintf(stderr,"Error in User \n");
						exit(0);
            }

            printf("\n Program crossed Switches");

            if((proc = getPcbByPid(pid)) >= 0)
            {
                printf("\nIn getPcbByPid from CHild");
                pcb[proc].last_burst = wait * NANO;
                pcb[proc].CPU_time += wait;
                printf("\nUpdated burst and CPU times");
                if(processStatus(proc) ==1 )
                {
                    printf("\n Program status is 1");
                    break;
                }
            }
            else
            {
                fprintf(stderr,"\nProc = %d",proc);
                fprintf(stderr,"\nError occurred in child process %d\n",getpid());
                exit(0);
            }
            clk->pid = -1;
            sem_post(sem); 
        
        }
    }
        endsec =  clk->sec;
        endn = clk->nsec;
        diffsec = endsec - startsec;
        diffn = endn - startn;
        _time = (float) diffn / NANO;
        printf("\n_time updated");
        //p = getPcbByPid(pid);
        pcb[getPcbByPid(pid)].total_time = (float) diffsec+_time+wait;
        printf("\nUpdated PCB Total Time\n");
        pcb[proc].status = 1;
        printf("\n Reached almost to the end of child\n");
        sem_post(sem);
        printf("\n sem_post done");
        fprintf(stderr,"\nSeconds : %d Nanoseconds : %ld\n",clk->sec,clk->nsec);

    shmdt(pcb);
    printf("\nShared Memory detached in child");
    shmdt(clk);
    printf("\nClock detached in child");

    return 0;

}

int genRandom()
{
    r = rand()%100;
    if(r >=75)
        return 0;
    if(r >=55)
        return 1;
    if(r >= 40)
        return 2;
    else return 3;
}

int processStatus(int proc)
{
	if(pcb[proc].CPU_time > 0.05)
	{
		srand(seed++);
		if(rand()%100 > 30)
			return 1;
		else
			return 0;
	}
}
int getPcbByPid(int pid)
{
	int i;
	for(i  = 0; i < 18; i++ )
		if(pcb[i].pid == pid)			
			return i;
	return -1;
}