#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>

#include <native/task.h>
#include <native/timer.h>
#include <native/sem.h>
#include <native/mutex.h>
#include <native/pipe.h>


#include  <rtdk.h>

#define NTASKS 2

#define QUEUE_SIZE 255
#define MAX_MESSAGE_LENGTH 40

RT_TASK task_struct[NTASKS];

#define PIPE_SIZE 255
RT_PIPE mypipe;

void taskOne(void *arg)
{
    int retval;
    char msgBuf[MAX_MESSAGE_LENGTH];
    char message[] = "Message from taskOne";

    //task message blocks
    RT_TASK_MCB mymcb, talk_reply;

    // this is to debug which task started first
    rt_printf("Entered Task one\n");
	
    mymcb.data = message;
    mymcb.size = sizeof(message);

    talk_reply.size = 0;
    talk_reply.data = NULL;

    retval = rt_task_send(&task_struct[1], &mymcb, &talk_reply, TM_NONBLOCK);
    if (retval == -EWOULDBLOCK ) {
       rt_printf("Would block error: %d\n", retval);
    } 
    else if(retval == -ETIMEDOUT){
	rt_printf("Timedout error: %d\n", retval);
    }
    else if(retval == -ENOBUFS){
	rt_printf("unblocked called error: %d\n", retval);
    }
    else if(retval == -EIDRM){
	rt_printf("sleep error: %d\n", retval);
    }
    else if(retval == -ESRCH){
    	rt_printf("Buffer error: %d\n", retval);
    }  
    else {
       rt_printf("taskOne sent message to mailbox\n");
    }
}

void taskTwo(void *arg)
{
    int retval;
    char msgBuf[MAX_MESSAGE_LENGTH];
    char message[] = "Message from taskTwo";
    RT_TASK_MCB mymcb, listen_reply;
  
    mymcb.data = (caddr_t)msgBuf;
    mymcb.size= sizeof(msgBuf);\

    // this is for debug
    rt_printf("Entered Task two\n");

    /* receive message */
    retval = rt_task_receive(&mymcb,TM_INFINITE); 	

    //error checks
    if (retval == -EWOULDBLOCK ) {
       rt_printf("Would block error: %d\n", retval);
    } 
    else if(retval == -ETIMEDOUT){
	rt_printf("Timedout error: %d\n", retval);
    }
    else if(retval == -ENOBUFS){
	rt_printf("unblocked called error: %d\n", retval);
    }
    else if(retval == -EIDRM){
	rt_printf("sleep error: %d\n", retval);
    }
    else if(retval == -ESRCH){
    	rt_printf("Buffer error: %d\n", retval);
    }    
    else if (retval < 0 ) {
          rt_printf("Receiving error\n");
    } else {
        rt_printf("taskTwo received message: %s\n",mymcb.data);
        rt_printf("with length %d\n",retval);
    }

    listen_reply.size = 0;
    listen_reply.data = NULL;
    rt_task_reply(retval, &listen_reply);

    	
}

//startup code
void startup()
{
  int i;
  char  str[10] ;

  void (*task_func[NTASKS]) (void *arg);
  task_func[0]=taskOne;
  task_func[1]=taskTwo;

  rt_pipe_create (&mypipe, "mypipe", 0, PIPE_SIZE)
	
  rt_timer_set_mode(0); // set timer to tick in nanoseconds and not in jiffies
  for(i=0; i < NTASKS; i++) {
    rt_printf("create task  : %d\n",i);
    sprintf(str,"task%d",i);
    rt_task_create(&task_struct[i], str, 0, 50, 0);
  }

  for(i=1; i > -1 ; i--) {
    rt_printf("start task  : %d\n",i);
    sprintf(str,"task%d",i);
    rt_task_start(&task_struct[i], task_func[i], &i);
  }	  
  
}

void init_xenomai() {
  /* Avoids memory swapping for this program */
  mlockall(MCL_CURRENT|MCL_FUTURE);

  /* Perform auto-init of rt_print buffers if the task doesn't do so */
  rt_print_auto_init(1);
}

int main(int argc, char* argv[])
{
  printf("\nType CTRL-C to end this program\n\n" );

  // code to set things to run xenomai
  init_xenomai();

  //startup code
  startup();

  pause();
}
