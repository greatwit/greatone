
#include "Utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int getCpuCoreNumber()
{
	char *cmd_get_num_cores = "cat /proc/cpuinfo | grep 'core id' | uniq |wc -l"; 
	FILE *pd = NULL;    
	int num_cores;
	assert( (pd = popen( cmd_get_num_cores, "r")) != NULL );
	int nc = fscanf(pd, "%d", &num_cores);
	assert( nc == 1 && num_cores > 0 );
	pclose(pd);
	return nc;
}

int getCpuMainFreq()
{
	char *cmd_get_main_freq = "dmesg | grep -e 'Detected' | grep -e 'processor' | sed -e 's/.*\\s\\+\\([.0-9]\\+\\)\\s\\+MHz.*/\\1/'";
	FILE *pd = NULL;   
	int num_cores;
	assert( (pd = popen( cmd_get_main_freq, "r")) != NULL );
	int nc = fscanf(pd, "%d", &num_cores);
	assert( nc == 1 && num_cores > 0 );
	pclose(pd);
	return num_cores;
}


