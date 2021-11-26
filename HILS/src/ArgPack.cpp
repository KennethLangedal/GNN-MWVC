/*
 * ArgPack.cpp
 *
 *  Created on: 14/09/2015
 *      Author: bruno
 */

#include "ArgPack.h"
#include "InitError.h"

#include <string>
#include <cstring>
#include <unistd.h> // for getopt
#include <iostream>

extern int optind;

using namespace std;

namespace opt
{

ArgPack *ArgPack::def_ap_ = 0;

ArgPack::ArgPack(int argc, char * const argv []) :
	verbose(true),
	rand_seed(1),
	target(0),
	complement(0),
	iterations(2000000),
	p{2,4,4,1}
{

	assert(!def_ap_);
	def_ap_ = this;
	program_name = argv[0];

	string usage = string("Usage: ") + program_name + " [options] <file>\n" +
	               "Compile time: " + __DATE__ + " " + __TIME__ + "\n" +
	               "	-h			: show this help\n" +
	               "	-i			: maximum number of iterations [default: " + to_string(iterations) +  "] \n" +
	               "	-s<random seed>		: random seed [default: " + to_string(rand_seed) + "]\n" +
	               "	-v			: disable verbose mode \n" +
//	               "	-o<output>      : output solution file [default: " + output_name + "]\n" +
	               "	-t<target>		: stop execution if the target is found\n" +
	               "	-W			: run the algorithm on the graph's complement\n" +
	               "	-p p1:p2:p3:p4		: search exploration/intensification parameters [default: p1=" + 
	               			to_string(p[0]) + " p2=" + to_string(p[1]) + " p3=" + 
							to_string(p[2]) + " p4=" + to_string(p[3]) + "]\n";
	string help = "Use -h for more information\n";

	const char *opt_str = "hs:vt:Wp:i:";

	long ch;

	while ((ch = getopt(argc, argv, opt_str)) != -1)
	{
		switch (ch)
		{
		case 'h':
			throw InitError(usage);
		case 'W':
			complement = 1;
			break;
		case 't':
			target = strtoul(optarg, NULL, 10);
			break;
		case 'i':
			iterations = strtoul(optarg, NULL, 10);
			break;
		case 's':
			rand_seed = strtoul(optarg, NULL, 10);
			break;
		case 'v':
			verbose = false;
			break;
		case 'p':
		{
			char* token = NULL;
			int i = 0;
			token = strtok(optarg, ":");
			while (token) {
				p[i] = strtoul(token, NULL, 10);
				token = strtok(NULL, ":");
				i++;
			}

			if(i != 4)
				throw InitError("wrong input format for -p option\n");
			break;
		}
		case '?':
			throw InitError(help);
		}
	}
	argc -= optind;
	argv += optind;

	if (argc > 1)
	{
		throw InitError("Too many arguments\n" + help);
	}
	else if (argc < 1)
	{
		throw InitError("Too few arguments\n" + help );
	}
	else
	{
		input_name = argv[0];
	}
}

} // namespace opt