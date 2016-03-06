/*
 * Copyright 2016,
 *
 * This file is part of PowerTOP
 *
 * This program file is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program in a file named COPYING; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 */
#include <string.h>
#include <jni.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <android/log.h>

#include "deamon_bat.h"
#include "powertop.h"

jstring
Java_com_p_battery_BatteryService_pBatterySaverFromJNI( JNIEnv* env,
                                                  jobject thiz )
{
	  int ret = 0;
	  int bstatus = 0;
	  FILE *fp = NULL;
	  char line[4];

	  
	  __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "BatterySaverFromJNI\n");

	  // disable driver/reduce clock according battery status.
	  fp = fopen("/sys/class/power_supply/test_battery/capacity", "r");
	  if (NULL == fp) {
	  	__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, " pBatterySaverFromJNIL = %d \n , fp");
	     //return;
	  } else {
		  memset(line, 0, 4);
		  if (fgets(line, 4, fp) != NULL) {
			bstatus = atoi(line);
		  }
		  fclose(fp);	
	  }
	  if (bstatus == 50) {
		printf("battery status = 50%!\n");
	  } if (bstatus == 30) {
		printf("battery status = 30%!\n");
	  } if (bstatus == 20) {
		printf("battery status = 20%!\n");
	  }

	 ret = baterry_low (); 

  	 __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "battery_low =  %d", ret);
   
   	return (*env)->NewStringUTF(env, " P_BatDeamon  ");
//  return (*env)->NewStringUTF(env, " Smart BatDeamon  " ABI ".");
}

int baterry_low () {
	signed int retnmi = -1;
	
	// switch of the driver..
	suggest_bluetooth_off ();	// bluetooth.c- not working
	
	//suggest_wireless_powersave ();	// wireless.c - not working, NDK not supported. Enable first in Android.mk file
	suggest_wifi_new_powersave ();	// wifi.c     - not working
			
       retnmi =/* nmi_watchdog_off (); */suggest_nmi_watchdog ();// misctips.c - working  [NOT]   
	 __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "[baterry_low] nmi_watchdog_off  =  %d", retnmi);
	   
	retnmi = suggest_ondemand_governor();    // cpufreq.c  - Working
	__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "[baterry_low] suggest_ondemand_governor  =  %d", retnmi);
	
	retnmi = /*noatime_on ();       */suggest_noatime ();// misctips.c - not working
	__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "[baterry_low] suggest_noatime  =  %d", retnmi);
	
	//suggest_sata_alpm();			      -	work on implementation
	retnmi = suggest_powersched ();		// misctips.c - working [NOT]
	__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "[baterry_low] suggest_powersched  =  %d", retnmi);
	
	retnmi = suggest_writeback_time();	//  misctips.c - working	
	__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "[baterry_low] suggest_writeback_time  =	%d", retnmi);
	
	retnmi =  activate_usb_autosuspend();     //suggest_usb_autosuspend();//usb.c working [PARTIAL]
	__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "[baterry_low] activate_usb_autosuspend  =	%d", retnmi);

	usb_activity_hint();		// urbnum.c, calling usb.c above func.
  	
	//alsa_activity_hint();
	//ahci_activity_hint();
	return 0;
}

jstring
Java_com_p_battery_BatteryService_batteryless30JNI( JNIEnv* env,
                                                  jobject thiz )
{
	int ret = -1;
	ret = set_cpu_freq();
	__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "[baterry_low] set_cpu_freq  =	%d", ret);
	ret = set_gpu_freq();
	__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "[baterry_low] set_gpu_freq  =	%d", ret);
}

