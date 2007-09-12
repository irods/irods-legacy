/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/*
  header file for the low level, uses either Oracle or Postgres
 */

#ifdef ORA_ICAT
#include "icatLowLevelOracle.h"
#else
#include "icatLowLevelPostgres.h"
#endif
