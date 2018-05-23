/**********************************************************************/
/*                                                                    */
/* Program Name: Scheduler.c                                          */
/* Author:       David Norris                                         */
/* Installation: Pensacola Christian College, Pensacola, Florida      */ 
/* Cousre:       CS326 - Operating Systems                            */
/* Data Written: March 17, 2017                                       */
/*                                                                    */
/**********************************************************************/

/**********************************************************************/
/*                                                                    */
/* I pledge  the C language  statements in  this  program are  my own */
/* original  work,  and none  of the  C language  statements in  this */
/* program were copied  from any one else,  unless I was specifically */
/* authorized to do so by my CS326 instructor.                        */
/*                                                                    */
/*                                                                    */
/*                           Signed: ________________________________ */
/*                                             (signature)            */
/*                                                                    */
/**********************************************************************/

/**********************************************************************/
/*                                                                    */
/* This program simulates the environment in Unix where new           */
/* processes are continually arriving, existing processes are vying   */
/* for the CPU, processes are using their given quantum (CPU bound)   */
/* or blocking because of I/O operations, and processes are           */
/* terminating when their work is finished.                           */
/*                                                                    */
/**********************************************************************/

#include <stdio.h>
#include <stdlib.h>

/**********************************************************************/
/*                         Symbolic Constants                         */
/**********************************************************************/
#define TABLE_BEGIN     MIN_PROC - 1 /* Start of process table        */
#define TABLE_END       MAX_PROC + 1 /* End of process table          */
#define MIN_PROC        1            /* Minimum number of processes   */
#define MAX_PROC        100          /* Maximum number of processes   */
#define BEGIN_PROCESSES 5            /* Starting number of processes  */
#define TABLE_MAX       9            /* Maximum processes that can be */
                                     /* on the table at a time        */
#define RANDOM_MAX_TIME 18           /* Range for the maximum time    */
#define RANDOM_BLK_TIME 5            /* Range for 2/3 of the blocks   */
#define RANDOM_CREATE   5            /* Range to create new proess    */

/**********************************************************************/
/*                         Program Structure                          */
/**********************************************************************/
/* A process' information                                             */
struct process
{
   int  pid,                       /* Process identification number   */
        cpu_used,                  /* Amount of cpu used              */
        max_time;                  /* Maximum time a process can run  */
   char state;                     /* State of the process            */     
   int  pri,                       /* Priority of the process         */
        quantum_used,              /* Quantum used when process runs  */
        block_time,                /* Tell if the process is CPU bound*/
        wait_ticks;                /* Amount of time the process waits*/
   struct process *p_next_process; /* Points to the next process      */
};
typedef struct process PROCESS;

/**********************************************************************/
/*                        Function Prototypes                         */
/**********************************************************************/
PROCESS *create_table();
   /* Create a table to hold processes                                */
void initialize_table(PROCESS *p_process_table, int *next_pid, 
                                                int number_of_processes);
   /* Create the stating processes                                    */
void create_process(PROCESS *p_process_table, int *next_pid, 
                                              int number_of_processes);
   /* Create a new process                                            */
void print_before(PROCESS *p_process_table, int next_pid, 
                                            int number_of_processes);
   /* Print the before heading and processes                          */
void print_after(PROCESS *p_process_table, int next_pid, 
                                           int number_of_processes);
   /* Print the after heading and processes                           */
void print_processes(PROCESS *p_process_table);
   /* Print the processes and their information                       */
int get_number_of_processes(PROCESS *p_process_table);
   /* Get the number of processes on the table                        */
void sort_pid(PROCESS *p_process_table, int number_of_processes);
   /* Sort the processes by process identification number             */
void sort_pri(PROCESS *p_process_table, int number_of_processes);
   /* Sort the processes by process priority                          */
void calculate_process(PROCESS *p_process_table);
   /* Set the process' cpu_used, quantum_used, and wait_ticks         */
void terminate_process(PROCESS *p_process_table);
   /* Delete a the current running process if cpu_used equals max_time*/
void schedule_process(PROCESS *p_process_table, int number_of_processes);
   /* Set a new process to run                                        */
void set_to_block(PROCESS *p_current_process);
   /* Set the state of the process to blocked based on block_time     */
void set_pri(PROCESS *p_current_process);
   /* Set the priority of the process based on the state              */
void reset_quantum(PROCESS *p_process_table, int number_of_processes);
   /* Set the quantum back to 0 after process runs                    */
