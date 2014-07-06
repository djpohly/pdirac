#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>

#include <stdbool.h>
#include <Dirac.h>

#include "options.h"

void usage(char *fname)
{
	printf("%s -{options} -f <infile> {<infile2> <infile3> ...}\n", fname);
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

int parse_options(int argc, char **argv, struct opts opt)
{
	static struct option longopts[] = {
		// Dirac parameters
		{"time",	required_argument,	NULL,	'T'},
		{"bpm",		required_argument,	NULL,	'B'},
		{"pitch",	required_argument,	NULL,	'P'},
		{"formant",	required_argument,	NULL,	'F'},
		{"lambda",	required_argument,	NULL,	'L'},
		{"quality",	required_argument,	NULL,	'Q'},

		// Input parameters
		{"rate",	required_argument,	NULL,	'r'},
		{"channels",	required_argument,	NULL,	'c'},
	};

	int c, idx;
	char *end;
	while ((c = getopt_long(argc, argv, "r:c:L:Q:T:B:P:F:", longopts, &idx)) >= 0) {
		switch (c) {
			case 'T':
				if (parse_ratio("Time factor", optarg, &opt.time, 0))
					return 1;
				break;
			case 'B':
				if (parse_ratio("BPM factor", optarg, &opt.time, 0))
					return 1;
				opt.time = 1 / opt.time;
				break;
			case 'P':
				if (parse_ratio("Pitch factor", optarg, &opt.pitch, 1))
					return 1;
				break;
			case 'F':
				if (parse_ratio("Formant factor", optarg, &opt.formant, 0))
					return 1;
				break;
			case 'L':
				if (parse_enum("Lambda", optarg, &opt.lambda,
							kDiracPropertyNumLambdas - kDiracLambdaPreview - 1))
					return 1;
				break;
			case 'Q':
				if (parse_enum("Quality", optarg, &opt.quality,
							kDiracPropertyNumQualities - kDiracQualityPreview - 1))
					return 1;
				break;
			case 'r':
				break;
			case 'c':
				break;
		}
	}

	return 0;
}
