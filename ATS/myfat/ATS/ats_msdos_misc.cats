//
// Author: Zhiqiang Ren (aren AT cs DOT bu DOT edu)
// Time: Jun. 6th., 2011
//
(* ************** ************* *)

#ifndef ATS_MSDOS_MISC_CATS
#define ATS_MSDOS_MISC_CATS
#include "msdos_inode.h"

ATSinline ()
int ats_msdos_format_name(const unsigned char *name, int len,
			     unsigned char *res, struct fat_mount_options *opts)
{
    msdos_format_name(name, len, res, opts);
}

#endif

/***** Creates a directory entry (name is already formatted). */
ATSinline ()
int ats_msdos_add_entry(struct inode *dir, const unsigned char *name,
			   int is_dir, int is_hid, int cluster,
			   struct timespec *ts, struct fat_slot_info *sinfo)
{
    return msdos_add_entry(dir, name, is_dir, is_hid, cluster, ts, sinfo)
}

