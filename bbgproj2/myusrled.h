/**
* @file myusrled.h
* @brief fxns for interacting with USR LEDS on the BeagleBone Green
* @author Andrew Kuklinski and Adam Nuhaily
* @date 03/11/2018
**/


#ifndef myusrled_h_
#define myusrled_h_


/**
 *@brief used to toggle the USR LED's on the beaglebone green
 *
 *@param "lednum" which led to toggle, not including USRLED 0, options 1,2,3
 *@param "togvalue" 0 for off, 1 for on
 *
 *@return VOID
 */

int usr_led_toggle(int lednum, int togvalue);

#endif /*__myusrled_h_*/
