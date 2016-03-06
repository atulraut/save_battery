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

void set_laptop_mode(void)
{
	FILE *file;
	file = fopen("/proc/sys/vm/laptop_mode", "w");
	if (!file)
		return;
	fprintf(file,"5\n");
	fclose(file);
}

void suggest_laptop_mode(void)
{
	FILE *file;
	int i;
	char buffer[1024];
	/*
	 * Check to see if we are on AC - lots of distros have
	 * annoying scripts to turn laptop mode off when on AC, which
	 * results in annoying distracting return of set laptop mode
	 * hint.
	 */
	file = fopen("/proc/acpi/ac_adapter/AC/state", "r");
	if (!file)
		return;
	memset(buffer, 0, 1024);
	if (!fgets(buffer, 1023, file)) {
		fclose(file);
		return;
	}
	fclose(file);
	if (strstr(buffer, "on-line") != NULL)
		return;

	/* Now check for laptop mode */
	file = fopen("/proc/sys/vm/laptop_mode", "r");
	if (!file)
		return;
	memset(buffer, 0, 1024);
	if (!fgets(buffer, 1023, file)) {
		fclose(file);
		return;
	}
	i = strtoul(buffer, NULL, 10);
	if (i<1) {
		add_suggestion( _("Suggestion: Enable laptop-mode by executing the following command:\n"
		 	"   echo 5 > /proc/sys/vm/laptop_mode \n"), 15, 'L', _(" L - enable Laptop mode "), set_laptop_mode);
	}
	fclose(file);
}

int nmi_watchdog_off(void)
{
	FILE *file;
	file = fopen("/proc/sys/kernel/nmi_watchdog", "w");
	if (!file) {
		__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, " failed to open nmi_watchdog file[w], returing ! \n");	
		return  -1;
	}
	fprintf(file,"0\n");
	fclose(file);
	system("su");
	system ("echo 0 >  /proc/sys/kernel/nmi_watchdog"); // alternative
	__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, " disable non maskable interrupt \n");
	return 0;
}
int suggest_nmi_watchdog(void)
{
	FILE *file;
	int i;
	int retn = -1;
	char buffer[1024];
	file = fopen("/proc/sys/kernel/nmi_watchdog", "r");
	if (!file) {
		__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, " [suggest_nmi_watchdog] failed to open nmi_watchdog file, returing r mode ! \n");	
		return -1;
	}
	memset(buffer, 0, 1024);
	if (!fgets(buffer, 1023, file)) {
		fclose(file);
		__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "[suggest_nmi_watchdog] failed to buffer, returing ! \n");	
		return -1;
	}
	i = strtoul(buffer, NULL, 10);
	if (i!=0) {
		retn = nmi_watchdog_off ();
		add_suggestion( _("Suggestion: disable the NMI watchdog by executing the following command:\n"
		 	"   echo 0 > /proc/sys/kernel/nmi_watchdog \n"
			"The NMI watchdog is a kernel debug mechanism to detect deadlocks"), 25, 'N', _(" N - Turn NMI watchdog off "), nmi_watchdog_off);
	}
	fclose(file);
	return retn;
}

void suggest_hpet(void)
{
	FILE *file;
	char buffer[1024];
	file = fopen("/sys/devices/system/clocksource/clocksource0/available_clocksource", "r");
	if (!file)
		return;
	memset(buffer, 0, 1024);
	
	if (!fgets(buffer, 1023, file)) {
		fclose(file);
		return;
	}
	
	if (strstr(buffer, "hpet")) {
		fclose(file);
		return;
	}

	fclose(file);

	add_suggestion( _("Suggestion: enable the HPET (Multimedia Timer) in your BIOS or add \n"
		          "the kernel patch to force-enable HPET. HPET support allows Linux to \n"
			  "have much longer sleep intervals."), 7, 0, NULL, NULL);
}

void ac97_power_on(void)
{
	FILE *file;
	file = fopen("/sys/module/snd_ac97_codec/parameters/power_save", "w");
	if (!file)
		return;
	fprintf(file,"1");
	fclose(file);
	if (access("/dev/dsp", F_OK))
		return;
	file = fopen("/dev/dsp", "w");
	if (file) {
		fprintf(file,"1");
		fclose(file);
	}
}

void suggest_ac97_powersave(void)
{
	FILE *file;
	char buffer[1024];
	file = fopen("/sys/module/snd_ac97_codec/parameters/power_save", "r");
	if (!file)
		return;
	memset(buffer, 0, 1024);
	if (!fgets(buffer, 1023, file)) {
		fclose(file);
		return;
	}
	if (buffer[0]=='N') {
		add_suggestion( _("Suggestion: enable AC97 powersave mode by executing the following command:\n"
		 	"   echo 1 > /sys/module/snd_ac97_codec/parameters/power_save \n"
			"or by passing power_save=1 as module parameter."), 25, 'A', _(" A - Turn AC97 powersave on "), ac97_power_on);
	}
	fclose(file);
}

