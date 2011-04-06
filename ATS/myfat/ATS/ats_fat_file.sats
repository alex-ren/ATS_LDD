//
// Author: Zhiqiang Ren (aren AT cs DOT bu DOT edu)
// Time: April 6th., 2011
//
(* ****** ****** *)

#define ATS_STALOADFLAG 0

%{#
#include "ATS/ats_fat_inode.cats"
%}  // end of [%{#]


fun fat_sync_read ((*   *)): int = "ats_fat_sync_read"


