//
// Author: Zhiqiang Ren (aren AT cs DOT bu DOT edu)
// Time: April 6th., 2011
//
(* ****** ****** *)

#define ATS_STALOADFLAG 0

%{#
#include "ATS/ats_fat_file.cats"
%}  // end of [%{#]

staload UN = "prelude/SATS/unsafe.sats"

staload Basics = "contrib/linux/basics.sats"

// has been defined somewhere, where?
// abst@ype ssize_t = $extype "ssize_t"

abst@ype loff_t (ofs: int) = $extype "loff_t"
typedef loff_t = [ofs: int] loff_t (ofs)

(*
/* bad cluster mark */
#define BAD_FAT12	0xFF7
#define BAD_FAT16	0xFFF7
#define BAD_FAT32	0x0FFFFFF7

/* standard EOF */
#define EOF_FAT12	0xFFF
#define EOF_FAT16	0xFFFF
#define EOF_FAT32	0x0FFFFFFF

#define FAT_ENT_FREE	(0)
#define FAT_ENT_BAD	(BAD_FAT32)
#define FAT_ENT_EOF	(EOF_FAT32)
*)

datasort
cluster_kind =
 | CKnorm
 | CKfree
 | CKend
 | CKbad
// todo how to specify both norm and free

sortdef ck = cluster_kind

abst@ype ncluster (ck) = int
typedef ncluster = [k:ck] ncluster (k)

abst@ype nblock = $extype "sector_t"
viewtypedef file = $extype_struct "file_struct" of {
  empty = empty
}

viewtypedef inode = $extype_struct "inode_struct" of {
  empty = empty
}

viewtypedef inode_own = $extype_struct "inode_struct" of {
  empty = empty,
  i_size = loff_t
}

fun cmp_loff_lt {m,n: int} (l: loff_t m, r: loff_t m): bool (m < n) 
overload < with cmp_loff_lt

fun cmp_loff_gt {m,n: int} (l: loff_t m, r: loff_t m): bool (m > n) 
overload > with cmp_loff_lt

viewtypedef fat_inode = $extype_struct "fat_inode_info_struct" of {
  empty = empty
}

viewtypedef fat_inode_own = $extype_struct "fat_inode_info_struct" of {
  empty = empty,
  i_start = ncluster (CKnorm)
}

(*
ssize_t
ats_fat_sync_read(struct file *filp,
 char __user *buf, size_t len, loff_t *ppos)
{
 return 0;
}
*)


fun fat_sync_read
 {l:addr}
 {n:nat} 
 {ofs:nat} (
 pfbuf: !bytes(n) @ l
, file: &file
, p: $Basics.uptr l
, n: size_t (n)
, pos: &loff_t (ofs) >> loff_t (ofs + max(0,n1))
) : #[n1:int | n1 <= n] ssize_t (n1) = "atsfs_fat_sync_read"

fun file2inode (
 file: &file
) : [l:addr] ($UN.viewout (inode @ l) | ptr l)
 = "mac#atsfs_file2inode"

fun file2inode_acquire (
 file: &file
) : [l:addr] (inode_own @ l | ptr l)
 = "mac#atsfs_file2inode_acquire"

fun file2inode_release {l: addr} (
 pf: inode_own @ l | p: ptr l
) : void
 = "mac#atsfs_file2inode_read_release"

fun inode2fat_inode (
 node: &inode
) : [l:addr] ($UN.viewout (fat_inode @ l) | ptr l)
 = "mac#atsfs_file2inode"

fun inode2fat_inode_own (
 node: &inode_own
) : [l:addr] ($UN.viewout (fat_inode @ l) | ptr l)
 = "mac#atsfs_file2inode_own"

fun get_first_cluster
 (node: &fat_inode): ncluster

fun offset2cluster (ofs: &loff_t): intGte (0)

fun get_next_cluster
 (cls: ncluster (CKnorm)): ncluster

fun get_nth_cluster {n:nat} (
 cls: ncluster (CKnorm), n: int n, n1: &int? >> int n1
) : #[n1:nat | n1 <= n] ncluster

fun ncluster2block (n: ncluster CKnorm): nblock

(*
sbread: read a block
*)

absviewtype bufferheadptr (l:addr)

fun bufferheadptr_free_null
 (bf: bufferheadptr null): void = "mac#atspre_ptr_free_null"
fun bufferheadptr_free {l:agz} (bf: bufferheadptr l): void

(*

*)

sta blksz : int

fun
get_blocksize (): int (blksz)

////
fun
bufferheadptr_get_buf {l:addr}(
 p: bufferheadptr l
) : (minus (bufferheadptr l, bytes_v (blksz) | ptr l))




