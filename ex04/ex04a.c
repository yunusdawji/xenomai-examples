/* ex04a */
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>

#include <native/task.h>
#include <native/timer.h>

#include  <rtdk.h>

#include <sys/io.h>

RT_TASK sound_task;

/* time to toggle sound speaker
   two times this time is the period of the sound wave generated
   (3 periods : -_-_-_ )
 */

RTIME sound_period_ns = 1e5;    /* in nanoseconds, -> 0.1 ms -  10 kHz */

#define SOUND_PORT 0x61        /* address of speaker */
#define SOUND_MASK 0x02        /* bit to set/clear */

RT_TASK demo_task;

 /* sound_function() toggles the speaker port */
 void sound_function(void *arg)
 {
    volatile unsigned char sound_byte;
    unsigned char toggle = 0;
    //int errn=0;

    /* allow access to sound port */
    ioperm(SOUND_PORT, 1, 1) ;
    ioperm(0x80, 1, 1) ;  // to enable _p version of inb and outb

    ioperm(0x43, 1, 1) ;   // PIT timer - set mode
    ioperm(0x42, 1, 1) ;   // PIT timer - set counter

     /* bit0
         0 : raw programming through manual toggling bit1 (PIT doesn't toggle bit1)          -> doesn't work in vmware!!
         1 : PIT programming : bit1 toggled by PIT  (note: manual toggling also still works)
      hack : -> only works on vmware -> doesn't work on real hardware (hack stops sound on real hardware!!)
        instead of manual toggling only bit0=0 which doesn't work in vmware
        we programming PIT (bit0=1) to do blockwave with count=1 -> highest frequency
        and toggle bit1  to toggle to high frequency on and of with a lower frequency
    */

    sound_byte = inb_p(SOUND_PORT);

    /* next code line we should use for manual toggling,
        however this only works on real hardware and not on vmware
        // disable bit 0  -> disable GATE 2
        // -> disable timer 2 (out=high when disabled)
        sound_byte = sound_byte &  ~0x01;
        instead we use:
     */
    // set bit 0 : enable timer 2 on PIT:
    // set GATE2=1 on PIT : set bit 0 on 0x61 to 1
    sound_byte = sound_byte | 0x01;

    // program timer with highest frequency
    // (resets to original value before it was changed by the pcspkr module)
    outb_p(0xB6, 0x43);   // enable PIT timer: square wave on timer 2
    outb_p(0x01, 0x42);   //  with counter=1
    outb_p(0x00, 0x42);
    outb_p(sound_byte, SOUND_PORT);

    sound_byte = inb_p(SOUND_PORT);
    rt_printf("  sound period : %e ns\n", sound_period_ns*2.0 );
    rt_printf("  sound freq   : %.0f Hz\n",1e9/(sound_period_ns*2.0));

    // make this task run periodic with period sound_period_ns
    rt_task_set_periodic(NULL, TM_NOW, sound_period_ns);
    while (1) {
        // Toggle the sound port
        sound_byte = inb_p(SOUND_PORT);

        if (toggle) {
          sound_byte = sound_byte | SOUND_MASK;
        } else {
          sound_byte = sound_byte & ~SOUND_MASK;
        }
        outb_p(sound_byte, SOUND_PORT);
        toggle = ! toggle;

        // let this executing task wait till its next period
        rt_task_wait_period(NULL);
    }
   /* we'll never get here */
   return;
 }

//startup code
void startup()
{
  rt_task_create(&sound_task, "sound", 0, 99, 0);
  rt_task_start(&sound_task, &sound_function, NULL);
}

void init_xenomai() {
 	/* Avoids memory swapping for this program */
	mlockall(MCL_CURRENT|MCL_FUTURE);

	/* Perform auto-init of rt_print buffers if the task doesn't do so */
	rt_print_auto_init(1);
}

// define an empty signal handler
void catch_signal(int sig) {
}

/* catch signals, instead of relying on default OS signal handlers
   the default os behavior is to end program immediately, however
   we install an empty signal handler to allow execution of our
   cleanup code after the signal handler
 */
void wait_for_ctrl_c() {
	signal(SIGTERM, catch_signal);
	signal(SIGINT, catch_signal);
  // wait for SIGINT (CTRL-C) or SIGTERM signal
	pause();
}

//cleanup code
void cleanup()
{
    unsigned char sound_byte;

    rt_task_delete(&sound_task);

    ioperm(SOUND_PORT, 1, 1) ;
    ioperm(0x80, 1, 1) ;  // to enable _p version of inb and outb
    sound_byte = inb_p(SOUND_PORT);
    rt_printf("  sound_byte  : %u\n",sound_byte);
    sound_byte = sound_byte & ~0x01;   // disable bit 0 to disable sound
    outb_p(sound_byte, SOUND_PORT);
}

int main(int argc, char* argv[])
{
  printf("\nType CTRL-C to end this program\n\n" );

  // code to set things to run xenomai
  init_xenomai();

  //startup code
  startup();

  // wait for CTRL-c is typed
  wait_for_ctrl_c();

  //cleanup code
  cleanup();

  // under normal operation you should see the next line at ending of program
  printf("\n\nEnding program\n\n");
}
