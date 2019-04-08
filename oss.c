/************************************************/
/* Title : Process Scheduling *******************/
/* Desctiption : This is a brief replication     **/
/*  of process scheduling in an Operating System */
/************************************************/
 
#include <stdio.h> 
#include <stdlib.h>
#include <signal.h> 
#include <semaphore.h> 
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <sys/ipc.h> 
#include <sys/shm.h>
#include <string.h>
#include "processqueue.h"
#include "processinfo.h"


FILE *fptr;
sem_t *sem;
int pcbid,clkid;
PCB *pcb;
Clock *clk;
int linecount = 0;

int main (int argc,char *argv[]) 
{ 
	char arg1[10];
	char arg2[10];
	char arg3[20];
	char *logfile = "log";
	int startsec = 0,endsec = 0, diffSec = 0;
	long int SNano = 0, ENano = 0, dNano =0,difference =0;

	int opt, i,status,pid,t=2;
	int generateindex = 0;

	/* Queue declarations */
	struct Queue rr;
	struct Queue hq;
	struct Queue mq;
	struct Queue lq;
	signal(SIGALRM, sigintHandler);
    signal(SIGINT, sigintHandler);
	
	/*********************/
	
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

	fptr = fopen(logfile, "w");
    if(fptr == NULL)
    {
      fprintf(stderr,"File open error \n");
      exit(1);
    }
	fprintf(stderr,"Log file : %s\n",logfile);

	// Keys for Shared Memories for PCB and Clock
	key_t memkey = ftok("ponagand",'m');
	key_t clkkey = ftok("ponagand",'j');
	int sizepcb = 18 * sizeof(pcb);
	if((sem = sem_open(SemName, O_CREAT, 0666, 0)) == SEM_FAILED )
		fprintf(stderr,"Semaphore could not be opened\n");
	
	pcbid = shmget(memkey,sizepcb,0666|IPC_CREAT);
	if ( pcbid == -1 )
		fprintf(stderr,"Shared Memory : Error Opening");
	
	pcb = (PCB *)shmat(pcbid, NULL, 0);
	
	clkid = shmget(clkkey,sizeof(clk),0666|IPC_CREAT);
	if ( clkid == -1 )
		fprintf(stderr,"Clock Memory : Error opening");
	
	clk = (Clock *)shmat(clkid, NULL, 0);
	
	clk -> sec = 0;
	clk -> nano = 0;
	clk -> pid = -1;
	clk -> quantum = 0;
	clk -> count  = 0;
	
	
	
	// Initializing PCB for future use 
	for(i  = 0; i < 18; i++ )
	{
		pcb[i].CPU_time = 0.0;
		pcb[i].total_time = 0.0;
		pcb[i].burst_time = 0;
		pcb[i].pid = -1;
		pcb[i].priority = -1;
		pcb[i].status = 0;
	}
	
	
	
	
	
	snprintf(arg1,10,"%d", pcbid);
	snprintf(arg2,10,"%d",clkid);
	snprintf(arg3,20,"%s", SemName);
	
	alarm(t);
	       
    
 	int location = -1,proc;
	srand(time(NULL)); // Initiate some value to make rand() generate new values
	init(&rr);
	init(&lq);
	 
	while(1){
		
		if( (location = createChildProc()) >= 0  && clk -> sec  >= generateindex)
		{
	
	
			
			if( (pid = fork()) == 0)
			{
				
				execlp("./user","./user",arg1,arg2,arg3,(char *)NULL);
				fprintf(stderr,"%s failed to exec worker!\n",argv[0]);
				exit(0);
			}
			
			generateindex += rand()%3; 
			pcb[location].priority = setPriority();  
			pcb[location].pid = pid;
			if(pcb[location].priority == 0)
			{
				if(linecount < 10000)
					fprintf(fptr,"OSS: Generating process with PID %d (Round Robin High Priority) and putting it in queue 1 at time %d:%ld\n",pid,clk -> sec,clk -> nano);
					linecount++;
				push(&rr, pid);
			}
			else if(pcb[location].priority == 1)
			{
				if(linecount < 10000)
					fprintf(fptr,"OSS: Generating process with PID %d (High priority) and putting it in queue 0 at time %d:%ld\n",pid,clk -> sec,clk -> nano);
					linecount++;
				push(&lq,pid);
			}
			 else if(pcb[location].priority == 2)
			{
				if(linecount < 10000)
					fprintf(fptr,"OSS: Generating process with PID %d (Medium priority) and putting it in queue 0 at time %d:%ld\n",pid,clk -> sec,clk -> nano);
					linecount++;
				push(&rr,pid);
			}
			else if(pcb[location].priority == 3)
			{
				if(linecount < 10000)
					fprintf(fptr,"OSS: Generating process with PID %d (Low priority) and putting it in queue 0 at time %d:%ld\n",pid,clk -> sec,clk -> nano);
					linecount++;
				push(&lq,pid);
			} 

			location = -1;	 
		}
		
	
		clk -> nano += rand()%1000;	
		if(clk -> nano > NANO)
		{
			clk -> sec += clk -> nano/NANO;
			clk -> nano %= NANO;
		}	
		
		if((proc = pop(&rr)) > 0)
		{
			if(linecount < 10000)
				fprintf(fptr,"OSS: Dispatching pid : %d at %d:%ld\n",proc,clk -> sec,clk -> nano);
				linecount++;
			clk -> pid = proc;
			clk -> quantum = Quantum/2;
			endsec = clk -> sec;
			ENano = clk -> nano;
			diffSec = endsec - startsec;
			dNano = ENano - SNano;
			difference = diffSec * NANO + dNano;
			if(linecount < 10000)
				fprintf(fptr,"OSS: Total time this dispatch was %ld\n",difference,linecount++);
			sem_wait(sem);
			location = getPCBbyPID(proc);
			if(pcb[location].status == 1)
			{
				if(linecount < 10000)
					fprintf(fptr,"OSS: Receiving pid : %d executed %ld\n",proc,pcb[location].burst_time);
					linecount++;
				waitpid(proc, &status,0);
				if(linecount < 10000)
					fprintf(fptr,"OSS: PID %d finished execution at time  %f  Total : %f\n",proc,pcb[location].CPU_time,pcb[location].total_time);
					linecount++;
				clearPCB(location);
			}
			else
			{
			if(linecount < 10000)		
				fprintf(fptr,"OSS: Receiving that process with PID %d ran for %ld\n",proc,pcb[location].burst_time);
				linecount++;
			if(pcb[location].burst_time <= 5000000)
				if(linecount < 10000)
					fprintf(fptr,"OSS: not used it entire quantum \n");
					linecount++;
			if(linecount < 10000)
				fprintf(fptr,"OSS: Putting process with PID %d into queue 0\n",proc++);
				linecount;
			push(&rr,proc);
			}
			clk -> nano +=  pcb[location].burst_time;
			if(clk -> nano > NANO)
			{
				clk -> sec += clk -> nano/NANO;
				clk -> nano %= NANO;
			}
			
			startsec = clk -> sec;
			SNano = clk -> nano;
			location = -1;
			
		} 
		else if((proc = pop(&lq)) > 0)
		{
			if(linecount < 10000)
				fprintf(fptr,"OSS: Dispaching PID %d at time %d:%ld\n",proc,clk -> sec,clk -> nano);
				linecount++;
			clk -> pid = proc;
			clk -> quantum = Quantum;
			endsec = clk -> sec;
			ENano = clk -> nano;
			diffSec = endsec - startsec;
			dNano = ENano - SNano;
			difference = diffSec * NANO + dNano;
			if(linecount < 10000)
				fprintf(fptr,"OSS: Total time this dispatch was %ld\n",difference);
				linecount++;
			sem_wait(sem);
			location = getPCBbyPID(proc);
			if(pcb[location].status == 1)
			{
				if(linecount < 10000)
					fprintf(fptr,"OSS: Receiving that process with PID %d ran for %ld\n",proc,pcb[location].burst_time);
					linecount++;
				waitpid(proc, &status,0);
				if(linecount < 10000)
					fprintf(fptr,"OSS: PID %d finished execution at time  %f  Total : %f\n",proc,pcb[location].CPU_time,pcb[location].total_time);
					linecount++;
				clearPCB(location);
			}
			else
			{
			if(linecount < 10000)
				fprintf(fptr,"OSS: Receiving that process with PID %d ran for %ld\n",proc,pcb[location].burst_time);
				linecount++;
			if(pcb[location].burst_time <= 10000000)
				if(linecount < 10000)
					fprintf(fptr,"OSS: not used it entire quantum \n");
					linecount++;
			if(linecount < 10000)	
				fprintf(fptr,"OSS: Putting process with PID %d into queue 1\n",proc);
				linecount++;
			push(&lq,proc);
			}
			clk -> nano +=  pcb[location].burst_time;
			if(clk -> nano > NANO)
			{
				clk -> sec += clk -> nano/NANO;
				clk -> nano %= NANO;
			}	
			startsec = clk -> sec;
			SNano = clk -> nano;
			location = -1;
		}
		
		else
			advanceClock();
		
	
	}
 
	shmdt(pcb);
	shmdt(clk);
	shmctl(pcbid,IPC_RMID,NULL);
	shmctl(clkid,IPC_RMID,NULL);
	return 0;
}

