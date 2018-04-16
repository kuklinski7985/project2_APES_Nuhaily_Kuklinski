/**
 *  @file sync_fileio.c
 *  @author Adam Nuhaily and Andrew Kuklinski
 *  @date 17 Mar 2018
 *  @brief file io operations with mutex locking
 *  This source file implements thread-safe file io operations
 */

#include "sync_fileio.h"

pthread_mutex_t file_mutex;

/**
 *  @brief Create new text file, overwrite if exists
 *  @param fp Container for file pointer and other data
 *
 *  @return Function exit status
 */
int8_t fileCreate(file_t* fp)
{
  char newfilename[64];
  strcpy(newfilename, fp->filename);  // we seem to need to copy the contents of
                                  // the filename array to a new array of fixed
                                  // length before calling fopen

  fp->fileptr = fopen(newfilename, FILE_CREATE_NEW);

  if(!(fp->fileptr))
  {
    printf("Error creating file.\n");
    return -1;
  }

  return 0;
}

/**
 *  @brief Write to text file. Close file when done so other threads can open
 *  @param fp Container for file pointer and other data
 *  @param str Text to write to file
 *  @return Function exit status
 */
int8_t fileWrite(file_t* fp, char* str)
{
  pthread_mutex_lock(&file_mutex);

  fp->fileptr = fileOpen(fp);
  fputs(str, fp->fileptr);
  fclose(fp->fileptr);

  pthread_mutex_unlock(&file_mutex);

  return 0;
}

/**
 *  @brief Close file and free memory
 *  @param fp Container for file pointer and other data
 *
 *  @return Function exit status
 */
int8_t fileClose(file_t* fp)
{
  if(!fp->fileptr)
  {
    return -1;  // Invalid file pointer, exit
  }

  pthread_mutex_lock(&file_mutex);

  // Close file reference
  fclose(fp->fileptr);

  pthread_mutex_unlock(&file_mutex);

  return 0;
}

/**
 *  @brief Execute sequence of events called for by problem 2
 *  @param fp Container for file pointer and other data
 *
 *  @return Function exit status
 */
char fileRead(file_t* fp)
{
  if(!fp->fileptr)
  {
    return -1;
  }

  return fgetc(fp->fileptr);
}

/**
 *  @brief Open file in append mode
 *  @param fp Container for file info to open
 *
 *  @return File pointer to opened file
 */
FILE* fileOpen(file_t* fp)
{
  FILE* file_ptr = NULL;
  file_ptr = fopen(fp->filename, FILE_APPEND);


  return file_ptr;
}
