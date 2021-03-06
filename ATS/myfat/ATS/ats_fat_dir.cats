//
// Author: Zhiqiang Ren (aren AT cs DOT bu DOT edu)
// Time: Jun 6th., 2011
//

/* ****** ****** */

#ifndef ATS_FAT_DIR_CATS
#define ATS_FAT_DIR_CATS

#include "fat_dir.h"

typedef struct fat_slot_info fat_slot_info_struct;


ATSinline ()
ats_void_type atsfs_fat_slot_info_clear(ats_ref_type sinfo)
{
  brelse(((fat_slot_info_struct *)sinfo)->bh);
}


#endif


