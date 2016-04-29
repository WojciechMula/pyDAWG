/*
	This is part of pydawg Python module.

	Declaration of Python class DAWG.
	(wrapper for functions from dawg/dawgnode.{c,h})

	Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
	WWW       : http://0x80.pl/proj/pydawg/
	License   : 3-clauses BSD (see LICENSE)
*/

#ifndef dawgclass_h_included__
#define dawgclass_h_included__

#include "dawg.h"

typedef struct DAWGclass {
    PyObject_HEAD

	DAWG dawg;		///< DAWG data

	int	version;	///< version
#ifdef DAWG_PERFECT_HASHING
	int mph_version;		///< version of perfect hashing numbering
#endif
	int stats_version;		///< version for statistics
	DAWGStatistics stats;	///< statistics
} DAWGclass;

#endif
