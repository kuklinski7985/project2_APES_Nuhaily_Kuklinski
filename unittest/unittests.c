#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "logger.h"
#include "sync_fileio.h"
#include "comm.h"
#include "main.h"
#include "ipc_messq.h"

static void test_pass(void **state)
{
  assert_true(1);
}

static void test_fileCreate(void **state)
{
  file_t test_file;
  int ret = 0;

  strcpy(test_file.filename, "unittest.log");
  ret = fileCreate(&test_file);

  fileClose(&test_file);

  assert_true(ret != -1);
}

static void test_fileCreate_fail(void **state)
{
  file_t test_file;
  int ret = 0;

  strcpy(test_file.filename, "");
  ret = fileCreate(&test_file);

  fileClose(&test_file);
  
  assert_true(ret == -1);
}

// Also tests read
static void test_fileWrite(void **state)
{
  file_t test_file;
  char test_char[1];
  strcpy(test_char, "#");
  char ret;
  int n = 0;

  strcpy(test_file.filename, "unittest.log");
  fileCreate(&test_file);
  fileWrite(&test_file, test_char);
  fileOpen(&test_file);

  ret = fileRead(&test_file);


  fileClose(&test_file);
  assert_true(ret == test_char[0]);
    
}

static void test_fileWrite_fail(void **state)
{
  file_t test_file;
  char test_char[1] = "#";
  

  strcpy(test_file.filename, "");

  assert_true(fileWrite(&test_file, test_char) == -1);

}

// Also tests write
static void test_fileRead(void **state)
{
  file_t test_file;
  char test_char[1];
  strcpy(test_char, "#");
  char ret;
  int n = 0;

  strcpy(test_file.filename, "unittest.log");
  fileCreate(&test_file);
  fileWrite(&test_file, test_char);
  fileOpen(&test_file);

  ret = fileRead(&test_file);

  fileClose(&test_file);
  assert_true(ret == test_char[0]);
    
}

static void test_fileRead_fail(void **state)
{
  file_t test_file;
  char test_char[1] = "#";
  

  strcpy(test_file.filename, "");

  assert_true(fileRead(&test_file) == -1);

}

static void test_getCurrentTimeStr(void **state)
{
  time_t current_time;
  struct tm* current_time_tm;
  char time_str[13];
  current_time = time(0);
  current_time_tm = localtime(&current_time);
  // get current time and compare with output of test function
  sprintf(time_str, "%02d:%02d:%02d > ", current_time_tm->tm_hour,\
   current_time_tm->tm_min, current_time_tm->tm_sec);
  
  assert_int_equal( strcmp(time_str, getCurrentTimeStr()), 0 );
}

static void test_thread_sprintf(void **state)
{
  char s1[2];
  char s2[2];
  sprintf(s1, "%d", 56);
  thread_sprintf(s2, 56, "%d");

  assert_int_equal( strcmp(s1, s2), 0 );
}

static void test_build_ipc_msg(void **state)
{
  char ipc_str[DEFAULT_BUF_SIZE];
  char test_str[DEFAULT_BUF_SIZE];
  ipcmessage_t ipc_struct;
  
  strcpy(test_str, "1\n2\n3\n4\n5\n6\n");
  strcpy(ipc_struct.timestamp, "1");
  ipc_struct.type = 2;
  ipc_struct.source = 3;
  ipc_struct.src_pid = (pid_t)4;
  ipc_struct.destination = 5;
  strcpy(ipc_struct.payload, "6");

  build_ipc_msg(ipc_struct, ipc_str);

  assert_int_equal(strcmp(ipc_str, test_str), 0);

}

static void test_decipher_ipc_msg(void **state)
{
  char ipc_str[DEFAULT_BUF_SIZE];
  ipcmessage_t ipc_struct;
  ipcmessage_t test_struct;
  char test;
  int ret;

  strcpy(ipc_str, "1\n2\n3\n4\n5\n6\n");
  strcpy(test_struct.timestamp, "1");
  test_struct.type = 2;
  test_struct.source = 3;
  test_struct.src_pid = (pid_t)4;
  test_struct.destination = 5;
  strcpy(test_struct.payload, "6");

  decipher_ipc_msg(ipc_str, &ipc_struct);

  ret = (strcmp(test_struct.timestamp, ipc_struct.timestamp) == 0);
  
  ret &= (test_struct.type == ipc_struct.type);
  
  ret &= (test_struct.source == ipc_struct.source);
  
  ret &= (test_struct.src_pid == ipc_struct.src_pid);
  
  ret &= (test_struct.destination == ipc_struct.destination);
  
  ret &= ( strcmp(test_struct.payload, ipc_struct.payload) == 0 );
  
  assert_int_equal(ret, 1);
}

int main(void)
{
	const struct CMUnitTest tests[] = {
    cmocka_unit_test(test_pass),
    cmocka_unit_test(test_fileCreate),
    cmocka_unit_test(test_fileCreate_fail),
    cmocka_unit_test(test_fileWrite),
    cmocka_unit_test(test_fileWrite_fail),
    cmocka_unit_test(test_fileRead),
    cmocka_unit_test(test_fileRead_fail),
    cmocka_unit_test(test_getCurrentTimeStr),
    cmocka_unit_test(test_thread_sprintf),
    cmocka_unit_test(test_build_ipc_msg),
    cmocka_unit_test(test_decipher_ipc_msg),
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}