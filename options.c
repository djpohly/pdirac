#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>

#include <stdbool.h>
#include <Dirac.h>

#include "options.h"

static char *fname;

static void print_usage()
{
	printf(
		"Usage: %s [OPTION]... RATE CHANNELS\n"
		"Reads raw audio data from standard input, applies Dirac processing, and\n"
		"writes the result to standard output.  Input must be interleaved 32-bit\n"
		"float PCM in host endianness.\n"
		"\n"
		"Dirac options:\n"
		"  -l, --lambda=INT        Lambda value (0-%d).  Default is 0 (preview).\n"
		"  -q, --quality=INT       Quality (0-%d), with higher values being\n"
		"                          slower and better.  Default is 0 (preview).\n"
		"  -t, --time=FACTOR       Time stretch factor\n"
		"  -b, --bpm=FACTOR        BPM stretch factor (inverse of time)\n"
		"  -p, --pitch=FACTOR      Pitch shift factor\n"
		"  -f, --formant=FACTOR    Formant shift factor\n"
		"  -h, --help              Show usage\n"
		"\n"
		"Factors may be expressed as a multiplier, a fraction, a ratio, or a\n"
		"percentage change.  Pitch shifts may also be specified as a number of\n"
		"semitones.  For example, the following are all equivalent:\n"
		"  0.5   1/2   2:1   -50%%   -12s\n",
		fname,
		kDiracPropertyNumLambdas - kDiracLambdaPreview - 1,
		kDiracPropertyNumQualities - kDiracQualityPreview - 1);
}

static int parse_int(char *str, long *out, long max)
{
	// Make sure str is not NULL or empty
	if (!str || !*str)
		return 1;

	// Convert input
	char *end;
	long temp = strtol(str, &end, 10);

	// Make sure we consumed at least one character, that there are no
	// characters left, and that the value is in range
	if (end == str || *end || temp < 0 || (max > 0 && temp > max))
		return 1;

	*out = temp;
	return 0;
}

static int parse_float(char *str, float *out)
{
	// Make sure str is not NULL or empty
	if (!str || !*str)
		return 1;

	// Convert input
	char *end;
	float temp = strtof(str, &end);

	// Make sure we consumed at least one character and that there are no
	// more characters left
	if (end == str || *end)
		return 1;

	*out = temp;
	return 0;
}

static int parse_ldouble(char *str, long double *out)
{
	// Make sure str is not NULL or empty
	if (!str || !*str)
		return 1;

	// Convert input
	char *end;
	long double temp = strtold(str, &end);

	// Make sure we consumed at least one character and that there are no
	// more characters left
	if (end == str || *end)
		return 1;

	*out = temp;
	return 0;
}

static int parse_ratio(char *str, long double *out, int allow_semi)
{
	// Make sure str is not NULL or empty
	if (!str || !*str)
		return 1;

	// Convert first part of input
	char *end;
	long double temp = strtold(str, &end);

	// Make sure we consumed at least one character
	if (end == str)
		return 1;

	// Parse argument according to factor/fraction/ratio/percent/semitone
	// formats
	long double temp2;
	switch (*end) {
		case '\0':
			// Factor
			break;
		case '/':
			// Fraction
			if (parse_ldouble(end + 1, &temp2))
				return 1;
			temp /= temp2;
			break;
		case ':':
			// Ratio
			if (parse_ldouble(end + 1, &temp2))
				return 1;
			temp = temp2 / temp;
			break;
		case '%':
			// Percent change
			if (*(end + 1))
				return 1;
			temp += 100;
			temp /= 100;
			break;
		case 's':
			// Semitone change (if allowed)
			if (allow_semi) {
				if (*(end + 1))
					return 1;
				temp = pow(2, temp / 12);
				break;
			}
			// else fall through
		default:
			return 1;
	}

	*out = temp;
	return 0;
}

int parse_options(int argc, char **argv, struct opts *opt)
{
	int ret = 0;

	// Save executable name for error messages
	fname = argv[0];

	// Declare long options for getopt_long
	static struct option longopts[] = {
		// Dirac parameters
		{"time",	required_argument,	NULL,	't'},
		{"bpm",		required_argument,	NULL,	'b'},
		{"pitch",	required_argument,	NULL,	'p'},
		{"formant",	required_argument,	NULL,	'f'},
		{"lambda",	required_argument,	NULL,	'l'},
		{"quality",	required_argument,	NULL,	'q'},

		// Other options
		{"help",	no_argument,	NULL,	'h'},
	};

	// Process options
	for (;;) {
		int idx = -1;
		int c = getopt_long(argc, argv, "l:q:t:b:p:f:h", longopts, &idx);
		if (c < 0)
			break;
		switch (c) {
			case 't':
				// Time factor
				ret = parse_ratio(optarg, &opt->time, 0);
				break;
			case 'b':
				// BPM factor (inverse of time factor)
				ret = parse_ratio(optarg, &opt->time, 0);
				if (!ret)
					opt->time = 1 / opt->time;
				break;
			case 'p':
				// Pitch factor
				ret = parse_ratio(optarg, &opt->pitch, 1);
				break;
			case 'f':
				// Formant factor
				ret = parse_ratio(optarg, &opt->formant, 0);
				break;
			case 'l':
				// Lambda parameter
				ret = parse_int(optarg, &opt->lambda,
						kDiracPropertyNumLambdas - kDiracLambdaPreview - 1);
				break;
			case 'q':
				// Quality parameter
				ret = parse_int(optarg, &opt->quality,
						kDiracPropertyNumQualities - kDiracQualityPreview - 1);
				break;
			case 'h':
				// Help
				print_usage();
				return 2;
			case '?':
				// Bad option; error message was already printed
				// by getopt
				return 1;
		}

		// Option was good but the argument to the option wasn't
		if (ret) {
			if (idx < 0)
				fprintf(stderr, "%s: invalid argument '%s' for '-%c'\n",
						fname, optarg, c);
			else
				fprintf(stderr, "%s: invalid argument '%s' for '%s'\n",
						fname, optarg, longopts[idx].name);
			return 1;
		}
	}

	if (optind >= argc) {
		fprintf(stderr, "%s: missing sample rate and channels\n", fname);
		return 1;
	}

	if (parse_float(argv[optind], &opt->rate)) {
		fprintf(stderr, "%s: invalid sample rate '%s'\n", fname,
				argv[optind]);
		return 1;
	}
	optind++;

	if (optind >= argc) {
		fprintf(stderr, "%s: missing channels\n", fname);
		return 1;
	}

	if (parse_int(argv[optind], &opt->channels, -1)) {
		fprintf(stderr, "%s: invalid number of channels '%s'\n", fname,
				argv[optind]);
		return 1;
	}
	optind++;

	if (optind < argc) {
		fprintf(stderr, "%s: too many arguments\n", fname);
		return 1;
	}

	return 0;
}
