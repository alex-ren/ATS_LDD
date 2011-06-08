//
// Author: Zhiqiang Ren (aren AT cs DOT bu DOT edu)
// Time: Jun. 6th., 2011
//
/* ************** ************* */

#ifndef ATS_MSDOS_MISC_CATS
#define ATS_MSDOS_MISC_CATS

#include "msdos_inode.h"
#include "msdos_misc.h"

ATSinline ()
ats_int_type atsfs_msdos_format_name(ats_ptr_type name, 
     ats_int_type len, ats_ptr_type res, ats_ref_type opts)
{
    return msdos_format_name(name, len, res, opts);

}

/***** Creates a directory entry (name is already formatted). */
ATSinline ()
ats_int_type atsfs_msdos_add_entry(ats_ref_type dir, 
    ats_ptr_type name, ats_bool_type is_dir, ats_bool_type is_hid, 
    ats_int_type cluster, ats_ref_type ts, ats_ref_type sinfo)
{
    return msdos_add_entry(dir, name, is_dir, is_hid, cluster, ts, sinfo);
}


#endif


