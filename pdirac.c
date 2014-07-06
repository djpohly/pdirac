#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>

// stdbool and C99 are needed because Dirac.h uses "bool"
#include <stdbool.h>
#include <Dirac.h>

#include "options.h"


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

int main (int argc, char **argv)
{
	struct opts opt;

	switch (parse_options(argc, argv, &opt)) {
		case 1:
			return 1;
		case 2:
			return 0;
	}

	float sr = 96000.0;

	void *dirac = DiracCreate(kDiracLambdaPreview+3, kDiracQualityPreview+3, 8, sr, &myReadData, NULL);
	if (!dirac) {
		printf("!! ERROR !!\n\n\tCould not create DIRAC instance\n\tCheck number of channels and sample rate!\n");
		exit(-1);
	}

	DiracDestroy(dirac);


	return 0;
}
