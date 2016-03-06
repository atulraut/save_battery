/*
 *
 * Authors:
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

static int activate_ondemand(void)
{
	DIR *dir;
	struct dirent *dirent;
	FILE *file;
	char filename[PATH_MAX];
	int ret = -1;

	system("/sbin/modprobe cpufreq_ondemand &> /dev/null");


	dir = opendir("/sys/devices/system/cpu");
	if (!dir)
		return ret;

	while ((dirent = readdir(dir))) {
		if (dirent->d_name[0]=='.')
			continue;
		sprintf(filename, "/sys/devices/system/cpu/%s/cpufreq/scaling_governor", dirent->d_name);
		file = fopen(filename, "w");
		if (!file)
			continue;
		fprintf(file, "ondemand\n");
		fclose(file);
	}

	closedir(dir);
	ret = 0;
	__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "  activate_ondemand : Tuning the ondemand governor ! \n");	
	return ret;
}

int suggest_ondemand_governor(void)
{
	DIR *dir;
	struct dirent *dirent;
	FILE *file;
	char filename[PATH_MAX];
	char line[1024];

	char gov[1024];
	int ret = 0;
	int oret = -1;

	gov[0] = 0;

	//__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, " [suggest_ondemand_governor] Inside ! \n");	

	dir = opendir("/sys/devices/system/cpu");
	if (!dir) {
		__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, " ] failed to open /sys/device/system/cpu directory, returning ! \n");	
		return oret;
	}
	while ((dirent = readdir(dir))) {
		if (dirent->d_name[0]=='.')
			continue;
		sprintf(filename, "/sys/devices/system/cpu/%s/cpufreq/scaling_governor", dirent->d_name);
		file = fopen(filename, "r");
		if (!file)
			continue;
		memset(line, 0, 1024);
		if (fgets(line, 1023,file)==NULL) {
			fclose(file);
			continue;
		}
		if (strlen(gov)==0)
			strcpy(gov, line);
		else
			/* if the governors are inconsistent, warn */
			if (strcmp(gov, line))
				ret = 1;
		fclose(file);
	}

	closedir(dir);

	/* if the governor is set to userspace, also warn */
	if (strstr(gov, "userspace"))
		ret = 1;

	/* if the governor is set to performance, also warn */
	/* FIXME: check if this is fair on all cpus */
	if (strstr(gov, "performance"))
		ret = 1;
	if (!ret) {
		__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, " Already cpu governer is = %s", gov);
		oret = 0;
	}
	if (ret) {
/*		add_suggestion(_("Suggestion: Enable the ondemand cpu speed governor for all processors via: \n"
				 " echo ondemand > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor \n"),
				15, 'O', _(" O - enable Ondemand governor "), activate_ondemand);
*/		oret = activate_ondemand ();
	}
	return oret;
}
/* set_cpu_freq :300MHZ  Min Freq : Maxfreq :2.15GHZ  
* shell@android:/ # cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_min_freq
* 300000
* shell@android:/ # cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq
* 2150400
* shell@android:/ # cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_available_frequencies                                      
* 300000 422400 652800 729600 883200 960000 1036800 1190400 1267200 1497600 1574400 1728000 1958400 2150400
*/
int set_cpu_freq(void) {
	DIR *dir;
	struct dirent *dirent;
	FILE *file;
	char filename[PATH_MAX];
	char line[100024];
	int ret = -1;

	dir = opendir("/sys/devices/system/cpu");
	if (!dir)
		return ret;

	while ((dirent = readdir(dir))) {
		if (dirent->d_name[0]=='.')
			continue;
		sprintf(filename, "/sys/devices/system/cpu/%s/cpufreq/scaling_setspeed", dirent->d_name);
		file = fopen(filename, "w");
		if (!file)
			continue;
		fprintf(file, "300000");
		fclose(file);
	}

	closedir(dir);
	ret = 0;
	__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "  set_cpu_freq : set frequency to 300MHZr ! \n");	
	return ret;
}
/* cat /sys/devices/fdb00000.qcom,kgsl-3d0/kgsl/kgsl-3d0/max_gpuclk
* 600000000  - 600 MHz
* 533333000  - 533 MHz
* 450000000  - 450 MHz (DEFAULT)
* 320000000  - 320 MHz (SET)
*/
int set_gpu_freq(void) {
	FILE *file;
	int pret = -1;
	file = fopen("/sys/devices/fdb00000.qcom,kgsl-3d0/kgsl/kgsl-3d0/max_gpuclk", "w");
	if (!file) {	
		__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, " [05] failed open file[w] max_gpuclk, returning \n");
		return pret;
		}	
	fprintf(file,"320000000");
	fclose(file);
	pret = 0;
	__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, " set_gpu_freq =  \n", pret);
	return pret;
}

