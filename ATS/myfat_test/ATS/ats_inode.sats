//
// Author: Zhiqiang Ren (aren AT cs DOT bu DOT edu)
// Time: March 17th., 2011
//
(* ****** ****** *)

#define ATS_STALOADFLAG 0

viewtypedef inode
// ( m:int)
= $extype_struct "inode" of {
  empty = empty,
  _res = undefined_vt
}  // end of [inode]
  
viewtypedef super_block
// ( m:int)
= $extype_struct "super_block" of {
  empty = empty,
  _res = undefined_vt
}  // end of [super_block]

fun new_inode (sb: super_block): inode = "ats_new_node"
fun new_node_impl (sb: super_block): inode = "new_node"


