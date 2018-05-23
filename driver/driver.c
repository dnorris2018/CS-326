/**********************************************************************/
/*                                                                    */
/* Program Name: DRIVER.c - Device driver simulator                   */
/* Author:       David Norris                                         */
/* Installation: Pensacola Christian College, Pensacola, Florida      */
/* Course:       CS326, Operating Systems                             */
/* Date Written: April 11, 2017                                       */
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
/* This program accepts read and write requests from a FILE SYSTEM,   */
/* translates them from physical block numbers into disk drive        */
/* cylinder, track, and sector numbers, then instructs a DISK device  */
/* to carry out the read and write requests.                          */
/*                                                                    */
/**********************************************************************/

#include <stdio.h>

/**********************************************************************/
/*                         Symbolic Constants                         */
/**********************************************************************/
#define BYTES_PER_BLOCK        1024 /* Number of bytes for one block  */
#define BYTES_PER_SECTOR       512  /* Number of bytes for one sector */
#define CYLINDERS_PER_DISK     40   /* Number of cylinders for one    */
                                    /* disk                           */
#define DMA_SETUP              3    /* Set the DMA chip registers for */
                                    /* next read or write operation   */
#define FIRST_INDEX            0    /* First index in fs message      */
#define INVALID_ADDRESS        -16  /* Invalid data address           */
#define INVALID_BLOCK_NUMBER   -4   /* Invalid block number           */
#define INVALID_BLOCK_SIZE     -8   /* Invalid block size             */
#define INVALID_OPERATION_CODE -1   /* Invalid operation code         */
#define INVALID_REQUEST_NUMBER -2   /* Invalid request number         */
#define LOOP_INFINITELY        1    /* Infinity loop                  */
#define MAX_IDLE               2    /* Maximum idle request in a row  */
#define READ_DATA              6    /* Read the data per DMA chip     */
                                    /* values                         */
#define RECALIBRATE            9    /* Reset DISK heads to cylinder   */
                                    /* zero after a seek failure      */
#define REQUEST_MAX            20   /* Maximum number of request      */
#define SECTORS_PER_TRACK      2    /* Number of sectors for one      */
                                    /* track                          */
#define SEEK_TO_CYLINDER       2    /* Send the DISK heads to the     */
                                    /* given cylinder                 */
#define SENSE_CYLINDER         1    /* Sense the DISK heads current   */
                                    /* cylinder position              */
#define START_MOTOR            4    /* Turn the DISK drive motor on   */
#define STATUS_MOTOR           5    /* Wait until the DISK drive      */
                                    /* motor is up to speed           */
#define STOP_MOTOR             8    /* Turn the DISK drive motor off  */
#define TRACKS_PER_CYLINDER    9    /* Number of tracks for one       */
                                    /* cylinder                       */
#define WRITE_DATA             7    /* Write the data per DMA chip    */
                                    /* values                         */

/**********************************************************************/
/*                         Program Structure                          */
/**********************************************************************/
/* Messages information                                               */
struct message
{
                 int operation_code,  /* The disk opertion to be      */
                                      /* performed                    */
                     request_number,  /* A unique request number      */
                     block_number,    /* The block number to be read  */
                                      /* or written                   */
                     block_size;      /* The block size in bytes      */
   unsigned long int *p_data_address; /* Points to the data block in  */
                                      /* memory                       */
};
typedef struct message MESSAGE;

/**********************************************************************/
/*                        Function Prototypes                         */
/**********************************************************************/
void send_message();
   /* Sends and receives messages from the file system                */  
int disk_drive(int code, int arg1, int arg2, int arg3, 
                                             unsigned long int *p_data);
   /* Sends and receives messages from the disk drive                 */
void message_fill();
   /* Fills the message to let the file system know it needs work     */
void copy_fs(int *p_request_count);
   /* Copies fs message into pending_request                          */
void motor_on(int *p_motor_state, int *p_cylinder);
   /* Turns the motor on                                              */
