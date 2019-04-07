#include <stdio.h> 
#include <stdlib.h>
#include <signal.h> 
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h> 
#include <fcntl.h>
#include <sys/ipc.h> 
#include <sys/shm.h>
#include "processqueue.h"
#include "processinfo.h"
#define QUANTUM 10
#define semname "mysemaphore"

#define key1 0x98989898
#define key2 0x10101010

// Queue levels declaration
#define RT 0
#define UP 1
//***********************//

//Global variables 

FILE *fptr;
sem_t *sem;
int pcbid;
int clockid;
PCB *pcb;
Clock *clk;
int linecount = 0;

int TTG;
 char debugtext[128];
FILE *debugfile;

int main(int argc, char *argv[])
{
    char arg1[20];
	char arg2[20];
	char arg3[20];
	char opt;
	char *logfile = "log";
	char *dfile = "debugfile.txt";
	int i,status,pid,location = -1,proc;
	signal(SIGALRM, sigintHandler);
    signal(SIGINT, sigintHandler);

	// Queues
	struct Queue rr;
	struct Queue hq;
	struct Queue mq;
	struct Queue lq; 

    debugfile = fopen(dfile,"w");
	if(dfile == NULL)
	{
		perror("File open error");
		fprintf(stderr,"File open error");
	}
	else printf("\n Debug file opened");
	//key_t key1 = ftok("MJ768",'m'),key2 = ftok("MJClk",'s');
	sem = sem_open("ponagand",O_CREAT, 0666, 0);
	if(sem = SEM_FAILED)
	{
		perror("Semaphore open error");
		fprintf(stderr,"Semaphore Failed to open");
		writedebug("Semaphore Failed to open");
	}
	else printf("\nSemaphore address %d",sem);		

    int startsec = 0;
    int endsec = 0;
    int diffsec = 0;
	int t=2; // Initializing default burst time

	long int snano = 0, enano = 0, diffnano = 0, diff_time;
	
	
    while((opt= getopt(argc,argv, "hlt")) != -1)
    {
        switch(opt)
        {
            case 'h' : printf("\nUse -h for help");
                       printf("\n Syntax: -l [log file name] -t [time in seconds]");break;
            case 'l' : logfile = optarg; printf("\n%s\n",logfile); break;
			case 't' : t = atoi(optarg);
            case '?' : printf("\n Invalid arguments\n");
                       printf("\nPlease use -h for help");
                       printf("\n Syntax: -l [log file name] -t [burst time in seconds]"); break;
			default : abort();
        }
    }

	fptr = fopen(logfile,"w");
	if(fptr == NULL)
	{
		perror("File open error");
		fprintf(stderr,"File open error");
	}
	else printf("\nLog File opened ");
	fprintf(stderr,"Log File: %s\n",logfile);

	int shmid = shmget(key1, sizeof(PCB) * 18, IPC_CREAT | 0666);
	if(shmid == -1)
	{
		perror("Shared Memory open error");
		fprintf(stderr,"OSS : Shared Memory Open Error");
		writedebug("Shared Memory Open Error");
	}
	else printf("\nShared Memory Opened");

	pcb = (PCB *)shmat(shmid,0,0);

	clockid = shmget(key2,sizeof(clk), IPC_CREAT | 0666);
	if(clockid == -1)
	{
		perror("Clock creation error");
		fprintf(stderr,"OSS : Clock creation error");
		writedebug("Clock creation error");
	}
	else printf("\nClock opened");

	clk =  (Clock *)shmat(clockid, 0, 0);
	printf("\nClock attached to clk");
	snprintf(arg1,10,"%d", shmid);
	snprintf(arg2,10,"%d",clockid);
	snprintf(arg3,20,"%s", "ponagand");
	printf("\n PCB id from parent %d",shmid);
	printf("\n Clk id from parent %d",clockid);
	clk -> sec = 0;
	clk -> nsec = 0;
	clk -> pid = -1;
	clk -> quantum = 0;
	clk -> count = 0;

	alarm(t);
	for(i = 0; i<18 ; i++)
	{
		clearPCB(i); // Using this function just to make it easy initializing the PCB.
	}
	printf("\nPCB initialized\n");
	writedebug("\nPCB initialized\n");
	//fprintf(debugfile,"\nPCB Initialized");

	init(&rr);
	init(&hq);
	init(&mq);
	init(&lq);
	//alarm(5);
	srand(time(NULL));
	writedebug("Queues Initialized");
	printf("\nQueues Initialized\n");

	/* Test code 
	
	pcb[0].priority = getPriority();
	printf("\n Priority is %d", pcb[0].priority);
	pcb[1].priority = getPriority();
	printf("\n Priority is %d", pcb[1].priority);
	pcb[2].priority = getPriority();
	printf("\n Priority is %d", pcb[2].priority);
	pcb[3].priority = getPriority();
	printf("\n Priority is %d", pcb[3].priority); 
	
	*/
	while(1)
	{
		if(location = createChildProc() >= 0 && clk -> sec >= TTG)
		{
			if( (pid = fork()) == 0)
			{
				execlp("./user","./user",arg1,arg2,NULL);
				fprintf(stderr,"\nFork error occurred\n");
				exit(0);
			}


			TTG += rand()%3;
			pcb[location].priority = getPriority();
			if(pcb[location].priority == 1)
				pcb[location].pclass == RT;
			else if(pcb[location].priority == 0)
				pcb[location].pclass == UP;
			
			printf("\n Priority is %d\n", pcb[location].priority);
			pcb[location].pid = pid;
			switch (pcb[location].priority)
			{
				case 0:
					if(linecount < 10000)
					{
						fprintf(fptr,"OSS : Generating process with PID %d (Round-Robin High Priority) and putting it in queue 0 at time %d:%ld\n",pid,clk -> sec,clk ->nsec);
						printf("OSS : Generating process with PID %d (Round-Robin High Priority) and putting it in queue 0 at time %d:%ld\n",pid,clk -> sec,clk ->nsec);
						linecount++;
						push(&rr, pid);
					}
					break;
				case 1:
					if(linecount < 10000)
					{
						fprintf(fptr,"OSS : Generating process with PID %d (High Priority) and putting it in queue 1 at time %d:%ld\n",pid,clk -> sec,clk ->nsec);
						printf("OSS : Generating process with PID %d (High Priority) and putting it in queue 1 at time %d:%ld\n",pid,clk -> sec,clk ->nsec);
						linecount++;
						push(&hq, pid);
					}
					break;
				default:
					break;
				
			}
			location = -1;

		}
		clk -> nsec += rand()%1000;
		time_increment();

		if((proc = pop(&rr)) > 0)
		{
			if(linecount < 10000)
			{
				fprintf(fptr, "OSS: Dispatching PID %d at %d:%ld\n",pid,clk->sec,clk->nsec);
				linecount++;
			}
			// Calculating dispatch time.

			clk -> pid = pid;
			clk -> quantum = QUANTUM/2;
			enano = clk ->nsec;
			endsec = clk ->sec;
			diffsec = endsec - startsec;
			diffnano = enano - snano;
			diff_time = diffsec * NANO + diffnano;

			/*************************/

			if(linecount < 10000)
			{
				fprintf(fptr,"OSS: Total time this dispatch was %ld\n",diff_time);
				linecount++;
			}
			sem_wait(sem);
			location = getPcbByPid(proc);
			if(pcb[location].status == 1)
			{
				if(linecount < 10000)
				{
					fprintf(fptr,"OSS: Receiving that process with PID %d ran for %ld\n",pid,pcb[location].last_burst);
					linecount++;
				}
				waitpid(proc, &status, 0);
				if(linecount < 10000)
				{
					fprintf(fptr, "OSS: Process PID %d has finished executing and took time of %f\n",pid,pcb[location].total_time);
					linecount++;
				}
				clearPCB(location);
			}
			else
			{
				if(linecount < 10000)
				{
					fprintf(fptr,"OSS: Receiving that process with PID %d ran for %ld\n",pid,pcb[location].last_burst);
					linecount++;
				}
				if(pcb[location].last_burst <= 5000000)
				{
					fprintf(fptr,"OSS: Not used its entire time quantum\n");
					fprintf(fptr,"OSS: Putting process with PID %d into queue 0\n",proc);
					linecount+=2;
				}
				
				push(&rr,proc);
			}
				clk -> nsec += pcb[location].last_burst;
				time_increment();

				startsec = clk -> sec;
				snano = clk -> nsec;
				location = -1;

			

		}
		else if((proc = pop(&hq)) > 0)
		{
			if(linecount < 10000)
			{
				fprintf(fptr, "OSS: Dispatching PID %d at %d:%ld\n",pid,clk->sec,clk->nsec);
				linecount++;
			}
			// Calculating dispatch time.

			clk -> pid = pid;
			clk -> quantum = QUANTUM/2;
			enano = clk ->nsec;
			endsec = clk ->sec;
			diffsec = endsec - startsec;
			diffnano = enano - snano;
			diff_time = diffsec * NANO + diffnano;

			/*************************/

			if(linecount < 10000)
			{
				fprintf(fptr,"OSS: Total time this dispatch was %ld\n",diff_time);
				linecount++;
			}
			sem_wait(sem);
			location = getPcbByPid(proc);
			if(pcb[location].status == 1)
			{
				if(linecount < 10000)
				{
					fprintf(fptr,"OSS: Receiving that process with PID %d ran for %ld\n",pid,pcb[location].last_burst);
					linecount++;
				}
				waitpid(proc, &status, NULL);
				if(linecount < 10000)
				{
					fprintf(fptr, "OSS: Process PID %d has finished executing and took time of %f\n",pid,pcb[location].total_time);
					linecount++;
				}
				//push(&mq,proc);
				clearPCB(location);
			}
			else
			{
				if(linecount < 10000)
				{
					fprintf(fptr, "OSS: Not using its entire quantum");
					fprintf(fptr,"OSS: Putting process with PID %d into Queue 0",pid);
					linecount+=2;
				}
				push(&hq,proc);
			}
			clk -> nsec += pcb[location].last_burst;
			time_increment();

			startsec = clk -> sec;
			snano = clk -> nsec;
			location = -1;

			

		}
		else if((proc = pop(&mq)) > 0)
		{
			if(linecount < 10000)
			{
				fprintf(fptr, "OSS: Dispatching PID %d at %d:%ld\n",pid,clk->sec,clk->nsec);
				linecount++;
			}
			// Calculating dispatch time.

			clk -> pid = pid;
			clk -> quantum = QUANTUM/2;
			enano = clk ->nsec;
			endsec = clk ->sec;
			diffsec = endsec - startsec;
			diffnano = enano - snano;
			diff_time = diffsec * NANO + diffnano;

			

			if(linecount < 10000)
			{
				fprintf(fptr,"OSS: Total time this dispatch was %ld\n",diff_time);
				linecount++;
			}
			sem_wait(sem);
			location = getPcbByPid(proc);
			if(pcb[location].status == 1)
			{
				if(linecount < 10000)
				{
					fprintf(fptr,"OSS: Receiving that process with PID %d ran for %ld\n",pid,pcb[location].last_burst);
					linecount++;
				}
				waitpid(1, &status, NULL);
				if(linecount < 10000)
				{
					fprintf(fptr, "OSS: Process PID %d has finished executing and took time of %f\n",pid,pcb[location].total_time);
					linecount++;
				}
				//push(&lq,proc);
				clearPCB(location);
			}
			else
			{
				if(linecount < 10000)
				{
					fprintf(fptr, "OSS: Not using its entire quantum");
					fprintf(fptr,"OSS: Putting process with PID %d into Queue 0",pid);
					linecount+=2;
				}
				push(&hq,proc);
				
			}
			clk -> nsec += pcb[location].last_burst;
			time_increment();

			startsec = clk -> sec;
			snano = clk -> nsec;
			location = -1;

		}
		else if((proc = pop(&lq)) > 0)
		{
			if(linecount < 10000)
			{
				fprintf(fptr, "OSS: Dispatching PID %d at %d:%ld\n",pid,clk->sec,clk->nsec);
				linecount++;
			}
			// Calculating dispatch time.

			clk -> pid = pid;
			clk -> quantum = QUANTUM/2;
			enano = clk ->nsec;
			endsec = clk ->sec;
			diffsec = endsec - startsec;
			diffnano = enano - snano;
			diff_time = diffsec * NANO + diffnano;

			

			if(linecount < 10000)
			{
				fprintf(fptr,"OSS: Total time this dispatch was %ld\n",diff_time);
				linecount++;
			}
			sem_wait(sem);
			location = getPcbByPid(proc);
			if(pcb[location].status == 1)
			{
				if(linecount < 10000)
				{
					fprintf(fptr,"OSS: Receiving that process with PID %d ran for %ld\n",pid,pcb[location].last_burst);
					linecount++;
				}
				waitpid(1, &status, NULL);
				if(linecount < 10000)
				{
					fprintf(fptr, "OSS: Process PID %d has finished executing and took time of %f\n",pid,pcb[location].total_time);
					linecount++;
				}
				//push(&lq,proc);
				clearPCB(location);
			}
			else
			{
				if(linecount < 10000)
				{
					fprintf(fptr, "OSS: Not using its entire quantum");
					fprintf(fptr,"OSS: Putting process with PID %d into Queue 0",pid);
					linecount+=2;
				}
				push(&mq,proc);
				
			}
			clk -> nsec += pcb[location].last_burst;
			time_increment();

			startsec = clk -> sec;
			snano = clk -> nsec;
			location = -1;

		} 
		else
		{
			clk ->sec += 1;
			time_increment();
		}
		

	}

return 0;
}