void unblock_process(PROCESS *p_process_table, int number_of_processes);
   /* Give a changes to blocked processes to unblock                  */
void all_blocked (PROCESS *p_process_table, int next_pid,
                                            int number_of_processes);
   /* Checks to see if all processes are blocked and print message    */
void empty_table (PROCESS *p_process_table, int next_pid,
                                            int number_of_processes);
   /* Check for empty table                                           */
   
/**********************************************************************/
/*                           Main Function                            */
/**********************************************************************/
int main()
{
   int     next_pid = 1;         /* The ID number of the process      */
   PROCESS *p_process_table,     /* Points to the table of process    */
           *p_begin_process,     /* Points to the begin of the table  */
           *p_current_process;   /* Points process that is runNing    */
   
   p_process_table = create_table();
   p_begin_process = p_process_table;
   p_current_process = p_process_table;
   initialize_table(p_process_table, &next_pid, 
                    get_number_of_processes(p_process_table));

   /* Loop processing process table until the process id is 100       */
   while (next_pid <= MAX_PROC)
   {
      all_blocked(p_process_table, next_pid, 
                  get_number_of_processes(p_process_table));
      empty_table(p_process_table, next_pid, 
                  get_number_of_processes(p_process_table));
                  
      sort_pid(p_process_table, 
               get_number_of_processes(p_process_table));
               
      /* Loop processing process table until next process is runNing  */
      while (p_process_table->p_next_process->state =='B' || 
             p_process_table->p_next_process->state =='R')
      {
         p_process_table = p_process_table->p_next_process;
      }
      
      p_current_process = p_process_table->p_next_process;
      p_process_table   = p_begin_process;
      
      if (p_current_process->cpu_used == p_current_process->max_time)
      {
         print_before(p_process_table, next_pid, 
                      get_number_of_processes(p_process_table));
         terminate_process(p_process_table);
         schedule_process(p_process_table, 
                          get_number_of_processes(p_process_table));
         print_after(p_process_table, next_pid, 
                     get_number_of_processes(p_process_table));
      }
      else
      {
         if (p_current_process->quantum_used == 
             p_current_process->block_time)
         {
            print_before(p_process_table, next_pid, 
                         get_number_of_processes(p_process_table));
            set_to_block(p_current_process);
            set_pri(p_current_process);
            reset_quantum(p_process_table, 
                          get_number_of_processes(p_process_table));
            schedule_process(p_process_table, 
                             get_number_of_processes(p_process_table));
            print_after(p_process_table, next_pid, 
                        get_number_of_processes(p_process_table));
         }
         else
         {
            if (p_current_process->state == 'R')
            {
               print_before(p_process_table, next_pid, 
                            get_number_of_processes(p_process_table));
               schedule_process(p_process_table, 
                                get_number_of_processes(p_process_table));
               print_after(p_process_table, next_pid, 
                           get_number_of_processes(p_process_table));
            }
         }
      }
      
      calculate_process(p_process_table);
      unblock_process(p_process_table, 
                      get_number_of_processes(p_process_table));
      
      if (!(rand() % RANDOM_CREATE))
         create_process(p_process_table, &next_pid, 
                        get_number_of_processes(p_process_table));
   }
    
   return 0;
}

/**********************************************************************/
/*       Create a empty table with a Table Begin and Table End        */
/**********************************************************************/
PROCESS *create_table()
{  
   PROCESS *p_new_table; /* Point to a new table                      */
   
   if ((p_new_table = (PROCESS *) malloc(sizeof(PROCESS))) == NULL)
   {
      printf("The program is aborting.");
      exit(0);
   }
   
   p_new_table->pid = TABLE_BEGIN;
   
   if ((p_new_table->p_next_process = 
       (PROCESS *) malloc(sizeof(PROCESS))) == NULL)
   {
      printf("The program is aborting.");
      exit(0);
   }
   
   p_new_table->p_next_process->pid            = TABLE_END;
   p_new_table->p_next_process->p_next_process = NULL;
   
   return p_new_table;
}

/**********************************************************************/
/*                     Create the begin processes                     */
/**********************************************************************/
void initialize_table(PROCESS *p_process_table, int *next_pid, 
                                                int number_of_processes)
{
   int count; /* Number of processes at the beginning                 */
   
   /* Loop processing process table for five processes                */
   for (count = 0; count < BEGIN_PROCESSES; count++)
      create_process(p_process_table, next_pid, 
                     get_number_of_processes(p_process_table));
   
   return;
}

