/**
* @file i2c_wrapper.c
* @brief wrapper functions definitions for the the i2c read and write commands.
* mutex functionality
* @author Andrew Kuklinski
* @date 03/11/2018
**/

#include "i2c_wrapper.h"

//extern int i2ctarget;

/**
 * @brief Read from i2c buffer (fd)
 * 
 * @param fd 
 * @param buff 
 * @param count 
 * @return int 
 */
int i2c_read(int fd, char* buff, size_t count)
{
  int ret;
  ret = read(fd, buff, count);
  if(ret != count)
  {
    //printf("I2C read error. Requested bytes = %d, returned = %d\
\n", count, ret);
    return -1;
  }
  if(ret < 0)
  {
    printf("read failed\n");
    return -1;
  }

  return 0;
}

/**
 * @brief Write to i2c buffer (fd)
 * 
 * @param fd 
 * @param buff 
 * @param count 
 * @return int 
 */
int i2c_write(int fd, char * buff, size_t count)
{
  int ret;
  ret = write(fd, buff, count);
  if(ret != count)
    {
      //printf("I2C write error. Requested bytes = %d, returned = \
%d\n", count, ret);
      return -1;
    }
  if(ret < 0)
    {
      printf("write failed\n");
      return -1;
    }

  return 0;

}

/**
 * @brief Initialize i2c stream for i2c address and return handle.
 * 
 * @param filepath 
 * @param addr 
 * @return int 
 */
int i2c_init(char * filepath, int addr)
{
  int i2ctarget;

  if((i2ctarget = open(filepath, O_RDWR)) < 0)
    {
      printf("Could not open I2C bus!\n");
      return -1;
    }

  if(ioctl(i2ctarget, I2C_SLAVE, addr) < 0)
    {
      printf("Fail: bus access / talk to slave\n");
      return -1;
    }

  return i2ctarget;
}
