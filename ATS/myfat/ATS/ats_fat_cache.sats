//
// Author: Zhiqiang Ren (aren AT cs DOT bu DOT edu)
// Time: April 6th., 2011
//
/* ****** ****** */

#define ATS_STALOADFLAG 0

%{#
#include "ATS/ats_fat_cache.cats"
%}  // end of [%{#]

staload AFT = "ats_fs_types.sats"
staload UN = "prelude/SATS/unsafe.sats"

(* include/linux/msdos_fs.h *)
#define FAT_START_ENT 2  // todo which is better? this line or next
// #define FAT_START_ENT $extval (ncluster, "FAT_START_ENT")
(* ============================================= *)

(* ============================================= *)

viewtypedef buffer_head = $extype_struct "buffer_head_struct" of {
  empty = empty,
  b_data = [l: addr | l > null] (bytes ($AFT.blksz) @ l | ptr l)
}

// unsigned long: see super_block in fs.h
fun
get_blocksize (sb: & $AFT.super_block): size_t ($AFT.blksz)
 = "mac#atsfs_get_blocksize"

fun
get_clustersize (sbi: & $AFT.fat_sb_info): size_t ($AFT.clssz)
= "mac#atsfs_get_clustersize"

(* ******************* ********************* *)
absviewtype bufferheadptr (l:addr) = $extype "struct buffer_head *"


fun nblock_plus_loff_t {k: int} (nblk: $AFT.nblock, k: $AFT.loff_t k): $AFT.nblock
= "mac#atsfsnblock_plus_loff_t"
overload + with nblock_plus_loff_t

fun sbread (sb: & $AFT.super_block, nblk: $AFT.nblock, 
  bh: & bufferheadptr (null) ? >> opt (bufferheadptr l, l > null)): 
  #[l: addr] (ptr l)
= "mac#atsfs_sbread"

fun bufferheadptr_free {l:agz} (bh: bufferheadptr l): void
= "mac#atsfs_bufferheadptr_free"

fun bufferheadptr_get_buf {l: agz} (bh: !bufferheadptr l):
  [sz: agz] ($UN.viewout (bytes ($AFT.blksz) @ sz) | ptr sz)
= "mac#atsfs_bufferheadptr_get_buf"