void sort_request(int request_count);
   /* Sorts the request in the pending list                           */
void convert_block (int block_number, int *p_cylinder, int *p_track, 
                                                       int *p_sector);
   /* Converts block number into cylinder, track, and sector number   */
void operation_rw(int index_number);
   /* Reads or writes the pending request                             */
void remove_request(int index_number, int *p_request_count);
   /* Removes the currently finished request                          */
int error_check (int index_number);
   /* Checks for valid requset parameters                             */
void schedule_request(int *p_index_number, int request_count, 
                      int previous_cylinder, int cylinder);
   /* Schedules the next request on the list                          */

/**********************************************************************/
/*                          Global Variables                          */
/**********************************************************************/
MESSAGE fs_message[20];
MESSAGE pending_request[20];

/**********************************************************************/
/*                           Main Function                            */
/**********************************************************************/
int main()
{
   int cylinder,          /* Current cylinder number                  */
       error_code,        /* Code for invalid parameters              */
       idle_count = 1,    /* Counts the number of idle request in a   */
                          /* row                                      */
       index_number = 0,  /* Keeps track of the request its on        */
       motor_state = 0,   /* Keeps off the motor status               */
       previous_cylinder, /* Previous cylinder number                 */
       request_count = 0, /* Keeps track of the number of request     */
       sector,            /* Sector number                            */
       seek_error,        /* Cylinder use for recalibration           */
       track;             /* Track number                             */
   
   /* Fills the request and sends the initial message                 */
   message_fill();
   send_message(fs_message);
   
   /* Loop processing request from file system                        */
   while(LOOP_INFINITELY)
   {
      /* Checks the request and checks that request list for more     */
      /* request then if request is empty and no request on the list  */
      /* then it gets another request                                 */
      if((fs_message[index_number].operation_code == 0) && 
         (request_count == 0))
      {
         /* Checks the idle count then increments idle and fill       */
         /* request                                                   */
         if(idle_count == 1)
         {
            idle_count += 1;
            message_fill();
         }
         else
         {
            /* Turns motor off when two idle request come in from the */
            /* the file system                                        */
            if(idle_count == MAX_IDLE)
            {
               idle_count  += 1;
               motor_state = disk_drive(STOP_MOTOR, 0, 0, 0, 0);
            }
         }
      }
      
      /* Processes the requests                                       */
      else
      {
         /* Copies request from fs message to the pending request     */
         /* list and gets the number of request on the list           */
         copy_fs(&request_count);
         
         /* Turns motor on and waits for it to get up to speed and    */
         /* gets the cylinder it is on                                */
         motor_on(&motor_state, &cylinder);
         
         /* Sets the previous request cylinder                        */
         previous_cylinder = cylinder;
         
         /* Sorts the pending request list in block number order      */
         sort_request(request_count);
         
         /* Schedules the next request to be processed                */
         schedule_request(&index_number, request_count, 
                                         previous_cylinder, cylinder);

         /* Sends message back to file system if any of the current   */
         /* request parameters are invalid                            */
         error_code = error_check(index_number);
         if(error_code != 0)
         {
            pending_request[index_number].operation_code = error_code;
            fs_message[index_number] = pending_request[index_number];
            remove_request(index_number, &request_count);
            idle_count = 1;
         }
         /* No error with the parameters; continues processing the    */
         /* request                                                   */
         else
         {
            /* Converts the current request block number to cylinder, */
            /* track, and sector                                      */
            convert_block(pending_request[index_number].block_number, 
                          &cylinder, &track, &sector);
            
            /* Checks the previous request's cylinder too the current */
            /* request's cylinder if different processes new cylinder */
            if(previous_cylinder != cylinder)
            {
               /* Seeks the new cylinder and checks it against the    */
               /* current request's cylinder                          */
               previous_cylinder = disk_drive(SEEK_TO_CYLINDER, 
                                              cylinder, 0, 0, 0);
               while(previous_cylinder != cylinder)
               { 
                  /* Recalibrates if the seek fails                   */
                  do
                  {
                     seek_error = disk_drive(RECALIBRATE, 0, 0, 0, 0);
                  }while(seek_error != 0);
                  
                  /* Check the current request's cylinder and if not  */
                  /* zero the previous request's cylinder gets seek   */
                  if(cylinder != 0)
                     previous_cylinder = disk_drive(SEEK_TO_CYLINDER, 
                                                    cylinder, 0, 0, 0);
                  else
                     previous_cylinder = cylinder;
               }
            }
            
            /* Sets the DMA chip registers                            */
            disk_drive(DMA_SETUP, sector, track, 
                          pending_request[index_number].block_size, 
                          pending_request[index_number].p_data_address);
         
            /* Processes a read or write command                      */
            operation_rw(index_number);
            
            /* Gets the current request ready to be sent back to the  */
            /* file system and removes it from the pending request    */
            /* list                                                   */
            pending_request[index_number].operation_code = 0;
            fs_message[FIRST_INDEX] = pending_request[index_number];
            remove_request(index_number, &request_count);
            
            /* Resets idle if a process is successfully processed     */
            idle_count = 1;
         }
      }
      /* Sends the currently finish request back the the file system  */
      /* and possibly get more request                                */
      send_message(fs_message);
   }
   return 0;
}

