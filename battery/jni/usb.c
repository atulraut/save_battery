/*
 *
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <dirent.h>
#include <android/log.h>

#include "deamon_bat.h"
#include "powertop.h"

int activate_usb_autosuspend(void)
{
	  DIR *dir;
	  struct dirent *dirent;
	  FILE *file;
	  char filename[PATH_MAX];
	  int uret = -1;

	  dir = opendir("/sys/bus/usb/devices");
	  if (!dir) {
  	      __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, " [00]failed to open /usb/devices directory \n");  	
	      return uret;
	  }

	  while ((dirent = readdir(dir))) {
	    if (dirent->d_name[0]=='.')
	      continue;
	    sprintf(filename, "/sys/bus/usb/devices/%s/power/autosuspend", dirent->d_name);
	    file = fopen(filename, "w");
	    if (!file)
	      continue;
	    fprintf(file, "0\n");    
	    fclose(file);

	    sprintf(filename, "/sys/bus/usb/devices/%s/power/level", dirent->d_name);
	    file = fopen(filename, "w");
	    if (!file)
	      continue;
	    fprintf(file, "auto\n");
	    fclose(file);
	  }
	  closedir(dir);
	  uret = 0;
	   __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, " [02] activate usb autosuspend \n");
	return uret;
}

void suggest_usb_autosuspend(void)
{
	  DIR *dir;
	  struct dirent *dirent;
	  FILE *file;
	  char filename[PATH_MAX];
	  char line[1024];
	  int need_hint  = 0;

	  dir = opendir("/sys/bus/usb/devices");
	  if (!dir)
	    return;

	  while ((dirent = readdir(dir))) {
	    if (dirent->d_name[0]=='.')
	      continue;
	    sprintf(filename, "/sys/bus/usb/devices/%s/power/autosuspend", dirent->d_name);
	    file = fopen(filename, "r");
	    if (!file)
	      continue;
	    memset(line, 0, 1024);
	    if (fgets(line, 1023,file)==NULL) {
	      fclose(file);
	      continue;
	    }
	    if (strtoll(line, NULL,10)<0)
	      need_hint = 1;
	    fclose(file);

	    sprintf(filename, "/sys/bus/usb/devices/%s/power/level", dirent->d_name);
	    file = fopen(filename, "r");
	    if (!file)
	      continue;
	    memset(line, 0, 1024);
	    if (fgets(line, 1023,file)==NULL) {
	      fclose(file);
	      continue;
	    }
	    if (strstr(line, "on"))
	      need_hint = 1;
	    fclose(file);
	  }
	  closedir(dir);
	  if (need_hint) {
	    add_suggestion(_("Suggestion: Enable USB autosuspend by pressing the U key or adding \n"
			     "usbcore.autosuspend=1 to the kernel command line in the grub config"
			     ),
			   45, 'U', _(" U - Enable USB suspend "), activate_usb_autosuspend);
	  }
}


