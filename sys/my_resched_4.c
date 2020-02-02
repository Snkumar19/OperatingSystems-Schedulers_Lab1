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
	
        if (currpid != 0)
	        kprintf ("\n Resched Called by Process %d \n ", currpid);
       

	optr= &proctab[currpid];
	/* no switch if null process is current process and ready queue is empty*/
	if ((optr->pprio == 0) && (lastkey(rdytail)<optr->pprio))
        	return(OK);

	/* insert the current running process to ready queue*/
	if (optr->pstate == PRCURR)
	{
		optr->pstate = PRREADY;
		if (currpid != 0)
			kprintf("\nInserting process:%d",currpid);
		insert(currpid,rdyhead,optr->pprio);
	}

	/*check if random is greater*/
	 kprintf("\nRandom 0:%d",random);
	
	
	prev = q[rdytail].qprev;
	//kprintf("\nPrev=%d", prev);
	if (random > lastkey(rdytail)) 
	{
		//prev = q[rdytail].qprev;
		if (currpid != 0)
                        kprintf("\nBefore While prev:%d",prev);
		while((random > q[prev].qkey) && (q[prev].qkey>=0))
        	{
                	random = random - q[prev].qkey;
                	prev =q[prev].qprev;
			if(prev>=NPROC){
                        	prev = q[prev].qnext;
				break;
			}
               		kprintf("\nPrev1: %d", prev);
        	}
		if (currpid != 0)
                        kprintf("\nAfter While prev:%d",prev);


	}
	kprintf("\nPrev2: %d", prev);
	nptr = &proctab[ (currpid = dequeue(prev)) ];
        nptr->pstate = PRCURR;          


	#ifdef  RTCLOCK
        	preempt = QUANTUM;              /* reset preemption counter     */
	#endif

        ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);

        /* The OLD process returns here when resumed. */
        return OK;

}

