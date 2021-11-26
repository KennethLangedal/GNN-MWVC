/*
 *
 * ArgPack: reads and stores the configuration parameters for ILS-VND
 *
 *  Created on: 14/09/2015
 *      Author: bruno
 */

#ifndef ARGPACK_H_
#define ARGPACK_H_

#include <string>
#include <assert.h>

namespace opt {

class ArgPack {

public:

	//------------
	// program parameters
	//------------

	bool verbose;

	long rand_seed;

	int target;

	int complement;

	std::string input_name, program_name;

	int iterations; // maximum iteration number

	double p[4]; // intensification/exploration parameters

	//------------
	// singleton functions
	//------------

	static const ArgPack &ap() { assert(def_ap_); return *def_ap_; }

//	static ArgPack &write_ap() { assert(def_ap_); return *def_ap_; }

	ArgPack(int argc, char * const argv []);

	~ArgPack() { assert(def_ap_); def_ap_ = 0; }

private:

	//------------
	// singleton instance
	//------------

	static ArgPack *def_ap_;

};

}

#endif /* ARGPACK_H_ */