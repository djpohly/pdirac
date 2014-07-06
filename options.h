#ifndef OPTIONS_H_
#define OPTIONS_H_

struct opts {
	long double time;
	long double pitch;
	long double formant;
	long lambda;
	long quality;
	long channels;
	float rate;
};

void usage(char *fname);
int parse_options(int argc, char **argv, struct opts opt);

#endif
