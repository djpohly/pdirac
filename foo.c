
/*
 "main.cpp" Example Source File - Disclaimer:
 
 IMPORTANT:  This file and its contents are subject to the terms set forth in the 
 "License Agreement.txt" file that accompanies this distribution.
 
 Copyright © 2012 Stephan M. Bernsee, http://www.dspdimension.com. All Rights Reserved
 
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <memory.h>

#include <stdbool.h>
#include <Dirac.h>

// this defines the maximum number of files that we can use on input. This is enough to process
// 7.1 format and beyond. Increase accordingly if you need more
#define MAX_NUM_FILES		16

#ifdef WIN32
	#define strtold strtod
#endif



// This is the struct that holds state variables that our callback needs. In your program
// you will want to replace this by a pointer to "this" in order to access your instance methods
// and variables
typedef struct {
	unsigned long sReadPosition, sMaxFrames;
	long sTotalNumChannels, sNumFiles;
	long *sInFileNumChannels;
	char **sInFileNames;
	char **sOutFileNames;
} userDataStruct;


//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/*
 This is the callback function that supplies data from the input stream/file(s) whenever needed.
 It should be implemented in your software by a routine that gets data from the input/buffers.
 The read requests are *always* consecutive, ie. the routine will never have to supply data out
 of order.
 */
long myReadData(float **chdata, long numFrames, void *userData)
{	
	
	return 1;	
	
}




//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/*
 Creates an output path from a given input path using the specified file name prefix
 */
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/*
 Prints usage and CLI parameters to stdout.
 */

void usage(char *s)
{
	printf("%s -{options} -f <infile> {<infile2> <infile3> ...}\n",s);
	printf(" <infile> must be in AIFF format\n\n");
	printf(" options\n");
	printf("   -L     <int>          : Lambda value (0-6). This sets Dirac's lambda parameter\n");
	printf("                           default=0 (preview)\n");
	printf("   -Q     <int>          : Quality (0-3), with higher values being slower and better\n");
	printf("                           default=0 (preview)\n");
	printf("   -T     <long double>  : Time stretch factor\n");
	printf("                           default=1.0 (no change)\n");
	printf("   -B     <long double>  : BPM (inverse time) stretch factor\n");
	printf("                           default=1.0 (no change)\n");
	printf("   -P     <long double>  : Pitch shift factor\n");
	printf("                           default=1.0 (no change)\n");
	printf("   -F     <long double>  : Formant shift factor\n");
	printf("                           default=1.0 (no change)\n");
	printf("   -f     <string>       : Path to input file(s),\n");
	printf("\n");
	printf("   -h                    : print this message.\n\n");
	exit(1);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

int main (int argc, char **argv)
{
	float sr = 96000.0;
	
	// We stuff all our programs' state variables that we need to access in order to read from the file in a struct
	// You will normally pass your instance pointer "this" as userData, but since this is not a class we cannot do this here
	userDataStruct state;
	
	
	void *dirac = DiracCreate(kDiracLambdaPreview+3, kDiracQualityPreview+3, 8, sr, &myReadData, (void*)&state);		// (1) fastest
	if (!dirac) {
		printf("!! ERROR !!\n\n\tCould not create DIRAC instance\n\tCheck number of channels and sample rate!\n");
		exit(-1);
	}
	
	DiracDestroy(dirac);
	
	
	return 0;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
