//
// Author: Zhiqiang Ren (aren AT cs DOT bu DOT edu)
// Time: April 6th., 2011
//
(* ****** ****** *)

#define ATS_STALOADFLAG 0

%{#
#include "ATS/ats_fat_file.cats"
%}  // end of [%{#]

(* ***************** ****************** *)
staload UN = "prelude/SATS/unsafe.sats"
staload Basics = "contrib/linux/basics.sats"

(* ***************** ****************** *)

staload AFT = "ats_fs_types.sats"

(* ***************** ****************** *)

(*
ssize_t
ats_fat_sync_read(struct file *filp,
 char __user *buf, size_t len, loff_t *ppos)
{
 return 0;
}
*)
// ssize_t is defined in prelude/SATS/sizetype.sats
fun fat_sync_read
 {l:addr}
 {n:pos} 
 {ofs:nat} (
 pfbuf: !bytes(n) @ l |
 file: &($AFT.file)
, p: $Basics.uptr l
, n: size_t (n)
, pos: &($AFT.loff_t (ofs)) >> ($AFT.loff_t (ofs + max(0,n1)))
) : #[n1:int | n1 <= n] ssize_t (n1) = "atsfs_fat_sync_read"

fun copy_clusters_impl
  {pinode:addr}
  {pbuf: addr} 
  {n: nat}
  {ofs: nat | ofs < $AFT.clssz}   // offset in current cluster
  {len: pos | len <= n}  // total length to be copied
  {accu: nat}
  {e: nat} (
  pf_inode_r: !($AFT.inode_own) @ pinode,
  pf_buf: !bytes(n) @ pbuf |
  sb: &($AFT.super_block),
  pbuf: $Basics.uptr pbuf,
  ofs: $AFT.loff_t (ofs),
  len: size_t (len),
  ncls: $AFT.ncluster_norm,
  clssz: size_t ($AFT.clssz),
  accu: size_t (accu),
  err: &($AFT.errno_t e) >> $AFT.errno_t pe // last error
  ): #[accu': int | accu' >= accu; accu' - accu <= len] #[pe: nat]
  ($AFT.error_ret (e, accu' - accu) | size_t (accu'))

fun copy_cluster_impl
  {pinode:addr}
  {pbuf: addr} 
  {n: nat}
  {ofs: nat}   // offset in cluster
  {len: pos | len <= n; ofs + len <= $AFT.clssz}
  {e: nat} (
  pf_inode_r: !($AFT.inode_own) @ pinode,
  pf_buf: !bytes(n) @ pbuf |
  sb: &($AFT.super_block),
  pbuf: $Basics.uptr pbuf,
  ofs: &($AFT.loff_t (ofs)),
  len: size_t (len),
  ncls: $AFT.ncluster_valid,
  err: &($AFT.errno_t e) >> $AFT.errno_t pe
  ): #[len1:nat | len1 <= len] #[pe: nat] size_t len1

fun copy_phyblocks_impl
  {pinode:addr}
  {pbuf: addr} 
  {n: nat}
  {ofs: nat | ofs < $AFT.blksz}   // offset in current block
  {len: pos | len <= n}  // total length to be copied
  {accu: nat} 
  {e: nat} (
  pf_inode_r: !($AFT.inode_own) @ pinode,
  pf_buf: !bytes(n) @ pbuf |
  sb: &($AFT.super_block),
  pbuf: $Basics.uptr pbuf,
  ofs: $AFT.loff_t (ofs),
  len: size_t (len),
  pnblk: $AFT.nblock,  // starting number of physical block
  blksz: size_t ($AFT.blksz),
  accu: size_t (accu),
  err: &($AFT.errno_t) e >> ($AFT.errno_t pe)  // last error
  ): #[accu': int | accu' >= accu; accu' - accu <= len] #[pe: nat] 
  size_t (accu')

fun copy_phyblock_impl
  {pinode:addr}
  {pbuf: addr} 
  {n: nat}
  {ofs: nat}   // offset in cluster
  {len: pos | len <= n; ofs + len <= $AFT.blksz} (
  pf_inode_r: !($AFT.inode_own) @ pinode,
  pf_buf: !bytes(n) @ pbuf |
  sb: &($AFT.super_block),
  pbuf: $Basics.uptr pbuf,
  ofs: &($AFT.loff_t (ofs)),
  len: size_t (len),
  nblk: $AFT.nblock,
  err: &($AFT.errno_t 0) >> $AFT.errno_t pe
  ): #[len1:nat | len1 <= len] #[pe: nat] size_t len1

(* ***************** ****************** *)
fun file2inode (
 file: &($AFT.file)
) : [l:addr] ($UN.viewout ($AFT.inode @ l) | ptr l)
 = "mac#atsfs_file2inode"

fun file2inode_acquire (
 file: & $AFT.file
) : [l:addr] ($AFT.inode_own @ l | ptr l)
 = "mac#atsfs_file2inode_acquire"

fun file2inode_release {l: addr} (
 pf: $AFT.inode_own @ l | p: ptr l
) : void
 = "mac#atsfs_file2inode_release"
(* ============================================= *)

fun get_first_cluster
 (node: & $AFT.inode): $AFT.ncluster_norm  // ncluster_valid
= "mac#atsfs_get_first_cluster"

(* cls: current physical cluster no. in the volume *)
(* C code should guarantee to ruturn sth. not bad *)
fun get_next_cluster
 (sb: & $AFT.super_block, cur_cls: $AFT.ncluster_norm, 
  nxt_cls: & $AFT.ncluster? >> opt($AFT.ncluster_notbad, e == 0)): #[e: nat] $AFT.errno_t (e)
= "mac#atsfs_get_next_cluster"

(* cls: starting cluster *)
(* n: steps *)
fun get_nth_cluster {n:nat} (
 sb: & $AFT.super_block, cls: $AFT.ncluster_norm, n: int n, n1: &int? >> int n1
) : #[n1:nat | n1 <= n] $AFT.ncluster_norm

fun ncluster2block (sbi: & $AFT.fat_sb_info, n: $AFT.ncluster_valid): $AFT.nblock

(* ***************** ****************** *)

