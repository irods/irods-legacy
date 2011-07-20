/* For copyright information please refer to files in the COPYRIGHT directory
 */

#ifndef LOCKS_H
#define LOCKS_H
#ifdef USE_BOOST
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/creation_tags.hpp>
#else
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/fcntl.h>
#include <unistd.h>
#endif
#define SEM_NAME "irods_sem_re"
#ifdef USE_BOOST
typedef boost::interprocess::named_mutex mutex_type;
#else
typedef sem_t mutex_type;
#endif

void unlockMutex(mutex_type **mutex);
int lockMutex(mutex_type **mutex);

#endif
