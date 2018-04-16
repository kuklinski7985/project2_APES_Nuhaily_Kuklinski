/**
* @file myusrled.c
* @brief fxn definitions for interacting with USR LED's
* @author Andrew Kuklinski and Adam Nuhaily
* @date 03/11/2018
**/

#include <stdio.h>
#include <stdlib.h>
#include "myusrled.h"

int usr_led_toggle(int lednum, int togvalue)
{
  FILE * ledfile;

  if(togvalue > 1)
    {
      printf("Only values of 1 or 0: 0=OFF | 1=ON\n");
      return -1;
    }
  switch(lednum){
  case 1:
    ledfile = fopen("/sys/devices/platform/leds/leds/beaglebone:green:usr1/brightness", "w");
    fprintf(ledfile,"%d",togvalue);
    break;
  case 2:
    ledfile = fopen("/sys/devices/platform/leds/leds/beaglebone:green:usr2/brightness", "w");
    fprintf(ledfile,"%d",togvalue);
    break;
  case 3:
    ledfile = fopen("/sys/devices/platform/leds/leds/beaglebone:green:usr3/brightness", "w");
    fprintf(ledfile,"%d",togvalue);
    break;
  default:
    printf("Not a valid LED number.\n Please choose from LED 1,2, or 3\n");
  }

  if(ledfile == NULL)
    {
      printf("ERROR!  LED file not opened\n");
      return -1;
    }
  
  fclose(ledfile);
  return 0;
}
