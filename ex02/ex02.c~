#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>


#include <native/task.h>
#include <native/timer.h>
#include  <rtdk.h>

RT_TASK demo_task[5];

void demo(void *arg)

{
rt_task_set_periodic(NULL, TM_NOW, 10^10); 

while(1){
  RT_TASK *curtask;
  RT_TASK_INFO curtaskinfo;

  // hello world
  rt_printf("Hello World!\n");

  // inquire current task
  curtask=rt_task_self();
  rt_task_inquire(curtask,&curtaskinfo);

  // print task name
  rt_printf("Task name : %s \n", curtaskinfo.name);

  rt_task_wait_period(NULL);
}
return;
}

 

int main(int argc, char* argv[])
{
  char  str[5][10] ;

  // Perform auto-init of rt_print buffers if the task doesn't do so
  rt_print_auto_init(1);

  // Lock memory : avoid memory swapping for this program
  mlockall(MCL_CURRENT|MCL_FUTURE);

  rt_printf("start task\n");

  /*
   * Arguments: &task,
   *            name,
   *            stack size (0=default),
   *            priority,
   *            mode (FPU, start suspended, ...)
   */
  sprintf(str[0],"hello");
  sprintf(str[1],"hello1");
  sprintf(str[2],"hello2");
  sprintf(str[3],"hello3");
  sprintf(str[4],"hello4");

  int i = 0;
  for(i = 0; i < 5; i++)
  {
    rt_task_create(&demo_task[i], str[i], 0, 50 + i, 0);
   // rt_task_set_periodic(&demo_task[i],
   //                      TM_NOW,
   //                      (i+1)*10^9);
  }

  /*
   * Arguments: &task,
   *            task function,
   *            function argument
   */
  for(i = 0; i < 5; i++)
    rt_task_start(&demo_task[i], &demo, 0);

   rt_printf("end program by CTRL-C\n");

    pause();

}
