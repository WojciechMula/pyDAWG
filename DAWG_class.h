#ifndef dawgclass_h_included__
#define dawgclass_h_included__

#include "dawg.h"

typedef struct DAWGclass {
    PyObject_HEAD

	DAWG dawg;		///< DAWG data

	int	version;	///< version
	int stats_version;		///< version for statistics
	DAWGStatistics stats;	///< statistics
} DAWGclass;

#endif
