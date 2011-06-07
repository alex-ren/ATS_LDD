//
// Author: Zhiqiang Ren (aren AT cs DOT bu DOT edu)
// Time: March 17th., 2011
//
(* ****** ****** *)

#define ATS_STALOADFLAG 0

(* ****** ****** *)

%{#
// #include "contrib/linux/include/ats_types.h"
#include "contrib/linux/include/ats_basics.h"
#include "contrib/linux/include/ats_memory.h"

#include "ATS/include/atsfs_basic_types.h"
#include "ATS/include/atsfs_fat.cats"


#include "contrib/linux/CATS/array.cats"
#include "contrib/linux/CATS/integer.cats"
#include "contrib/linux/CATS/pointer.cats"
#include "contrib/linux/CATS/sizetype.cats"



//
// for handling a call like: printk (KERN_INFO "...")
//
#ifdef ATSstrcst
#undef ATSstrcst
#endif
#define ATSstrcst(x) x
//

%} // end of [%{#]

(* ****** ****** *)

(* end of [myheader.sats] *)