void sigintHandler(int sig_num) 
{ 

    if(sig_num == 2)
    	fprintf(stderr,"Program interrupted while running\n");
    else
	fprintf(stderr,"Program Time Limit Exceeded\n");
    int i;
	fprintf(stderr,"Clock status : sec = %d, nano %ld\n", clk -> sec, clk -> nsec);
	fprintf(stderr,"Total Children Forked : %d\n",clk -> count);
	
   for(i = 0; i < 18; i++)
   {
       if(pcb[i].pid > 0 )
       {

	       kill(pcb[i].pid, SIGTERM);
       }
    }
	fprintf(stderr,"Remaining Children : Removed\n");
    
    fprintf(stderr,"\nTTG : %d",TTG);
  
	shmdt(pcb);
	shmdt(clk);
	fprintf(stderr,"Shared Memory : Detached\n");
	fclose(fptr);
    sem_unlink("ponagand");
	fprintf(stderr,"Semaphore : Unlinked\n");
    shmctl(pcbid,IPC_RMID,NULL);
	shmctl(clockid,IPC_RMID,NULL);
	fprintf(stderr,"Shared Memory : Cleared\n");
    abort(); 
    fflush(stdout); 
} 

int createChildProc()
{
	int i;
	for(i  = 0; i < 18; i++ )
		if(pcb[i].pid == -1)
			return i;
	return -1;
}