/**********************************************************************/
/*                      Create a single process                       */
/**********************************************************************/
void create_process(PROCESS *p_process_table, int *next_pid, 
                                              int number_of_processes)
{
   int process_count     ; /* Number of process on the table          */
   PROCESS *p_new_process; /* Point to a new process                  */
   
   process_count = get_number_of_processes(p_process_table);
   if (process_count <= TABLE_MAX)
   {
      if ((p_new_process = (PROCESS *) malloc(sizeof(PROCESS))) == NULL)
      {
         printf("The program is aborting.");
         exit(0);
      }
      
      p_new_process   ->p_next_process = p_process_table->p_next_process;
      p_process_table ->p_next_process = p_new_process;
      p_new_process   ->pid            = *next_pid;
      p_new_process   ->cpu_used       = 0;
      p_new_process   ->max_time       = rand() % RANDOM_MAX_TIME + 1;
      p_new_process   ->state          = 'R';
      p_new_process   ->pri            = 0;
      p_new_process   ->quantum_used   = 0;
      if (rand() % 3)
         p_new_process->block_time     = rand() % RANDOM_BLK_TIME + 1;
      else
         p_new_process->block_time     = 6;
      p_new_process   ->wait_ticks     = 0;
      
      (*next_pid)++;
   }
   
   return;
}

/**********************************************************************/
/*                      Print the before heading                      */
/**********************************************************************/
void print_before(PROCESS *p_process_table, int next_pid, 
                                            int number_of_processes)
{
   int process_number; /* Number of process on the table              */
   
   process_number = get_number_of_processes(p_process_table);
   
   printf("\n\n BEFORE SCHEDULING CPU:  Next PID = %2d,  ", next_pid);
   printf(     "Number of Processes = %2d", process_number);
   printf(  "\n PID   CPU Used   MAX Time   STATE   PRI   ");
   printf(     "QUANTUM USED   BLK TIME   WAIT TKS");
   
   sort_pid(p_process_table, get_number_of_processes(p_process_table));
   print_processes(p_process_table);
   
   return;
}

/**********************************************************************/
/*                      Print the after heading                       */
/**********************************************************************/
void print_after(PROCESS *p_process_table, int next_pid, 
                                           int number_of_processes)
{  
   int process_number; /* Number of process on the table              */
   
   process_number = get_number_of_processes(p_process_table);
   
   printf("\n\n AFTER SCHEDULING CPU:  Next PID = %2d,  ", next_pid);
   printf(     "Number of Processes = %2d", process_number);
   printf(  "\n PID   CPU Used   MAX Time   STATE   PRI   ");
   printf(     "QUANTUM USED   BLK TIME   WAIT TKS");
   
   sort_pid(p_process_table, get_number_of_processes(p_process_table));
   print_processes(p_process_table);
   
   return;
}

/**********************************************************************/
/*                  Print the processes on the table                  */
/**********************************************************************/
void print_processes(PROCESS *p_process_table)
{
   if (p_process_table->p_next_process->pid != TABLE_END)
   {
      /* Loop processing processes until Table End                    */
      while (p_process_table       = p_process_table->p_next_process, 
             p_process_table->pid != TABLE_END)
      {
         printf("\n%4d %7d %10d %8c %7d %9d %12d %11d", 
         p_process_table->pid,      
         p_process_table->cpu_used,
         p_process_table->max_time, 
         p_process_table->state,
         p_process_table->pri,     
         p_process_table->quantum_used,
         p_process_table->block_time, 
         p_process_table->wait_ticks);
      }
   }
   
   return;
}

/**********************************************************************/
/*               Get the number of process on the table               */
/**********************************************************************/
int get_number_of_processes(PROCESS *p_process_table)
{
   int number_of_processes = 0; /* Number of process on the table     */
   
   /* Loop processing process table until Table End                   */
   while (p_process_table       = p_process_table->p_next_process, 
          p_process_table->pid != TABLE_END)
   {
      number_of_processes += 1;
   }
   
   return number_of_processes;
}