void noatime_on(void)
{
	system("mount -o remount,noatime,nodiratime /");
	__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, " enable the noatime filesystem option to save powe \n");
}

/* suggest_noatime: Reduce Disk Writes With noatime, nodiratime and relatime in Linux */
int suggest_noatime(void)
{
	FILE *file;
	char buffer[1024];
	int suggest = 0;
	int ret = -1;
	file = fopen("/proc/mounts","r");
	if (!file) {
	  	__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "failed to open /proc/mounts \n");
	  return ret;
	}
	while (!feof(file)) {
		memset(buffer, 0, 1024);
		if (!fgets(buffer, 1023, file))
			break;
		if (strstr(buffer, " / ext3") && !strstr(buffer, "noatime") && !strstr(buffer, "relatime"))
			suggest = 1;

	}
	if (suggest) {
		add_suggestion( _("Suggestion: enable the noatime filesystem option by executing the following command:\n"
		 	"   mount -o remount,noatime /          or by pressing the T key \n"
			"noatime disables persistent access time of file accesses, which causes lots of disk IO."), 5, 'T', _(" T - enable noatime "), noatime_on);
		noatime_on ();
	}
	fclose(file);
	__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "enable the noatime filesystem option to save power \n");
	return 0;
}

int powersched_on(void)
{
	FILE *file;
	int pret = -1;
	file = fopen("/sys/devices/system/cpu/sched_mc_power_savings", "w");
	if (!file) {	
		__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, " [05] failed open file[w] sched_mc_power_savings, returning \n");
		return pret;
		}	
	fprintf(file,"1");
	fclose(file);
	pret = 0;
	__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, " powersched_on =  \n", pret);
	return pret;
}

int suggest_powersched(void)
{
	FILE *file;
	char buffer[1024];
	int suggest = 0;
	int cpu;
	int pret = -1;

	__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, " [00] Inside suggest_powersched \n");
	
	file = fopen("/sys/devices/system/cpu/sched_mc_power_savings","r");
	if (!file) {
		__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, " [01] failed to open file[r] sched_mc_power_savings, returning \n");
		return pret; 
	}
	memset(buffer, 0, 1024);
	if (!fgets(buffer, 1023, file)) {
		fclose(file);
		__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, " [02] failed fgets, returning \n");
		return pret;
	}
	fclose(file);
	if (buffer[0]!='0') {
		__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, " [03] failed buffer, returning \n");
		return pret;
	}
	/* ok so power saving scheduler is off; now to see if we actually have a multi-package system */
	cpu =  sysconf(_SC_NPROCESSORS_ONLN);

	if (cpu<2) {
		__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, " [04] failed cpu<1, returning \n");
		return pret; /* UP system */
	}

	file = fopen("/proc/cpuinfo", "r");
	suggest = 1;
	if (!file) 
		return pret;
	while (!feof(file)) {
		memset(buffer, 0, 1024);
		char *c;
		if (!fgets(buffer, 1023, file))
			break;
		if (strstr(buffer, "cpu cores")) {
			c = strchr(buffer, ':');
			if (!c) 
				continue;
			c++;
			if (strtoll(c, NULL, 10) >= cpu)
				suggest = 0;
		}
	}
	fclose(file);


	if (suggest) { /*
		add_suggestion( _("Suggestion: enable the power aware CPU scheduler with the following command:\n"
		 	"  echo 1 > /sys/devices/system/cpu/sched_mc_power_savings\n"
			"or by pressing the C key."), 5, 'C', _(" C - Power aware CPU scheduler "), powersched_on);	*/
		pret = powersched_on ();
	}
	return pret; 
}


int  writeback_long(void)
{
	int wret = -1;
	FILE *file;
	file = fopen("/proc/sys/vm/dirty_writeback_centisecs", "w"); 
	if (!file)	{
		__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, " [01] failed to open file [w] dirty_writeback_centisecs, returning \n");
		return wret;
	}
	fprintf(file,"1500");
	fclose(file);
	wret = 0;	
	__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, " [02] increase the VM dirty writeback time \n");
	return wret;
}

int suggest_writeback_time(void)
{
	FILE *file;
	char buffer[1024];
	int i;
	int wret = -1;
	
	file = fopen("/proc/sys/vm/dirty_writeback_centisecs", "r");
	if (!file) 
		return;
	memset(buffer, 0, 1024);
	if (!fgets(buffer, 1023, file)) {
		fclose(file);
		return;
	}
	i = strtoull(buffer, NULL, 10);
	if (i<1400) {
		char line[1024];
		sprintf(line,_("Suggestion: increase the VM dirty writeback time from %1.2f to 15 seconds with:\n"
			 	"  echo 1500 > /proc/sys/vm/dirty_writeback_centisecs \n"
				"This wakes the disk up less frequently for background VM activity"),
			i/100.0);
//		add_suggestion(line, 15, 'W', _(" W - Increase Writeback time "), writeback_long);
		wret = writeback_long ();
	}

	fclose(file);
	return wret;
}
