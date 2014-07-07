#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

// stdbool and C99 are needed because Dirac.h uses "bool"
#include <stdbool.h>
#include <Dirac.h>

#include "options.h"


#define BUFFER_FRAMES 4096


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

static int do_processing(void *dirac, struct opts *opt)
{
	int ret = 0;

	// Allocate buffer to hold output audio data
	float *buf = malloc(opt->channels * BUFFER_FRAMES * sizeof(*buf));
	if (!buf) {
		perror("malloc");
		return 1;
	}

	// Do the processing
	long n;
	while ((n = DiracProcessInterleaved(buf, BUFFER_FRAMES, dirac)) > 0)
		write(STDOUT_FILENO, buf, n * opt->channels * sizeof(*buf));
	if (n < 0) {
		fprintf(stderr, "DIRAC error: %s\n", DiracErrorToString(n));
		ret = 1;
	}

	free(buf);
	return ret;
}

int main(int argc, char **argv)
{
	if (isatty(STDOUT_FILENO)) {
		fprintf(stderr, "Refusing to output to a terminal\n");
		return 1;
	}

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

	// Create DIRAC instance
	void *dirac = DiracCreateInterleaved(kDiracLambdaPreview + opt.lambda,
			kDiracQualityPreview + opt.quality,
			opt.channels, opt.rate, &read_callback, NULL);
	if (!dirac) {
		fprintf(stderr, "%s: could not create DIRAC instance\n", argv[0]);
		return 1;
	}

	// Set up DIRAC from command-line options
	DiracSetProperty(kDiracPropertyTimeFactor, opt.time, dirac);
	DiracSetProperty(kDiracPropertyPitchFactor, opt.pitch, dirac);
	DiracSetProperty(kDiracPropertyFormantFactor, opt.formant, dirac);

	int ret = do_processing(dirac, &opt);

	// Clean up DIRAC instance
	DiracDestroy(dirac);

	return ret;
}