/**********************************************************************/
/*                       Sort processes by PID                        */
/**********************************************************************/
void sort_pid(PROCESS *p_process_table, int number_of_processes)
{
   int     process_count;   /* Number of processes on the table       */
   PROCESS *p_process,      /* Point to the current process           */
           *p_temp_process; /* Point to temporary process for swap    */
   
   /* Loop processing for the number of processes on the table        */   
   for (process_count = 1; process_count <= 
        get_number_of_processes(p_process_table); process_count++)
   {
      p_process = p_process_table;
      
      /* Loop processing process table until Table End                */
      while (p_process->p_next_process->p_next_process->pid != TABLE_END)
      {
         if (p_process->p_next_process->pid > 
             p_process->p_next_process->p_next_process->pid)
         {
            p_temp_process 
               = p_process->p_next_process->p_next_process;
            p_process->p_next_process->p_next_process 
               = p_process->p_next_process->p_next_process->p_next_process;
            p_temp_process->p_next_process 
               = p_process->p_next_process;
            p_process->p_next_process      
               = p_temp_process;
         }
         p_process = p_process->p_next_process;
      }
   }
   
   sort_pri(p_process_table, get_number_of_processes(p_process_table));
   
   return;
}

/**********************************************************************/
/*                     Sort processes by Priority                     */
/**********************************************************************/
void sort_pri(PROCESS *p_process_table, int number_of_processes)
{
   int     process_count;   /* Number of processes on the table       */
   PROCESS *p_process,      /* Point to the current process           */
           *p_temp_process; /* Point to temporary process for swap    */
           
   /* Loop processing for the number of processes on the table        */
   for (process_count = 1; process_count <= 
        get_number_of_processes(p_process_table); process_count++)
   {
      p_process = p_process_table;
      
      /* Loop processing process table until Table End                */
      while (p_process->p_next_process->p_next_process->pid != TABLE_END)
      {
         if (p_process->p_next_process->pri > 
             p_process->p_next_process->p_next_process->pri)
         {
            p_temp_process 
               = p_process->p_next_process->p_next_process;
            p_process->p_next_process->p_next_process 
               = p_process->p_next_process->p_next_process->p_next_process;
            p_temp_process->p_next_process 
               = p_process->p_next_process;
            p_process->p_next_process 
               = p_temp_process;
         }
         p_process = p_process->p_next_process;
      } 
   }
   
   /* Loop processing for the number of processes on the table        */
   for (process_count = 1; process_count <= 
        get_number_of_processes(p_process_table); process_count++)
   {
      p_process = p_process_table;
      
      /* Loop processing process table until Table End                */
      while (p_process->p_next_process->p_next_process->pid != TABLE_END)
      {
         if (p_process->p_next_process->pri                 < 0 && 
             p_process->p_next_process->p_next_process->pri < 0)
         {
            if (p_process->p_next_process->pri < 
                p_process->p_next_process->p_next_process->pri)
            {
               p_temp_process 
                  = p_process->p_next_process->p_next_process;
               p_process->p_next_process->p_next_process 
                  = p_process->p_next_process->p_next_process->p_next_process;
               p_temp_process->p_next_process 
                  = p_process->p_next_process;
               p_process->p_next_process 
                  = p_temp_process;
            }
         }
         p_process = p_process->p_next_process;
      } 
   }
   
   return;
}

/**********************************************************************/
/*           Set the Wait Time, CPU Used, and Quantum Used            */
/**********************************************************************/
void calculate_process(PROCESS *p_process_table)
{
   PROCESS *p_process = p_process_table; /* Point to current process  */
   
   /* Loop processing process table until Table End                   */
   while (p_process->p_next_process->pid != TABLE_END)
   {
      if (p_process->p_next_process->state == 'N')
      {
         p_process->p_next_process->cpu_used += 1;
         p_process->p_next_process->quantum_used += 1;
      }
      else
      {
         if (p_process->p_next_process->state == 'R')
         {
            p_process->p_next_process->wait_ticks += 1;
         }
      }
      p_process = p_process->p_next_process;
   }
    
   return;
}

/**********************************************************************/
/*                         Removes a process                          */
/**********************************************************************/
void terminate_process(PROCESS *p_process_table)
{
   PROCESS *p_previous = p_process_table, 
                                     /* Point to the previous process */
           *p_current = p_process_table->p_next_process;
                                     /* Point to each process         */
   
   /* Loop processing process table until Table End                   */
   while (p_current->pid != TABLE_END)
   {
      if (p_current->cpu_used == p_current->max_time)
      {
         p_previous->p_next_process = p_current->p_next_process;
         free (p_current);
      }
      else
      {
         p_previous = p_current;
      }
      p_current     = p_current->p_next_process;
   }
   
   return;
}