/**********************************************************************/
/*    Fills the message to let the file system know it needs work     */
/**********************************************************************/
void message_fill()
{
   fs_message[0].operation_code = 0;
   fs_message[0].request_number = 0;
   fs_message[0].block_number   = 0;
   fs_message[0].block_size     = 0;
   fs_message[0].p_data_address = NULL;
   return;
}

/**********************************************************************/
/*               Copies fs message into pending request               */
/**********************************************************************/
void copy_fs(int *p_request_count)
{
   int index = 0;
   
   /* Loop processing copying fs message to pending request list and  */
   /* counting the request                                            */
   while((fs_message[index].operation_code != 0) && 
        (*p_request_count < REQUEST_MAX))
   {
      pending_request[*p_request_count] = fs_message[index];
      *p_request_count += 1;
      index += 1;
   }
   return;
}

/**********************************************************************/
/*                         Turns the motor on                         */
/**********************************************************************/
void motor_on(int *p_motor_state, int *p_cylinder)
{
   if(*p_motor_state == 0)
   {
      *p_motor_state = disk_drive(START_MOTOR, 0, 0, 0, 0);
      disk_drive(STATUS_MOTOR, 0, 0, 0, 0);
      *p_cylinder = disk_drive(SENSE_CYLINDER, 0, 0, 0, 0);
   }
   return;
}

/**********************************************************************/
/*               Sorts the request in the pending list                */
/**********************************************************************/
void sort_request(int request_count)
{
   int index,            /* Used to swap the request                  */
       count;            /* Count the request                         */
   MESSAGE temp_request; /* Temporary structure for swap              */
   
   /* Loop processing sorting the request in block number order       */
   for(count = 1; count < request_count; count++)
   {
      temp_request = pending_request[count];
      index        = count - 1;
      while((index >= 0) && 
      (pending_request[index].block_number > temp_request.block_number))
      {
         pending_request[index + 1] = pending_request[index];
         index                      -= 1;
      }
      pending_request[index + 1]    = temp_request;
   } 
   return;
}

/**********************************************************************/
/*               Schedules the next request on the list               */
/**********************************************************************/
void schedule_request(int *p_index_number, int request_count, 
                      int previous_cylinder, int cylinder)
{  
   *p_index_number = 0;
   
   /* Loop processing to next request                                 */
   while(((cylinder = 
          (int)(pending_request[*p_index_number].block_number - 1) / 
          TRACKS_PER_CYLINDER) < previous_cylinder) && 
          (*p_index_number < request_count))
   {
      *p_index_number += 1;
   }
   
   /* Checks the number of request to the current request index       */
   if(*p_index_number == request_count)
      *p_index_number = 0;
   return;
}

