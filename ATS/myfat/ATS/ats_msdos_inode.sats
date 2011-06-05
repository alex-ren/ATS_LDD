//
// Author: Zhiqiang Ren (aren AT cs DOT bu DOT edu)
// Time: May 31st., 2011
//
(* ********* ********* *)

#define ATS_STALOADFLAG 0

%{#
#include "ATS/ats_msdos_inode.cats"
%}  // end of [%{#]

(* ********* ********* *)

staload UN = "prelude/SATS/unsafe.sats"

staload Basics = "contrib/linux/basics.sats"

(* ********* ********* *)

staload AFT = "ats_fs_types.sats"

(* ********* ********* *)
fun msdos_create (dir: & $AFT.inode_locked, dentry: & $AFT.dentry, 
  mode: int, nd: & $AFT.nameidata): [i: int | i <= 0] $AFT.errno_t i = "atsfs_msdos_create"









