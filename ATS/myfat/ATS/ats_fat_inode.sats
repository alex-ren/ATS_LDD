//
// Author: Zhiqiang Ren (aren AT cs DOT bu DOT edu)
// Time: March 17th., 2011
//
(* ****** ****** *)

#define ATS_STALOADFLAG 0

%{#
#include "ATS/ats_fat_inode.cats"
%}  // end of [%{#]

staload AFT = "ats_fs_types.sats"
staload AFD = "ats_fat_dir.sats"
staload AMM = "ats_msdos_misc.sats"

(*
struct inode *fat_build_inode(struct super_block *sb,
	struct msdos_dir_entry *de, loff_t i_pos)
*)

fun fat_build_inode {ld: agz} (
  sb: & $AFT.super_block,
  de: ! $AFD.msdos_dir_entry_ptr ld, 
  i_pos: $AFT.loff_t):
  [l: addr] [ino: int] [b: bool] ($AFT.inode_born ino | 
  $AFT.opt_ptr_error ($AFT.inode_ptr (ino, l), b))





