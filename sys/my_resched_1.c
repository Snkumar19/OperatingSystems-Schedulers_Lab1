/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>

unsigned long currSP;	/* REAL sp of current process */
extern int ctxsw(int, int, int, int);
int random = 0;
/*-----------------------------------------------------------------------
 * resched  --  reschedule processor to highest priority ready process
 *
 * Notes:	Upon entry, currpid gives current process id.
 *		Proctab[currpid].pstate gives correct NEXT state for
 *			current process if other than PRREADY.
 *------------------------------------------------------------------------
 */

int resched()
{
        register struct pentry  *optr;  /* pointer to old process entry */
        register struct pentry  *nptr;  /* pointer to new process entry */

        int prev;
        int flag =0;
        struct pentry  *ptr=&proctab[currpid];

        /*kprintf("\n In resched");     */
        random = 30;
        /* no switch needed if current process priority higher than next*/
        if (currpid != 0)
        kprintf ("\n Resched Called by Process with priority: %d \n ", currpid);
        

/*
 *         if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
 *                    (lastkey(rdytail)<optr->pprio)) {
 *                                    if (currpid != 0)
 *                                                    kprintf("\nNo switch needed\n");
 *                                                                    return(OK);
 *                                                                            }
 *                                                                            */


        if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
                   (random<optr->pprio)) {
                        if (currpid != 0)
                                kprintf("\nNo switch needed\n");
                        return(OK);
        }

        if ((random - optr->pprio) >= 0)
                random = random - optr->pprio;
        prev = q[rdytail].qprev;
        kprintf("\n\tPrev0: %d\n", prev);
        kprintf("\nrandom = %d\n", random);

        while((random - q[prev].qkey) >=0 && q[prev].qkey>=0)
 {
                random = random - q[prev].qkey;
                prev =q[prev].qprev;
                kprintf("\n\tPrev1: %d\n", prev);
                /*flag = 1;*/

        }

if(prev>=NPROC)
prev = q[prev].qnext;
kprintf("\n\tPrev2: %d\n", prev);
kprintf("random = %d", random);
        /* force context switch */

        if (optr->pstate == PRCURR) {
                optr->pstate = PRREADY;
                if (currpid != 0)
         kprintf("\nContext Switch Start\n");
                insert(currpid,rdyhead,optr->pprio);
        }

        /* remove highest priority process at end of ready list */

if (prev>NPROC)
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

