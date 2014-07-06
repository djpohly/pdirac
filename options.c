#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>

#include <stdbool.h>
#include <Dirac.h>

#include "options.h"

void print_usage(char *fname)
{
	fprintf(stderr,
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

static int parse_enum(char *desc, char *str, long *out, long max)
{
	if (!*str) {
		fprintf(stderr, "%s: empty argument\n", desc);
		return 1;
	}

	char *end;
	long temp = strtol(str, &end, 10);
	if (*end) {
		fprintf(stderr, "%s: `%s' is not an integer\n", desc, str);
		return 1;
	}
	if (temp < 0 || temp > max) {
		fprintf(stderr, "%s: `%s' is not between 0 and %ld\n", desc, str, max);
		return 1;
	}

	return 0;
}

static int parse_float(char *str, long double *out)
{
	if (!*str)
		return 1;

	char *end;
	long double temp = strtold(str, &end);
	if (*end)
		return 1;

	*out = temp;
	return 0;
}

static int parse_ratio(char *desc, char *str, long double *out, int allow_semi)
{
	if (!*str) {
		fprintf(stderr, "%s: empty argument\n", desc);
		return 1;
	}

	char *end;
	long double temp = strtold(str, &end);
	long double temp2;
	switch (*end) {
		case '\0':
			break;
		case '/':
			if (parse_float(end + 1, &temp2)) {
				fprintf(stderr, "%s: `%s' is not a valid fraction\n",
						desc, str);
				return 1;
			}
			temp /= temp2;
			break;
		case ':':
			if (parse_float(end + 1, &temp2)) {
				fprintf(stderr, "%s: `%s' is not a valid ratio\n",
						desc, str);
				return 1;
			}
			temp = temp2 / temp;
			break;
		case '%':
			if (*(end + 1)) {
				fprintf(stderr, "%s, `%s' is not a valid percentage\n",
						desc, str);
				return 1;
			}
			temp += 100;
			temp /= 100;
			break;
		case 's':
			if (allow_semi) {
				if (*(end + 1)) {
					fprintf(stderr, "%s, `%s' is not a valid semitone value\n",
							desc, str);
					return 1;
				}
				temp = pow(2, temp / 12);
				break;
			}
			// else fall through
		default:
			fprintf(stderr, "%s: `%s' is not a valid argument\n",
					desc, str);
			return 1;
	}

	*out = temp;
	return 0;
}

int parse_options(int argc, char **argv, struct opts *opt)
{
	static struct option longopts[] = {
		// Dirac parameters
		{"time",	required_argument,	NULL,	'T'},
		{"bpm",		required_argument,	NULL,	'B'},
		{"pitch",	required_argument,	NULL,	'P'},
		{"formant",	required_argument,	NULL,	'F'},
		{"lambda",	required_argument,	NULL,	'L'},
		{"quality",	required_argument,	NULL,	'Q'},

		// Other options
		{"help",	no_argument,	NULL,	'h'},
	};

	int c, idx;
	char *end;
	while ((c = getopt_long(argc, argv, "l:q:t:b:p:f:h", longopts, &idx)) >= 0) {
		switch (c) {
			case 't':
				if (parse_ratio("Time factor", optarg, &opt->time, 0))
					return 1;
				break;
			case 'b':
				if (parse_ratio("BPM factor", optarg, &opt->time, 0))
					return 1;
				opt->time = 1 / opt->time;
				break;
			case 'p':
				if (parse_ratio("Pitch factor", optarg, &opt->pitch, 1))
					return 1;
				break;
			case 'f':
				if (parse_ratio("Formant factor", optarg, &opt->formant, 0))
					return 1;
				break;
			case 'l':
				if (parse_enum("Lambda", optarg, &opt->lambda,
							kDiracPropertyNumLambdas - kDiracLambdaPreview - 1))
					return 1;
				break;
			case 'q':
				if (parse_enum("Quality", optarg, &opt->quality,
							kDiracPropertyNumQualities - kDiracQualityPreview - 1))
					return 1;
				break;
			case 'h':
				print_usage(argv[0]);
				return 2;
		}
	}

	return 0;
}
