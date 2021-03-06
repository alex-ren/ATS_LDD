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

abst@ype errno_t = int
castfn int_of_errno (e: errno_t):<> [i:nat] int i
overload int_of with int_of_errno
castfn errno_of_int {i:nat} (i: int i):<> errno_t


(* ****** ****** *)
(* include/asm-generic/error-base.h *)
(*
#define	EPERM		 1	/* Operation not permitted */
#define	ENOENT		 2	/* No such file or directory */
#define	ESRCH		 3	/* No such process */
#define	EINTR		 4	/* Interrupted system call */
#define	EIO		 5	/* I/O error */
#define	ENXIO		 6	/* No such device or address */
#define	E2BIG		 7	/* Argument list too long */
#define	ENOEXEC		 8	/* Exec format error */
#define	EBADF		 9	/* Bad file number */
#define	ECHILD		10	/* No child processes */
#define	EAGAIN		11	/* Try again */
#define	ENOMEM		12	/* Out of memory */
#define	EACCES		13	/* Permission denied */
#define	EFAULT		14	/* Bad address */
#define	ENOTBLK		15	/* Block device required */
#define	EBUSY		16	/* Device or resource busy */
#define	EEXIST		17	/* File exists */
#define	EXDEV		18	/* Cross-device link */
#define	ENODEV		19	/* No such device */
#define	ENOTDIR		20	/* Not a directory */
#define	EISDIR		21	/* Is a directory */
#define	EINVAL		22	/* Invalid argument */
#define	ENFILE		23	/* File table overflow */
#define	EMFILE		24	/* Too many open files */
#define	ENOTTY		25	/* Not a typewriter */
#define	ETXTBSY		26	/* Text file busy */
#define	EFBIG		27	/* File too large */
#define	ENOSPC		28	/* No space left on device */
#define	ESPIPE		29	/* Illegal seek */
#define	EROFS		30	/* Read-only file system */
#define	EMLINK		31	/* Too many links */
#define	EPIPE		32	/* Broken pipe */
#define	EDOM		33	/* Math argument out of domain of func */
#define	ERANGE		34	/* Math result not representable */
*)
(* copy from contrib/linux/linux/SATS/errno.sats *)
macdef EIO = $extval (errno_t, "EFAULT")

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

(*
datasort
cluster_kind =
 | {n:int | n = 0 or nkk} CKnorm of (n)
 | CKfree
 | CKend
 | CKbad
*)
#define FAT_ENT_FREE  0
#define FAT_ENT_BAD  0x0FFFFFF7
#define FAT_ENT_EOF  0x0FFFFFFF

typedef ncluster = [i: int | i >= 0] int i
typedef ncluster_free = int (FAT_ENT_FREE)
typedef ncluster_bad = int (FAT_ENT_BAD)
typedef ncluster_notbad = 
    [i:int| i <> FAT_ENT_BAD] int (i)

typedef ncluster_norm = 
    [i:int|i <> FAT_ENT_FREE; i <> FAT_ENT_BAD; i <> FAT_ENT_EOF] 
    int (i)

typedef ncluster_valid = 
    [i:int| i <> FAT_ENT_BAD; i <> FAT_ENT_EOF] 
    int (i)

datasort
cluster_kind =
 | CKnorm  // todo norm means not bad and not free
 | CKfree
 | CKend
 | CKbad
// todo how to specify both norm and free

sortdef ck = cluster_kind

// abst@ype ncluster (ck) = int
// typedef ncluster = [k:ck] ncluster (k)

(* include/linux/msdos_fs.h *)
#define FAT_START_ENT 2  // todo which is better? this line or next
// #define FAT_START_ENT $extval (ncluster, "FAT_START_ENT")

abst@ype nblock = $extype "sector_t"
viewtypedef file = $extype_struct "file_struct" of {
  empty = empty
}

viewtypedef inode = $extype_struct "inode_struct" of {
  empty = empty
}

viewtypedef inode_own = $extype_struct "inode_struct" of {
  empty = empty,
  i_size = [i:nat] loff_t i
}

viewtypedef super_block = $extype_struct "super_block_struct" of {
  empty = empty
}

viewtypedef fat_inode = $extype_struct "fat_inode_info_struct" of {
  empty = empty
}

viewtypedef fat_inode_own = $extype_struct "fat_inode_info_struct" of {
  empty = empty,
  i_start = ncluster_valid
}

