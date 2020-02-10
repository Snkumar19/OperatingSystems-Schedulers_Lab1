/* insert.c  -  insert */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>

/*------------------------------------------------------------------------
 * insert.c  --  insert an process into a q list in key order
 *------------------------------------------------------------------------
 */
int insert(int proc, int head, int key)
{
	int	next;			/* runs through list		*/
	int	prev;
	int     test1;
	int	test2;

	next = q[head].qnext;
	while (q[next].qkey < key)	/* tail has maxint as key	*/
		next = q[next].qnext;
	q[proc].qnext = next;
	q[proc].qprev = prev = q[next].qprev;
	q[proc].qkey  = key;
	q[prev].qnext = proc;
	q[next].qprev = proc;
	struct pentry *proc1= &proctab[proc];
	//kprintf("\n---------------In Insert------------------");
	//test1 = q[proc].qprev;
	//test2 = q[proc].qnext;
	//kprintf("\n\nname: %s \nkey:%d, currpid=%d, prev = %d, next = %d, prev-key =%d, next-key = %d \n ", proc1->pname, q[proc].qkey,proc, q[proc].qprev, q[proc].qnext, q[test1].qkey, q[test2].qkey);
	//kprintf("\n---------------End Insert------------------");
	//kprintf("Queue: %d", printq(head));
	return(OK);
}
