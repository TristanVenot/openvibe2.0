#include "../ovasCTagStream.h"

using namespace OpenViBEAcquisitionServer;
using namespace OpenViBEAcquisitionServerPlugins;

#include <iostream>

int main()
{
	bool ok=false;

	CTagStream tagStream1;

	// The construction of the second TagStream must fail because of port already in use.
	try {
		CTagStream tagStream2;
	}
	catch( std::exception&) {
		// This exception is expected, don't print
		ok=true;
	}
	
	// The construction must succeed because another port is used.
	try {
		CTagStream tagStream3(15362);
	}
	catch( std::exception& e)
	{
		// Unexpected exception
		std::cout << "Exception: " << e.what() << "\n";
		ok = false;	
	}

	if (!ok)
		return 1;

	return 0;
}
