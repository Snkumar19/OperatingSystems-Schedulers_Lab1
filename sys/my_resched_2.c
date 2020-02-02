/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>

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

int resched()
{
        register struct pentry  *optr;  /* pointer to old process entry */
        register struct pentry  *nptr;  /* pointer to new process entry */

        int prev;
        int flag =0;
        struct pentry  *ptr=&proctab[currpid];
	int random = 0,min_priority =99;	
	unsigned int i =0, priority_sum =0;

	/*1. Random number allocation using Process priority sum*/
	for (i =0; i < NPROC; i++)
	{
		if(proctab[i].pprio != 0){
			priority_sum += proctab[i].pprio;
		}
		 
		
	}
	
	
	 random = rand() % priority_sum;
//	kprintf("\nrand= %d; min = %d",random,min_priority);

	
//	 random = rand() % priority_sum;
	if (currpid != 0){
       		kprintf("\nPriority Sum = %d", priority_sum);	
		kprintf("\nInit Randon = %d", random);
	}

        //random = 30;

	
        if (currpid != 0)
	        kprintf ("\n Resched Called by Process %d \n ", currpid);
        

/*
 *  no switch needed if current process priority higher than next
        if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
 *                    (lastkey(rdytail)<optr->pprio)) {
 *                                    if (currpid != 0)
 *                                                    kprintf("\nNo switch needed\n");
 *                                                                    return(OK);
 *                                                                            }
 */


	/* no switch nedded if random value is less than tail priority)*/
        if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
                   (random<(lastkey(rdytail)))) {
                        if (currpid != 0)
                                kprintf("\nNo switch needed\n");
                        return(OK);
        }


	/* no switch if null process is current process and ready queue is empty*/
	if ((optr->pprio == 0) && (lastkey(rdytail)<optr->pprio))
                                return(OK);

	/* if random greater than tail priority, get the differnce and get the ID of last value in the queue*/
        if ((random - (lastkey(rdytail))) >= 0)
              	random = random - lastkey(rdytail);
        prev = q[rdytail].qprev;
        kprintf("\nPrev0: %d", prev);
        kprintf("\nRandom0 = %d", random);

	/* get the prcess which has priority greater than random value generated */
        while((random - q[prev].qkey) >=0 && q[prev].qkey>=0)
 	{
                random = random - q[prev].qkey;
                prev =q[prev].qprev;
                kprintf("\nPrev1: %d", prev);
                /*flag = 1;*/

        }

	/* if process at the head of the queue is picked, get the next value*/
	if(prev>=NPROC)
		prev = q[prev].qnext;
	kprintf("\nPrev2: %d", prev);
	kprintf("\nRandom1 = %d", random);

        /* force context switch */
        if (optr->pstate == PRCURR) {
                optr->pstate = PRREADY;
                if (currpid != 0)
         		kprintf("\nContext Switch Start\n");
                insert(currpid,rdyhead,optr->pprio);
        }

        /* remove highest priority process at end of ready list */

	
	if (prev>=NPROC)
		prev = 0;
        nptr = &proctab[ (currpid = dequeue(prev)) ];
        nptr->pstate = PRCURR;          /* mark it currently running    */
	#ifdef  RTCLOCK
        	preempt = QUANTUM;              /* reset preemption counter     */
	#endif

        ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);

        /* The OLD process returns here when resumed. */
        if (currpid != 0)
        	kprintf("\nContext Switch end\n");
        return OK;

}