viewtypedef fat_sb_info = $extype_struct "fat_sb_info_struct" of {
  empty = empty,
  sec_per_clus = usint,  // unsigned short
  cluster_size = uint,  // unsigned int
  data_start = ulint  // unsigned long
  
}

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
 {n:nat} 
 {ofs:nat} (
 pfbuf: !bytes(n) @ l |
 file: &file
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
 = "mac#atsfs_file2inode_release"

fun inode_own2inode (
  node: &inode_own
) : [l:addr] ($UN.viewout (inode @ l) | ptr l)

fun inode2fat_inode (
 node: &inode
) : [l:addr] ($UN.viewout (fat_inode @ l) | ptr l)
 = "mac#atsfs_inode2fat_inode"

fun inode2fat_inode_own (
 node: &inode_own
) : [l:addr] ($UN.viewout (fat_inode @ l) | ptr l)
 = "mac#atsfs_inode2fat_inode_own"

symintr inode2sb

fun inode2super_block (
 node: &inode
) : [l:addr] ($UN.viewout (super_block @ l) | ptr l)
 = "mac#atsfs_inode2super_block"

overload inode2sb with inode2super_block

fun inode_own2super_block (
 node: &inode_own
) : [l:addr] ($UN.viewout (super_block @ l) | ptr l)
 = "mac#atsfs_inode2super_block"

overload inode2sb with inode_own2super_block

fun sb2fat_sb (
 sb: &super_block
) : [l:addr] ($UN.viewout (fat_sb_info @ l) | ptr l)
 = "mac#atsfs_sb2fat_sb"

fun get_first_cluster
 (node: &fat_inode): ncluster_valid

fun offset2cluster (ofs: &loff_t): intGte (0)

fun get_next_cluster
 (cls: ncluster_norm): ncluster

fun get_nth_cluster {n:nat} (
 cls: ncluster_norm, n: int n, n1: &int? >> int n1
) : #[n1:nat | n1 <= n] ncluster

fun ncluster2block (sbi: &fat_sb_info, n: ncluster_valid): nblock

castfn ulint_of_usint (i: usint): ulint
castfn nblock_of_ulint (i: ulint): nblock

// fun get_next_block (n: nblock): nblock

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
sta clssz : int

prfun blksz_pos (): [blksz > 0] void
prfun clssz_pos (): [clssz > 0] void

// unsigned long: see super_block in fs.h
fun
get_blocksize (sb: &super_block): size_t (blksz)

fun
get_clustersize (sbi: &fat_sb_info): size_t (clssz)

fun uptr_plus_size1 {l: addr} {m:nat} (
  l: $Basics.uptr l, m: size_t m): $Basics.uptr (l + m)
overload + with uptr_plus_size1


fun cmp_loff_lt {m,n: int} (l: loff_t m, r: loff_t n): bool (m < n) 
overload < with cmp_loff_lt

fun cmp_loff_gt {m,n: int} (l: loff_t m, r: loff_t n): bool (m > n) 
overload > with cmp_loff_gt

fun arith_loff_plus_loff {m,n: int} (l: loff_t m, r: loff_t n): loff_t (m + n)
overload + with arith_loff_plus_loff

fun arith_loff_minus_loff {m,n: int} (l: loff_t m, r: loff_t n): loff_t (m - n)
overload - with arith_loff_minus_loff

fun arith_loff_minus_size {m,n: int} (l: loff_t m, r: size_t n): loff_t (m - n)
overload - with arith_loff_minus_size

fun arith_size_minus_loff {m,n: int} (l: size_t m, r: loff_t n): loff_t (m - n)
overload - with arith_size_minus_loff

castfn size1_of_loff1 {m: int| m >=0} (l: loff_t m): size_t (m)

castfn loff1_of_size1 {m: int} (l: size_t m): loff_t (m)

castfn loff1_of_int1 {i: int} (i: int i): loff_t i

fun loff_mod_loff {m,n: nat|n > 0} (
  l: loff_t m, r: loff_t n): [k: nat | k < n] loff_t k
overload mod with loff_mod_loff

fun BUG_ON {b: bool} (b: bool b): [b == true] void

////
fun
bufferheadptr_get_buf {l:addr}(
 p: bufferheadptr l
) : (minus (bufferheadptr l, bytes_v (blksz) | ptr l))




