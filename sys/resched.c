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
			
		/* get sum of all priorities in RQ*/
		prev = q[rdytail].qprev;                // Last Entity before Tail of ReadyQueue
                while (prev < NPROC) {
			priority_sum += proctab[prev].pprio;
                        prev =  q[prev].qprev;
                }

		/* if sum is 0 - there is only null process*/		
		if (priority_sum == 0)
		{
			currpid = dequeue(NULLPROC);
		}
		/*otherwise - find random number
 		*1. check if random is less than priority of last process, if yes dequeue it
 		*2. if less, subtract the value and get the pid to dequeue 			
 		*3. if null, dequeue null
 		*/
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
		
		/*if (once) {
                	kprintf ("\n Linux Sched \n");
                        once = 0;                                
                 }
		*/

		optr = &proctab[currpid];

		/*update goodness for current process*/
		optr->goodness = (optr->goodness - optr->counter) + preempt; // (priority + counter)
		optr->counter = max(preempt, 0);	
		
		/*update counter and goodness for null process*/
		if (currpid == NULLPROC){
			optr->counter = 0;
			optr->goodness = 0;
		}
		
		/*update counter and goodness for counter <=0 , Important as this process should not be considered for scheduling*/
		if(optr->counter <= 0){
                        optr->goodness = 0;
                }

		 /*Get highest goodness value from all processes in Ready State*/

		 prev = q[rdytail].qprev;                // Last Entity before Tail of ReadyQueue

		/* Also handles the case when all processes have same priority */
                 while (prev < NPROC) {
                        if(proctab[prev].goodness > highest_goodness){
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
 		* 1. Start epoch if epoch is not running
 		* 2. if the current process is NULLPROC
 		* 3. if current process is not nullproc, add current process to RQ & ctxsw to nullproc */	 	

		//kprintf("\n 0 --------------------------------------------------------- 0 \n");
		// kprintf("\n 0 currpid : , optr->counter is : %d optr->goodness is %d, prempt is %d 0 \n",currpid, optr->counter, optr->goodness, preempt);
		if (highest_goodness <= 0){
			if (optr->pstate != PRCURR || optr->counter == 0){
				//kprintf("\n Highest goodness 0 - Case 1");
				//if (start_epoch_flag){
				//kprintf("\n Highest goodness 0 - Case 2");
				//start_epoch_flag = 0;
				start_epoch();	
				//}

			}
			
			/* If null process,  return */
			if (currpid == 0){
				//kprintf("\n Highest goodness 0 - Case 3");
				//kprintf("\n 1 --------------------------------------------------------- 1 \n");
				return (OK);
			}

			/* if none of the processes are eligible to run */
			if (optr->pstate == PRCURR){
				//kprintf("\n Highest goodness 0 - Case 4");
				optr->pstate = PRREADY;
	                        insert(currpid,rdyhead,optr->pprio);
			}
			/* Now that all values have been reinitialized and current proc is not NULL
 			 * schedule process with highest goodness, if highest goodness is zero schedule NULL */
			highest_goodness = 0;
			highest_goodness_pid = 0;
			prev = q[rdytail].qprev;                // Last Entity before Tail of ReadyQueue
			while (prev < NPROC) {
                        	if(proctab[prev].goodness > highest_goodness){
                                	highest_goodness = proctab[prev].goodness;
                               		 highest_goodness_pid = prev;
                        	}
                        	 prev =  q[prev].qprev;
                	} 	

			if (highest_goodness == 0){
				nptr = &proctab[ (currpid = dequeue(NULLPROC)) ];
				preempt = QUANTUM; 
			}
			else{
				nptr = &proctab[ (currpid = dequeue(highest_goodness_pid)) ];
	                	preempt = nptr->counter;

			}
			nptr->pstate = PRCURR;          /* mark it currently running    */
			//kprintf("\n Highest goodness 0 - Case 5");
			//kprintf ("\n Case 1 : DQ NULL PROC: highest goodness is %d and highest_pid is %d, optr-goodness is %d, and currpid is %d \n", highest_goodness, 
		 	//highest_goodness_pid, optr->goodness, currpid);
			// kprintf ("\n Case 1-preempt check : DQ NULL PROC: highest goodness is %d and highest_pid is %d, optr-goodness is %d, and currpid is %d, preempt = %d \n", 
			//highest_goodness, highest_goodness_pid, optr->goodness, currpid, preempt);
			//kprintf("\n 1 --------------------------------------------------------- 2 \n");
                	ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);

                	return OK;
		
		}
		/* check if current process goodness is > highest goodness - then current process is eligible to run */
		else if (highest_goodness < optr->goodness && optr->pstate == PRCURR) {
				//kprintf ("\n If you are current process  : highest goodness is %d and pid is %d \n", highest_goodness, highest_goodness_pid);
				//kprintf ("\n If you are PRCURR: highest goodness is %d and highest_pid is %d, optr-goodness is %d, and currpid is %d \n", highest_goodness, 
				//highest_goodness_pid, optr->goodness, currpid);
				//kprintf("\n Preempt is : %d, optr->counter is : %d \n",preempt, optr->counter);
			return(OK);
		}
				
			

		/* Context Switch - 
 		* 1. When there's a highest goodness process in the RQ whose goodness is greater than current process
 		* 2. or if current process' counter is 0, which means a new process can be scheduled
 		* 3. or if a blocked process has moved to ready state 
 		* */
		else{
			if (highest_goodness > optr->goodness || optr->counter == 0 ){

				//kprintf ("\n Final Case  : highest goodness is %d and pid is %d \n", highest_goodness, highest_goodness_pid);
				//kprintf ("\n Final Case: highest goodness is %d and highest_pid is %d, optr-goodness is %d, and currpid is %d \n", highest_goodness, highest_goodness_pid, 
				//optr->goodness, currpid);
			
				if (optr->pstate == PRCURR) 
				{
                        		optr->pstate = PRREADY;
                        		insert(currpid,rdyhead,optr->pprio);
                		}

				nptr = &proctab[currpid = dequeue(highest_goodness_pid)];
				nptr -> pstate = PRCURR;
				preempt = nptr->counter;
				//kprintf("\n 3 --------------------------------------------------------- 1 \n");
				ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);

                		/* The OLD process returns here when resumed. */
              		  	return OK;
			}
	
			if(optr->pstate != PRCURR)
			{
				nptr = &proctab[currpid = dequeue(highest_goodness_pid)];
                        	nptr -> pstate = PRCURR;
                       	 	preempt = nptr->counter;
                        	//kprintf("\n 3 --------------------------------------------------------- 13 \n");
                         	ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
				return OK;
			}
		}	
	}
	else{
		/* no switch needed if current process priority higher than next*/
		//kprintf ("\n Default Sched \n");
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

