#include <conf.h>
#include <kernel.h>
#include <q.h>
#include <lab1.h>
/* test1.c
 * This test program creates three processes, prA, prB, and prC, at
 * priority 20.  The main process also has priority 20.
 */

int prch(), prA, prB, prC;

main()
{
	int i;
	setschedclass(LINUXSCHED);
	kprintf("\n\nTEST1:\n");
	resume(prA = create(prch,2000,20,"proc A",1,'A'));
	resume(prB = create(prch,2000,20,"proc B",1,'B'));
	resume(prC = create(prch,2000,20,"proc C",1,'C'));

	while (1) {
		kprintf("%c", 'D');
		for (i=0; i< 10000; i++);
	}
	kprintf("\n");
}

prch(c)
char c;
{
	int i;

	while(1) {
		kprintf("%c\n", c);
		for (i=0; i< 10000; i++);
	}
}