void advanceClock()
{
	clk -> sec += 1;
	if(clk -> nsec > NANO)
		{
			clk -> sec += clk -> nsec/NANO;
			clk -> nsec %= NANO;
		}	
}

void clearPCB(int location)
{
		pcb[location].CPU_time = 0.0;
		pcb[location].total_time = 0.0;
		pcb[location].last_burst = 0;
		pcb[location].pid = -1;
		pcb[location].priority = -1;
		pcb[location].status = 0;
		pcb[location].pclass = -1;
		pcb[location].pqueue = -1;
}

void initclock()
{
	clk -> sec = 0;
	clk -> nsec = 0;
	clk -> pid = -1;
	clk -> quantum = 0;
	clk -> count = 0;
	writedebug("\nClock initiated to 0s");
}

void writedebug(char errortext[128])
{
	//snprintf(debugtext, 100,"%s",errortext);
	fprintf(debugfile,"%s",errortext);
}

int setClass()
{
	if(rand()%100 > 75 )
		return 1;			//Real Time
	else 
		return 0;		    //Normal Process	
}

int setPriority()
{
	if(rand()%100 > 85 )
		return RT;			// Medium Priority
	else 
		return UP;		    //Low Priority
		
}
int getPcbByPid(int pid)
{
	int i;
	for(i  = 0; i < 18; i++ )
		if(pcb[i].pid == pid)			
			return i;
	return -1;
}

void time_increment()
{
	if(clk -> nsec > NANO)
	{
		clk -> sec += clk -> nsec/NANO;
		clk -> nsec %= NANO;
	}
}