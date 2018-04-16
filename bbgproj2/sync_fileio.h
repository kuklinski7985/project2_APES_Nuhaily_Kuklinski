/**
 *  @file sync_fileio.h
 *  @author Adam Nuhaily
 *  @date 18 Feb 2018
 *  @brief file io operations with mutex locking
 *  This header file implements thread-safe file io operations for use
 *  with the multithreaded program hw3.c
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#define FILE_READ_WRITE   "w"
#define FILE_READ_ONLY    "r"
#define FILE_CREATE_NEW   "w+"
#define FILE_APPEND       "a+"

// Struct containing information about text file for this code
typedef struct {
  FILE * fileptr;
  char filename[64];
} file_t;

// File interface functions
int8_t fileCreate(file_t* fp);
int8_t fileWrite(file_t* fp, char* str);
int8_t getString(file_t* fp);
int8_t fileClose(file_t* fp);
char fileRead(file_t* fp);
FILE* fileOpen(file_t* fp);