void sigintHandler(int sig_num) 
{ 

    if(sig_num == 2)
    	fprintf(stderr,"Program: Interrupted\n");
    else
	fprintf(stderr,"Program: Time limit exceeded\n");
    int i;
	fprintf(stderr,"Clock Time: Seconds = %d, Nano %ld\n", clk -> sec, clk -> nano);
	fprintf(stderr,"Total Childs Forked : %d\n",clk -> count);
	
   for(i = 0; i < 18; i++)
   {
       if(pcb[i].pid > 0 )
       {
           //fprintf(stderr,"Killing Child : %d --- %d\n",pcb[i].pid,i);
	       kill(pcb[i].pid, SIGTERM);
       }
    }
	fprintf(stderr,"Remaining children being removed\n");
    
    
  
	shmdt(pcb);
	shmdt(clk);
	fprintf(stderr,"Shared Memory: Detached\n");
	fclose(fptr);
    sem_unlink(SemName);
	fprintf(stderr,"Semaphore: Unlinked\n");
    shmctl(pcbid,IPC_RMID,NULL);
	shmctl(clkid,IPC_RMID,NULL);
	fprintf(stderr,"Shared Memory: Cleared \n");
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

int getPCBbyPID(int pid)
{
	int i;
	for(i  = 0; i < 18; i++ )
		if(pcb[i].pid == pid)			
			return i;
	return -1;
}

void clearPCB(int location)
{
		pcb[location].CPU_time = 0.0;
		pcb[location].total_time = 0.0;
		pcb[location].burst_time = 0;
		pcb[location].pid = -1;
		pcb[location].priority = -1;
		pcb[location].status = 0;
}

void advanceClock()
{
	clk -> sec += 1;
	if(clk -> nano > NANO)
		{
			clk -> sec += clk -> nano/NANO;
			clk -> nano %= NANO;
		}	
	
}
	  
