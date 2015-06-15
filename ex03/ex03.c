#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>

#include <native/task.h>
#include <native/timer.h>
#include <native/sem.h>
 
#include  <rtdk.h>

#define ITER 10

static RT_TASK  t1;
static RT_TASK  t2;

int global = 0;

static RT_SEM sem;

void taskOne(void *arg)
{
    int i;

    for (i=0; i < ITER; i++)
    {
	//while(rt_sem_p (&sem, TM_NONBLOCK) == -EWOULDBLOCK){
	//	  rt_task_sleep(100000000);      	
        //}
	rt_sem_p (&sem, TM_INFINITE);
        rt_printf("I am taskOne and global = %d................\n", ++global);
	rt_sem_v (&sem);
	//rt_sem_broadcast (&sem);
    	rt_task_sleep(10000);	
    }
}

void taskTwo(void *arg)
{
    int i;

    for (i=0; i < ITER; i++)
    {
	// this can also be if TM_INIFINITE is not used
	//while(rt_sem_p (&sem, TM_NONBLOCK) == -EWOULDBLOCK){
	//	  rt_task_sleep(100000000);      	
        //}

	rt_sem_p (&sem, TM_INFINITE);	
	rt_printf("I am taskTwo and global = %d----------------\n", --global);
	rt_sem_v (&sem);
	//rt_sem_broadcast (&sem);
        rt_task_sleep(10000); 	    
    }
}

int main(int argc, char* argv[])
{

    /* Perform auto-init of rt_print buffers if the task doesn't do so */
    rt_print_auto_init(1);

    /* Avoids memory swapping for this program */
    mlockall(MCL_CURRENT|MCL_FUTURE);

    // create the semphore	
    rt_sem_create (&sem, "semphore 1", 1, S_FIFO);	

    /* create the two tasks */
    rt_task_create(&t1, "task1", 0, 1, T_JOINABLE);
    rt_task_create(&t2, "task2", 0, 1, T_JOINABLE);

    /* start the two tasks */
    rt_task_start(&t1, &taskOne, 0);
    // just to make sure that task one doesnt get scheduled before 
    // task zero in any case.
    rt_task_sleep(100);
    rt_task_start(&t2, &taskTwo, 0);

   
    rt_task_join(&t1);
    rt_task_join(&t2);

    

    return 0;
}
