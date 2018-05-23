/* cd ../../mnt/c/Users/David\ Norris/Desktop/School/cs326/driver */
/**********************************************************************/
/*                                                                    */
/* Program Name: CONVERT - Convert Block Number to Cylinder, Track,   */
/*                         and Sector Number                          */
/* Author:       David Norris                                         */
/* Installation: Pensacola Christian College, Pensacola, Florida      */
/* Cousre:       CS326, Operating Systems                             */
/* Data Written: April 7, 2017                                        */
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
/* This program calculates the cylinder, track, and sector number     */
/* using the block number then prints out the block, cylinder, track, */
/* and sector number.                                                 */
/*                                                                    */
/**********************************************************************/

#include <stdio.h>

/**********************************************************************/
/*                         Symbolic Constants                         */
/**********************************************************************/
#define BYTES_PER_BLOCK     1024 /* Number of bytes for one block     */
#define BYTES_PER_SECTOR    512  /* Number of bytes for one sector    */
#define CYLINDERS_PER_DISK  40   /* Number of cylinders for one disk  */
#define TRACKS_PER_CYLINDER 9    /* Number of tracks for one cylinder */
#define SECTORS_PER_TRACK   2    /* Number of sectors for one track   */

/**********************************************************************/
/*                        Function Prototypes                         */
/**********************************************************************/
void convert_block (int block_number, int *p_cylinder, int *p_track, 
                                                       int *p_sector);
   /* Convert block number into cylinder, track, and sector number    */
   
/**********************************************************************/
/*                           Main Function                            */
/**********************************************************************/
int main()
{
   int block,          /* Block number                                */
       blocks_on_disk = (CYLINDERS_PER_DISK * TRACKS_PER_CYLINDER *
                        SECTORS_PER_TRACK * BYTES_PER_SECTOR) /
                        BYTES_PER_BLOCK, 
                       /* Number of blocks per disk                   */
       cylinder,       /* Cylinder number                             */
       sector,         /* Sector number                               */
       track;          /* Track number                                */
                     
   printf("\n Block     Cylinder     Track     Sector");
   printf("\n-------   ----------   -------   --------");
   
   /* Loop processing block until it equals the number of blocks per  */
   /* disk                                                            */
   for(block = 1; block <= blocks_on_disk; block++)
   {
      convert_block(block, &cylinder, &track, &sector);
      printf("\n %3d          %2d          %d         %d", block, 
                                               cylinder, track, sector);
   }
   
   printf("\n\n");
   return 0;
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
      *p_track = 0;
      *p_sector = block_remainder * 2;
   }
   else
   {
      *p_track = 1;
      *p_sector = (block_remainder * 2) - TRACKS_PER_CYLINDER;
   }
   
   return;
}