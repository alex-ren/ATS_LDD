//
// Author: Zhiqiang Ren (aren AT cs DOT bu DOT edu)
// Time: May 31st., 2011
//
/* ****** ****** */

#ifndef ATS_MSDOS_INODE_CATS
#define ATS_MSDOS_INODE_CATS

#include "ATS/ats_msdos_inode.h"

#include "ATS/ats_fs_types.h"

ATSinline ()
void atsfs_CURRENT_TIME_SEC(ats_ref_type ts)
{
    ((timespec_struct *)ts)->tv_sec = get_seconds();
    ((timespec_struct *)ts)->tv_nsec = 0;
}

#endif


