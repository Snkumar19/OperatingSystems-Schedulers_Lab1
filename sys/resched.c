/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lab1.h>

unsigned long currSP;	/* REAL sp of current process */
extern int ctxsw(int, int, int, int);
/* -----------------------------------------------------------------------
 * resched  --  reschedule processor to highest priority ready process
 *
 * Notes:	Upon entry, currpid gives current process id.
 *		Proctab[currpid].pstate gives correct NEXT state for
 *			current process if other than PRREADY.
 * ------------------------------------------------------------------------
 */
unsigned int once = 1;
int resched()
{
        register struct pentry  *optr;  /* pointer to old process entry */
        register struct pentry  *nptr;  /* pointer to new process entry */

        int prev;
        int flag =0;
        struct pentry  *ptr=&proctab[currpid];
	int random = 0;	
	unsigned int i =0, priority_sum =0;
	
	extern long ctr1000;
	if (getschedclass() == RANDOMSCHED)
	{
		optr= &proctab[currpid];
		//if (once) {
			//kprintf ("\n Random Sched \n");
		//once = 0;
		//}
 		/* 1. Random number allocation using Process priority sum	*/
	
		for (i =0; i < NPROC; i++)
		{
			if(proctab[i].pprio != 0){
				priority_sum += proctab[i].pprio;
			}
		 
		}
		
		/* If Null Process is the only process in the queue */
		if (priority_sum <= 0)
			priority_sum = 1;

		/*  Assign Random_Number by taking any random return value and % to be between 0 and prio_sum-1 */
		srand(ctr1000);
		random = rand() % priority_sum; 

	
		/* no switch if null process is current process and ready queue is empty*/
	
		/* insert the current running process to ready queue*/
		if (optr->pstate == PRCURR)
		{
			optr->pstate = PRREADY;
			//if (currpid != 0)
			//kprintf("\nInserting process:%d",currpid);
			insert(currpid,rdyhead,optr->pprio);
		}

		//kprintf("\n Random: %d",random);	
		unsigned int prio_of_rdyProc = lastkey(rdytail);     // Priority of the last process in the ReadyQueue
        	prev = q[rdytail].qprev;                // Last Entity before Tail of ReadyQueue	
	
		if(prev > NPROC)	
			currpid = NULLPROC;

		/* Random Scheduler Logic */
		else if (random > prio_of_rdyProc) {
			/* If Random value is greater than the key of tail -> this means that there is no proc that
 			* can be selected from the queue */
			// Step 1 . Reduce the value of random and keep checking it with all the priorities in the queue
			// Step 2 . If prev > NPROC, this means that the list is empty at this point, then choose which proc?
			// Step 3 . If prev < NPROC, then there is a candidate, choose that by dequeueing the process to currpid
		
			prio_of_rdyProc = lastkey(rdytail); 	// Priority of the last process in the ReadyQueue
			prev = q[rdytail].qprev; 		// Last Entity before Tail of ReadyQueue
			while (random > prio_of_rdyProc && q[prev].qkey > 0){
				random -=  prio_of_rdyProc;
				prev =  q[prev].qprev; 		
				prio_of_rdyProc = q[prev].qkey;
			}
			if(prev > NPROC)
                		currpid = NULLPROC;
			else
				currpid = dequeue(prev);
		}
		else {
			/* If the Random value is Less than the highest value in the Ready Queue
 		 	* then, pid of tail (proc with highest prio) and assign to currpid*/ 		
			currpid = getlast(rdytail);
		}
	
		//kprintf("\nPrev2: %d", prev);
		nptr = &proctab[currpid];
        	nptr->pstate = PRCURR;          


		#ifdef  RTCLOCK
        		preempt = QUANTUM;              /* reset preemption counter     */
		#endif

        	ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
	

        	/* The OLD process returns here when resumed. */
        	return OK;
	}
	else if (getschedclass() == LINUXSCHED)
	{
		int highest_goodness = 0;
		unsigned int highest_goodness_pid=0, start_epoch_flag = 1;
		struct pentry  *p;
		
		if (once) {
                	kprintf ("\n Linux Sched \n");
                        once = 0;                                
                 }

		optr = &proctab[currpid];

		/*update counter and goodness for current process*/
		optr->counter = max(preempt, 0);
		if (optr->counter == 0)
			optr->goodness = 0;
		else
			optr->goodness = optr->goodness - optr->counter + preempt;

		/*update counter and goodness for null process*/
		if (currpid == NULLPROC){
			optr->counter = 0;
			optr->goodness = 0;
		}


		 /*Highest goodness value from all processes in Ready Queue*/

		 for (i = 0; i < NPROC; i++)
                {
                        if(proctab[i].pstate == PRREADY){
                                if (proctab[i].goodness > highest_goodness){
					highest_goodness_pid = i;
					highest_goodness = proctab[i].goodness;
				}
                        }

                }
						
		
		kprintf("\nHighest goodness = %d\n", highest_goodness);
		kprintf("\nHighest goodness pid = %d\n", highest_goodness_pid);

		
		/* Possible outcomes for highest goodness:
 		 * 1. highest goodenss = 0
		 * 2. highest goodenss > 0
		 * */

		/* if highest goodness is 0:
 		* 1. Start epoch if epoch is not running denoted with epoch start flag
 		* 2. if the current process is NULLPROC
 		* 3. if current process is not nullproc, add current process to RQ & ctxsw to nullproc */	 	
		if (highest_goodness <= 0){
			if (optr->pstate != PRCURR || optr->counter == 0){
				kprintf("\n Hisghest goodness 0 - Case 1");
				if (start_epoch_flag){
					kprintf("\n Hisghest goodness 0 - Case 2");
					start_epoch_flag = 0;
					start_epoch();
				}

			}
			if (currpid == NULLPROC){
				kprintf("\n Hisghest goodness 0 - Case 3");
				return (OK);
			}
			/* if none of the processes are eligible to run */
			if (optr->pstate == PRCURR){
				kprintf("\n Hisghest goodness 0 - Case 4");
				 optr->pstate = PRREADY;
	                        insert(currpid,rdyhead,optr->pprio);
			}

			nptr = &proctab[ (currpid = dequeue(NULLPROC)) ];
	                nptr->pstate = PRCURR;          /* mark it currently running    */
			kprintf("\n Hisghest goodness 0 - Case 5");
	                #ifdef  RTCLOCK
        	                preempt = QUANTUM;              /* reset preemption counter     */
               		#endif

                	ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);

                	return OK;
		
		}
		/*1. check if current process goodness is > highest goodness - then current process is eligible to run */
		else if (optr->pstate == PRCURR){
			if (optr->goodness > highest_goodness)

				return(OK);
			else
			{
				 if (optr->pstate == PRCURR) {
                        		optr->pstate = PRREADY;
                       			 insert(currpid,rdyhead,optr->pprio);
                		}

 		               	/* remove highest priority process at end of ready list */
		
        		        nptr = &proctab[ (currpid = dequeue(highest_goodness_pid)) ];
                		nptr->pstate = PRCURR;          /* mark it currently running    */
				preempt = nptr->counter;
                		ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
	
        		        /* The OLD process returns here when resumed. */
	        	        return OK;
			}
				
		}

		/*Here we know that the process is not the current process - insert it to RQ*/
		else if (optr->counter == 0 || optr->goodness < highest_goodness){

			/* Schedule the process with highest goodness */
		
			nptr = &proctab[currpid = dequeue(highest_goodness_pid)];
			nptr -> pstate = PRCURR;
			preempt = nptr->counter;
		
			ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);

                	/* The OLD process returns here when resumed. */
              		  return OK;
		}
	}
	else{
		/* no switch needed if current process priority higher than next*/
//		kprintf ("\n Default Sched \n");
		if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
	   	(lastkey(rdytail)<optr->pprio)) {
			return(OK);
		}
	
		/* force context switch */

		if (optr->pstate == PRCURR) {
			optr->pstate = PRREADY;
			insert(currpid,rdyhead,optr->pprio);
		}

		/* remove highest priority process at end of ready list */

		nptr = &proctab[ (currpid = getlast(rdytail)) ];
		nptr->pstate = PRCURR;		/* mark it currently running	*/
		#ifdef	RTCLOCK
			preempt = QUANTUM;		/* reset preemption counter	*/
		#endif
	
		ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
	
		/* The OLD process returns here when resumed. */
		return OK;
	}
}

