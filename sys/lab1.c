#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include<lab1.h>

void setschedclass(int sched_class)
{
	schedflag = sched_class;

}

int getschedclass()
{
	return schedflag;
}

void start_epoch()
{
	unsigned int i = 0;
	//if (currpid!=0)
	//kprintf("\nStart EPOCH \n");
	for ( i = 0; i < NPROC; i++)
	{
		if (proctab[i].pstate != PRFREE)
		{
			if (proctab[i].counter == 0)
				proctab[i].counter = proctab[i].pprio;
			else
				proctab[i].counter = (proctab[i].counter/2) + proctab[i].pprio;
			proctab[i].goodness = proctab[i].counter + proctab[i].pprio;
		}
	}
}
