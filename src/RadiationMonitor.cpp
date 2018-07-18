/*  
 *  Geiger Counter - Radiation Sensor Board
 *  
 *  Copyright (C) Libelium Comunicaciones Distribuidas S.L. 
 *  http://www.libelium.com 
 *  
 *  This program is free software: you can redistribute it and/or modify 
 *  it under the terms of the GNU General Public License as published by 
 *  the Free Software Foundation, either version 3 of the License, or 
 *  (at your option) any later version. 
 *  a
 *  This program is distributed in the hope that it will be useful, 
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License 
 *  along with this program.  If not, see http://www.gnu.org/licenses/. 
 *  
 *  Radiation Code:	David Gascón and Marcos Yarza
 *  UoM Radiation Monitor Software Design and Implementation:	Nicolas Douard (nicolasdouard@gmail.com)            
 */

#include "arduPi.h"

#include <stdio.h>
#include <stdlib.h> /* For exit() function*/

#include <time.h>
#include <sstream>
#include <string.h>

FILE *fp;

// Conversion factor - CPM to uSV/h
#define CONV_FACTOR 0.00812

int geiger_input = 2;
long count = 0;
long countPerMinute = 0;
float radiationValue = 0.0;

char FileName[128];
char BaseYearMonth[128];
char RealYearMonth[128];
char UnitName[1000];

void countPulse();

void setup(){
	pinMode(geiger_input, INPUT);
	digitalWrite(geiger_input,HIGH);
	attachInterrupt(2,countPulse, FALLING);
}

void printTime(){
	static int seconds_last = 99;
	char TimeString[128];

	timeval curTime;
	gettimeofday(&curTime, NULL);
	if (seconds_last == curTime.tv_sec)
		return;
	
	seconds_last = curTime.tv_sec;
	
	strftime(TimeString, 80, "%Y-%m-%d %H:%M:%S", localtime(&curTime.tv_sec));

	//console output
	fprintf(stderr, "%s, ", TimeString);

	//write to global log file
	fp = fopen(FileName, "a");
	fprintf(fp, "%s, ", TimeString);
	fclose(fp);	
}


void writeHeader(){
	fprintf(stderr,"Writing log file header...\n");
	//write to global log file
	fp = fopen(FileName, "a");
	fprintf(fp, "%s\n", "Time, CPM, Activity (uSv/h)");
	fclose(fp);
}


void loop(){

	//detachInterrupt for accesing securely to count variable
	detachInterrupt(2);

	countPerMinute = 6*count;
	radiationValue = countPerMinute * CONV_FACTOR;

	printTime();

	//console output
	fprintf(stderr,"CPM=");
	fprintf(stderr,"%d, ",countPerMinute);
	fprintf(stderr,"%f",radiationValue);
	fprintf(stderr," uSv/h\n");

	//write to global log file
	fp = fopen(FileName, "a");
	//fprintf(fp,"CPM=");
	fprintf(fp,"%d, ",countPerMinute);
	fprintf(fp,"%f\n",radiationValue);
	//fprintf(fp," uSv/h\n");
	fclose(fp);

	count = 0;

	attachInterrupt(2,countPulse,FALLING);
	
	delay(10000);
}

void countPulse(){
  count++;
}

void setBaseYearMonth(int silent){
	static int seconds_last = 99;

	timeval curTime;
	gettimeofday(&curTime, NULL);
	if (seconds_last == curTime.tv_sec)
		return;
	
	seconds_last = curTime.tv_sec;
	
	strftime(BaseYearMonth, 80, "%Y-%m", localtime(&curTime.tv_sec));

	if (!(silent == 1)){
		fprintf(stderr, "BaseYearMonth: ");
		fprintf(stderr, BaseYearMonth);
		fprintf(stderr, "\n");
	}
}

void setRealYearMonth(int silent){
	static int seconds_last = 99;

	timeval curTime;
	gettimeofday(&curTime, NULL);
	if (seconds_last == curTime.tv_sec)
		return;
	
	seconds_last = curTime.tv_sec;
	
	strftime(RealYearMonth, 80, "%Y-%m", localtime(&curTime.tv_sec));
	
	if (!(silent == 1)){
		fprintf(stderr, "RealYearMonth: ");
		fprintf(stderr, RealYearMonth);
		fprintf(stderr, "\n");
	}
}




void fileCheck(){
	char FileNameEnding[128];
	//determine and set current log file name
	static int seconds_last = 99;

	timeval curTime;
	gettimeofday(&curTime, NULL);
	if (seconds_last == curTime.tv_sec)
		return;

	seconds_last = curTime.tv_sec;
	
	if (sizeof(FileName) < strlen(UnitName) + 1 ) { /* +1 is for null character */
        	fprintf(stderr, "The unit name '%s' is too long. The program will now stop.\n", UnitName);
        	exit(1);
    	}

	//copy unit name as log file name beginning 
	strncpy(FileName, UnitName, sizeof(FileName));

	//write log file name ending in buffer variable
	strftime(FileNameEnding, 80, "_log_%Y-%m.txt", localtime(&curTime.tv_sec));
	
	if (sizeof(FileName) < (strlen(FileName) + strlen(FileNameEnding) + 1) ) {
        	fprintf(stderr, "The final size of the log file name is too long. Try using a shorter unit name. The program will now stop.\n");
        	exit(1);
    	}

	//concatenate the two
	strncat(FileName, FileNameEnding, (sizeof(FileName) - strlen(FileName)) );
	
	//print final log file name to console
	printf("Log file name is: %s\n", FileName);
	
	//create log file if not existing
	fp = fopen(FileName, "r+");
	if (fp == NULL){
		fprintf(stderr,"Log file could not be accessed. Attempting to create a new file...\n");
		fp = fopen(FileName, "w+");
		fclose(fp);
		fprintf(stderr,"Log file created.\n");
		writeHeader();
		}
}


void readConfig(){
	FILE *fptr;
	if ((fptr=fopen("radmon.cfg","r"))==NULL){
		printf("Config file \"radmon.cfg\" could not be accessed. The program will now stop.");
		exit(1);         /* Program exits if file pointer returns NULL. */
	}
	fscanf(fptr,"%[^\n]",UnitName);
	printf("Unit name is set to: %s\n",UnitName);
	fclose(fptr);
}

int main (){
	while(1){

	//console init message
	fprintf(stderr,"Measurement program starting...\n");		
	
	readConfig();

	fileCheck();
	
	fprintf(stderr,"Setting up Time Base...\n");
	//argument 1 for silent call
	setBaseYearMonth(0);
	setRealYearMonth(0);

	fprintf(stderr,"Setting up I/O...\n");
	setup();

	//console init done message
	fprintf(stderr, "Measurement program started.\n");

	while(strcmp(BaseYearMonth, RealYearMonth) == 0){
		loop();
		//argument 1 for silent call
		//here silent call
		setRealYearMonth(1);
	}
	fprintf(stderr, "A month has passed: it`s time for a new file.\n");
	fprintf(stderr, "The system will restart in 10 seconds...\n");
	delay(10000);
	system("sudo reboot");
	}
	return (0);
}


    
