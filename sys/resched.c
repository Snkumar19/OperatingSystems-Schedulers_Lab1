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
unsigned int once = 0;
int resched()
{
        register struct pentry  *optr;  /* pointer to old process entry */
        register struct pentry  *nptr;  /* pointer to new process entry */

        int prev,prev1;
        int flag =0;
        struct pentry  *ptr=&proctab[currpid];
	int random = 0;	
	unsigned int i =0, priority_sum =0;
	
	extern long ctr1000;
	if (getschedclass() == RANDOMSCHED)
	{
		optr= &proctab[currpid];
		
		 /* insert the current running process to ready queue*/
                if (optr->pstate == PRCURR)
                {
                        optr->pstate = PRREADY;
                        insert(currpid,rdyhead,optr->pprio);
                }
			

		prev = q[rdytail].qprev;                // Last Entity before Tail of ReadyQueue
                while (prev < NPROC) {
			priority_sum += proctab[prev].pprio;
                        prev =  q[prev].qprev;
                }
		
		if (priority_sum == 0)
		{
			currpid = dequeue(NULLPROC);
		}
		else
		{
		
			srand(ctr1000);
			random = rand() % priority_sum; 
	
			unsigned int prio_of_rdyProc = lastkey(rdytail);        // Priority of the last process in the ReadyQueue
			if (random < prio_of_rdyProc)
			{
				/* If the Random value is Less than the highest value in the Ready Queue
                        	* then, pid of tail (proc with highest prio) and assign to currpid*/
                        	currpid = getlast(rdytail);
			}
			else
			{
 	       			prev = q[rdytail].qprev;                		// Last Entity before Tail of ReadyQueue	
			
	
				if(prev > NPROC)	
					currpid = dequeue(NULLPROC);

				/* Random Scheduler Logic */
				if (random >= prio_of_rdyProc) {
					prev = q[rdytail].qprev; 		// Last Entity before Tail of ReadyQueue
					while (prev < NPROC){
						if (random >= q[prev].qkey){
							random -=  q[prev].qkey;
							prev =  q[prev].qprev;
						}
						else{
							break;
						} 	
					
					}
					if(prev > NPROC)
                                       		currpid = dequeue(NULLPROC);
                                	else
                                        	currpid = dequeue(prev);
					
				}
			}
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

		/*update goodness for current process*/
		optr->goodness = (optr->goodness - optr->counter) + preempt;
		if (preempt == 0)
			optr->goodness = 0;
		optr->counter = max(preempt, 0);	
		
		/*update counter and goodness for null process*/
		if (currpid == NULLPROC){
			optr->counter = 0;
			optr->goodness = 0;
		}
		
		/*update counter and goodness for counter <=0 , Important as this process should not be considered for scheduling*/
		if(optr->counter <= 0){
                        optr->counter = 0;
                        optr->goodness = 0;
                }

		 /*Get highest goodness value from all processes in Ready State*/

		 prev = q[rdytail].qprev;                // Last Entity before Tail of ReadyQueue

                 while (prev < NPROC) {
                        if(proctab[prev].goodness >= highest_goodness){
                        	 highest_goodness = proctab[prev].goodness;
				highest_goodness_pid = prev;
			}
                         prev =  q[prev].qprev;
                }


		
		/* Possible outcomes for highest goodness:
 		 * 1. highest goodenss = 0
		 * 2. highest goodenss > 0
		 * */

		/* if highest goodness is 0:
 		* 1. Start epoch if epoch is not running denoted with epoch start flag
 		* 2. if the current process is NULLPROC
 		* 3. if current process is not nullproc, add current process to RQ & ctxsw to nullproc */	 	
//		 kprintf("\n 0 --------------------------------------------------------- 0 \n");
//		 kprintf("\n 0 currpid : , optr->counter is : %d optr->goodness is %d, prempt is %d 0 \n",currpid, optr->counter, optr->goodness, preempt);
		if (highest_goodness <= 0){
			if (optr->pstate != PRCURR || optr->counter == 0){
		//		 kprintf("\n Highest goodness 0 - Case 1");
				if (start_epoch_flag){
		//			kprintf("\n Highest goodness 0 - Case 2");
					start_epoch_flag = 0;
					start_epoch();
			
					preempt = optr->counter;
				}

			}
			if (currpid == NULLPROC){
				//kprintf("\n Highest goodness 0 - Case 3");
				//kprintf("\n 1 --------------------------------------------------------- 1 \n");
				preempt = optr->counter;	
				return (OK);
			}
			/* if none of the processes are eligible to run */
			if (optr->pstate == PRCURR){
		//		kprintf("\n Highest goodness 0 - Case 4");
				optr->pstate = PRREADY;
	                        insert(currpid,rdyhead,optr->pprio);
			}

			nptr = &proctab[ (currpid = dequeue(NULLPROC)) ];
	                nptr->pstate = PRCURR;          /* mark it currently running    */
		//	kprintf("\n Highest goodness 0 - Case 5");
		//	kprintf ("\n Case 1 : DQ NULL PROC: highest goodness is %d and highest_pid is %d, optr-goodness is %d, and currpid is %d \n", highest_goodness, 
		 //	highest_goodness_pid, optr->goodness, currpid);
	                #ifdef  RTCLOCK
        	                preempt = QUANTUM;              /* reset preemption counter     */
               		#endif
		//	 kprintf ("\n Case 1-preempt check : DQ NULL PROC: highest goodness is %d and highest_pid is %d, optr-goodness is %d, and currpid is %d, preempt = %d \n", 
		//	 	highest_goodness, highest_goodness_pid, optr->goodness, currpid, preempt);
		//	kprintf("\n 1 --------------------------------------------------------- 2 \n");
                	ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);

                	return OK;
		
		}
		/*1. check if current process goodness is > highest goodness - then current process is eligible to run */
		else if (optr->pstate == PRCURR && optr->goodness > highest_goodness){
		//	kprintf ("\n If you are current process  : highest goodness is %d and pid is %d \n", highest_goodness, highest_goodness_pid);
		//	kprintf ("\n If you are PRCURR: highest goodness is %d and highest_pid is %d, optr-goodness is %d, and currpid is %d \n", highest_goodness, 
		//	 	highest_goodness_pid, optr->goodness, currpid);
		//	kprintf("\n Preempt is : %d, optr->counter is : %d \n",preempt, optr->counter);
			return(OK);
				
		}

		/*Here we know that the process is not the current process - Context switch*/
		else if (optr->counter == 0 || optr->goodness < highest_goodness || optr->pstate != PRCURR){

			/* Schedule the process with highest goodness */
		//	 kprintf ("\n Final Case  : highest goodness is %d and pid is %d \n", highest_goodness, highest_goodness_pid);
		//	 kprintf ("\n Final Case: highest goodness is %d and highest_pid is %d, optr-goodness is %d, and currpid is %d \n", highest_goodness, highest_goodness_pid, 
		//	 	optr->goodness, currpid);
			if (optr->pstate == PRCURR) {
                        	optr->pstate = PRREADY;
                        	insert(currpid,rdyhead,optr->pprio);
                	}

			nptr = &proctab[currpid = dequeue(highest_goodness_pid)];
			nptr -> pstate = PRCURR;
			preempt = nptr->counter;
		//		kprintf("\n 3 --------------------------------------------------------- 1 \n");
			ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);

                	/* The OLD process returns here when resumed. */
              		  return OK;
		}
		else
		   return SYSERR;
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