/**********************************************************************/
/*                Checks for valid requset parameters                 */
/**********************************************************************/
int error_check (int index_number)
{
   int op_code = 0, /* Number of invalid parameters                   */
       bytes_per_disk = 
       BYTES_PER_SECTOR * SECTORS_PER_TRACK * TRACKS_PER_CYLINDER;
                    /* Number of bytes per disk                       */
   
   /* Checks the operation code to be valid                           */
   if(pending_request[index_number].operation_code <= 0 || 
      pending_request[index_number].operation_code > 2)
      op_code += INVALID_OPERATION_CODE;
   
   /* Checks the request umber to be valid                            */
   if(pending_request[index_number].request_number <= 0)
      op_code += INVALID_REQUEST_NUMBER;
   
   /* Checks the block number to be valid                             */
   if(pending_request[index_number].block_number < 1 || 
      pending_request[index_number].block_number > 360)
      op_code += INVALID_BLOCK_NUMBER;
   
   /* Checks the block size to be valid                               */
   if(pending_request[index_number].block_size < 0 || 
     (pending_request[index_number].block_size & 
     (pending_request[index_number].block_size - 1)) != 0 || 
     pending_request[index_number].block_size > bytes_per_disk)
      op_code += INVALID_BLOCK_SIZE;
   
   /* Checks the data to be read or written to be valid               */
   if(pending_request[index_number].p_data_address < 0)
      op_code += INVALID_ADDRESS;
   
   return op_code;
}

/**********************************************************************/
/*                      Convert the block number                      */
/**********************************************************************/
void convert_block (int block_number, int *p_cylinder, int *p_track, 
                                                       int *p_sector)
{
   int block_remainder = (block_number - 1) % TRACKS_PER_CYLINDER; 
                                   /* Block remainder for calculation */
   
   /* Calculate cylinder number                                       */
   *p_cylinder = (int)(block_number - 1) / TRACKS_PER_CYLINDER;

   /* Calculate track and sector number                               */
   if (block_remainder <= (int) (TRACKS_PER_CYLINDER * .5f))
   {
      *p_track  = 0;
      *p_sector = block_remainder * 2;
   }
   else
   {
      *p_track  = 1;
      *p_sector = (block_remainder * 2) - TRACKS_PER_CYLINDER;
   } 
   return;
}

/**********************************************************************/
/*                Reads or writes the pending request                 */
/**********************************************************************/
void operation_rw(int index_number)
{
   int read_status,  /* Keeps track if the request is a read          */
       write_status; /* Keeps track if the request is a write         */
       
   /* Checks the request to be a read                                 */
   if (pending_request[index_number].operation_code == 1)
   {
      read_status = disk_drive(READ_DATA, 0, 0, 0, 0);
      /* Loop processing while read is not done                       */
      while (read_status != 0)
      {
         read_status = disk_drive(READ_DATA, 0, 0, 0, 0);
      }
   }
   else
   {
      /*Checks the request to be a write                              */
      if (pending_request[index_number].operation_code == 2)
      {
         write_status = disk_drive(WRITE_DATA, 0, 0, 0, 0);
         /* Loop processing while write is not done                   */
         while (write_status != 0)
         {
            write_status = disk_drive(WRITE_DATA, 0, 0, 0, 0);
         }
      }    
   }  
   return;
}

/**********************************************************************/
/*               Removes the currently finished request               */
/**********************************************************************/
void remove_request(int index_number, int *p_request_count)
{
   int index; /* Number of the request that needs to be removed       */
   
   /* Loop processing remove currently finished request               */
   for(index = index_number; index < REQUEST_MAX; index++)
   {
      pending_request[index] = pending_request[index + 1];
   }  
   *p_request_count          -= 1;
   return;
}