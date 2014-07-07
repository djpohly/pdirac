#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>

// stdbool and C99 are needed because Dirac.h uses "bool"
#include <stdbool.h>
#include <Dirac.h>

#include "options.h"


/*
 This is the callback function that supplies data from the input stream/file(s) whenever needed.
 It should be implemented in your software by a routine that gets data from the input/buffers.
 The read requests are *always* consecutive, ie. the routine will never have to supply data out
 of order.
 */
static long read_callback(float *chdata, long numFrames, void *userData)
{
	// Pretend EOF
	return 0;
}

int main(int argc, char **argv)
{
	// Set up default options
	struct opts opt = {
		.time = 1,
		.pitch = 1,
		.formant = 1,
		.lambda = 0,
		.quality = 0,
		.channels = 1,
		.rate = 48000,
	};

	switch (parse_options(argc, argv, &opt)) {
		case 1:
			fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
			return 1;
		case 2:
			// Special return value if -h/--help was given
			return 0;
	}

	fprintf(stderr,
			"-- DIRAC options --\n"
			"Time shift\t%Lf\n"
			"Pitch shift\t%Lf\n"
			"Formant shift\t%Lf\n"
			"Lambda setting\t%ld\n"
			"Quality setting\t%ld\n"
			"Channels\t%ld\n"
			"Sample rate\t%lf\n",
			opt.time, opt.pitch, opt.formant,
			opt.lambda, opt.quality,
			opt.channels, opt.rate);

	void *dirac = DiracCreateInterleaved(kDiracLambdaPreview + opt.lambda,
			kDiracQualityPreview + opt.quality,
			opt.channels, opt.rate, &read_callback, NULL);
	if (!dirac) {
		fprintf(stderr, "%s: could not create DIRAC instance\n", argv[0]);
		return 1;
	}

	DiracDestroy(dirac);

	return 0;
}