/**********************************************************************/
/*             Schedule the next Ready process to runNing             */
/**********************************************************************/
void schedule_process(PROCESS *p_process_table, int number_of_processes)
{
   int     process_count; /* Number of processes on the table         */
   PROCESS *p_process;    /* Points to current process                */
   
   p_process = p_process_table;
   
   sort_pid(p_process_table, get_number_of_processes(p_process_table));
   
   /* Loop processing process table for number of process on the table*/
   for (process_count = 1; process_count <= 
        get_number_of_processes(p_process_table); process_count++)
   {
      if(p_process->p_next_process->state == 'B')
      {
         p_process = p_process->p_next_process;
      }
      else
      {
         if (p_process->p_next_process->state == 'R')
         {
            p_process->p_next_process->state  = 'N';
            break;
         }
      }
   }
   
   return;
}

/**********************************************************************/
/*                 Set the current prcess to Blocked                  */
/**********************************************************************/
void set_to_block(PROCESS *p_current_process)
{
   if (p_current_process->block_time < 6)
   {
      p_current_process->state = 'B';
   }
   else
   {
      p_current_process->state = 'R';
   }
      
   return;
}

/**********************************************************************/
/*                  Set the priority of the process                   */
/**********************************************************************/
void set_pri(PROCESS *p_current_process)
{
   PROCESS *p_process; /* Point to the current process                */
   
   p_process = p_current_process;
   
   p_process->pri = ((abs(p_process->pri) + 
                    (float)p_process->quantum_used) * .5) + .5;
   
   if (p_process->state == 'B')
   {
      p_process->pri = p_process->pri * -1;
   }
   
   return;
}

/**********************************************************************/
/*                    Reset the Quantum back to 0                     */
/**********************************************************************/
void reset_quantum(PROCESS *p_process_table, int number_of_processes)
{
   int process_count,  /* Number of process on the table              */
       process_number; /* Total number of process on the table        */
   
   process_number =get_number_of_processes(p_process_table);
   
   /* Loop processing process table for number of processes on table  */
   for (process_count = 0; process_count <= 
                           process_number; process_count++)
   {
      p_process_table->p_next_process->quantum_used = 0;
      p_process_table = p_process_table->p_next_process;
   }
   
   return;
}

/**********************************************************************/
/*                    Chance to unblock processes                     */
/**********************************************************************/
void unblock_process(PROCESS *p_process_table, int number_of_processes)
{
   int process_count,  /* Number of process on the table              */
       process_number; /* Total number of process on the table        */
   
   process_number = get_number_of_processes(p_process_table);
   
   /* Loop processing process table for number of processes on table  */
   for (process_count = 0; process_count <= 
                           process_number; process_count++)
   {
      if (p_process_table->p_next_process->state == 'B')
      {
         if (!(rand() % 20))
            p_process_table->p_next_process->state = 'R';
      }
      
      p_process_table = p_process_table->p_next_process;
   }
   
   return;
}

/**********************************************************************/
/*            Special message for table being all blocked             */
/**********************************************************************/
void all_blocked (PROCESS *p_process_table, int next_pid,
                                            int number_of_processes)
{
   int process_count,  /* Number of process on the table              */
       process_number; /* Total number of process on the table        */
   
   process_number = get_number_of_processes(p_process_table);
   
   /* Loop processing process table for number of processes on table  */
   for (process_count = 0; process_count <= 
                           process_number; process_count++)
   {
      if (p_process_table->p_next_process->state == 'B')
      {
         p_process_table = p_process_table->p_next_process;
         if (p_process_table->p_next_process->pid == TABLE_END)
         {
            print_before(p_process_table, next_pid, 
                         get_number_of_processes(p_process_table));
            print_after( p_process_table, next_pid, 
                         get_number_of_processes(p_process_table));
         }
      }
      
   }
   return;
}

/**********************************************************************/
/*                            Empty table                             */
/**********************************************************************/
void empty_table (PROCESS *p_process_table, int next_pid,
                                            int number_of_processes)
{
   int process_count;  /* Number of process on the table              */
   
   process_count = get_number_of_processes(p_process_table);
   
   if (process_count == 0)
   {
      print_before(p_process_table, next_pid, 
                   get_number_of_processes(p_process_table));
      print_after( p_process_table, next_pid, 
                   get_number_of_processes(p_process_table));
   }
   return;
}
