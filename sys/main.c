/* user.c - main */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include<lab1.h>

/*------------------------------------------------------------------------
 *  main  --  user main program
 *------------------------------------------------------------------------
 */
extern int ctr1000;
int prch(), prA, prB, prC;

int prA, prB, prC;
int proc_a(char), proc_b(char), proc_c(char);
int proc(char);
volatile int a_cnt = 0;
volatile int b_cnt = 0;
volatile int c_cnt = 0;
/*
prch(c)
char c;
{
	int i;
	//kprintf ("\n In Proc A %d \n", ctr1000);
        //sleep(5);
	//kprintf ("\n In Proc A after  Sleep statement: %d \n", ctr1000);
        for (i = 0; i < 10000; i++);
	//kprintf ("\n In Proc A after For Loop statement: %d \n", ctr1000);
}*/


int main()
{
	kprintf("\n\nHello World, Xinu lives\n\n");
	//kprintf( " \n In Main now =%d", numproc);
		setschedclass(RANDOMSCHED);
		prA = create(proc_a, 2000, 15, "proc A", 1, 'A');
		prB = create(proc_b, 2000, 30, "proc B", 1, 'B');
		prC = create(proc_c, 2000, 45, "proc C", 1, 'C');
		resume(prA);
		resume(prB);
		resume(prC);
		//sleep(10);
		kill(prA);
		kill(prB);
		kill(prC);

		kprintf("\nTest Result: A = %d, B = %d, C = %d\n", a_cnt, b_cnt, c_cnt);
return 0;
}
int proc_a(char c) {
	int i;
	kprintf("Start... %c\n", c);
	b_cnt = 0;
	c_cnt = 0;

	while (1) {
		for (i = 0; i < 10000; i++)
			;
		a_cnt++;
	}
	kprintf("a count=%d", a_cnt);
	return 0;
}

int proc_b(char c) {
	int i;
	kprintf("Start... %c\n", c);
	a_cnt = 0;
	c_cnt = 0;

	while (1) {
		for (i = 0; i < 10000; i++)
			;
		b_cnt++;
	}
	return 0;
}

int proc_c(char c) {
	int i;
	kprintf("Start... %c\n", c);
	a_cnt = 0;
	b_cnt = 0;

	while (1) {
		for (i = 0; i < 10000; i++)
			;
		c_cnt++;
	}
	return 0;
}

		
