//
// Author: Zhiqiang Ren (aren AT cs DOT bu DOT edu)
// Time: March 17th., 2011
//
/* ****** ****** */

#define ATS_DYNLOADFLAG 0

%{^
#include <linux/fs.h>
%}

(* This must be staloaded first *)
staload "myheader.sats"

(* ***** ***** ***** *)
staload "ats_inode.sats"


(* fun new_inode (sb: super_block): inode = "ats_new_node" *)
implement new_inode (sb) = new_node_impl (sb)




