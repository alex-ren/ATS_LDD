//
// Author: Zhiqiang Ren (aren AT cs DOT bu DOT edu)
// Time: April 6th., 2011
//
(* ************** ************* *)

#define ATS_STALOADFLAG 0

%{#
#include "ATS/ats_fs_types.cats"
%}  // end of [%{]

(* ************** ************* *)
staload UN = "prelude/SATS/unsafe.sats"
staload Basics = "contrib/linux/basics.sats"

(* ************** ************* *)
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

typedef ncluster = [i: nat] int i
typedef ncluster_free = int (FAT_ENT_FREE)
typedef ncluster_bad = int (FAT_ENT_BAD)
typedef ncluster_notbad = 
    [i:nat| i <> FAT_ENT_BAD] int (i)

typedef ncluster_norm = 
    [i:nat|i <> FAT_ENT_FREE; i <> FAT_ENT_BAD; i <> FAT_ENT_EOF] 
    int (i)

typedef ncluster_valid = 
    [i:nat| i <> FAT_ENT_BAD; i <> FAT_ENT_EOF] 
    int (i)

abst@ype nblock = $extype "sector_t"
viewtypedef file = $extype_struct "file_struct" of {
  empty = empty
}

sta blksz : int
sta clssz : int

prfun blksz_pos (): [blksz > 0] void
prfun clssz_pos (): [clssz > 0] void

(* ************** ************* *)

abst@ype errno_t (int) = int
typedef errno_t = [i: int] errno_t i

castfn int_of_errno (e: errno_t):<> [i: int] int i
castfn int1_of_errno1 {i: int} (e: errno_t i):<> int i

castfn errno1_of_int1 {i:int} (i: int i):<> errno_t i

fun error_neg {e: int} (e: errno_t e): errno_t (~e)
  = "mac#atsfs_error_neg"

(* really ugly *)
dataprop error_ret (int, int) =
| {e: int | e <> 0} error_ret_pos (e, 0)
| {r: int} error_ret_any (0, r)

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
macdef EIO = $extval ([i: pos] errno_t i, "EIO")
macdef EINVAL = $extval ([i: pos] errno_t i, "EINVAL")

(* ************** ************* *)

absviewtype opt_ptr_error (a: viewtype, good: bool) = a

praxi opt_ptr_err_good {a:viewtype} (x: !opt_ptr_error (a, true) >> a):<prf> void

fun opt_ptr_err_bad {a:viewtype} (x: opt_ptr_error (a, false)): [e:int | e < 0] errno_t e
  = "mac#atsfs_opt_ptr_err_bad"

(* ************** ************* *)

#define MSDOS_NAME 11

(* ************** ************* *)

viewtypedef timespec = $extype_struct "timespec_struct" of {
  empty = empty
}

(* ************** ************* *)

abst@ype loff_t (ofs: int) = $extype "loff_t"
typedef loff_t = [ofs: int] loff_t (ofs)

(* ************** ************* *)

// todo specify reference count
viewtypedef inode = $extype_struct "inode_struct" of {
  empty = empty
}

// todo what is "own" exactly
viewtypedef inode_own = $extype_struct "inode_struct" of {
  empty = empty,
  i_size = [i:nat] loff_t i
}

absviewtype inode_ptr (ino: int(*inode no. *), l: addr) = 
  $extype "struct inode *"

viewtypedef inode_locked = $extype_struct "inode_struct" of {
  empty = empty
}

// absview inode_refcount (ino: int)
// absview inode_ctl (ino: int)  // We hold the inode exclusively.
absview inode_born (ino: int)
// praxi inode_giveup {ino: int} (pf: inode_ctl ino): void
// praxi inode_own_ref {ino: int} (pf: inode_ctl ino): inode_refcount ino
// praxi inode_new {ino: int} (pf: inode_born ino): inode_ctl ino

// todo this is too special
fun inode_init_time {ino: int} {l: addr} (
  pf: inode_born ino | inode: !inode_ptr (ino, l),
  ts: &timespec): void
  = "mac#atsfs_inode_init_time"


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
// todo

viewtypedef nameidata = $extype_struct "nameidata_struct" of {
  empty = empty
}

viewtypedef dentry = $extype_struct "dentry_struct" of {
  empty = empty
}

absviewtype dentry_ptr (l: addr) = $extype "struct dentry *"


viewtypedef fat_sb_info = $extype_struct "fat_sb_info_struct" of {
  empty = empty,
  sec_per_clus = usint,  // unsigned short
  // cluster_size = uint,  // unsigned int
  data_start = ulint  // unsigned long
}

viewtypedef fat_mount_options = $extype_struct "fat_mount_options_struct" of {
  empty = empty
}

(* ************** ************* *)

fun inode_own2inode (
  node: &inode_own
) : [l:agz] ($UN.viewout (inode @ l) | ptr l)
= "mac#atsfs_inode_own2inode"

fun inode2fat_inode (
 node: &inode
) : [l:agz] ($UN.viewout (fat_inode @ l) | ptr l)
 = "mac#atsfs_inode2fat_inode"

fun inode2fat_inode_own (
 node: &inode_own
) : [l:agz] ($UN.viewout (fat_inode @ l) | ptr l)
 = "mac#atsfs_inode2fat_inode"

symintr inode2sb

fun inode2super_block (
 node: &inode
) : [l:agz] ($UN.viewout (super_block @ l) | ptr l)
 = "mac#atsfs_inode2super_block"

overload inode2sb with inode2super_block

fun inode_own2super_block (
 node: &inode_own
) : [l:agz] ($UN.viewout (super_block @ l) | ptr l)
 = "mac#atsfs_inode2super_block"

overload inode2sb with inode_own2super_block

fun inode_locked2super_block (
 node: &inode_locked
) : [l:agz] ($UN.viewout (super_block @ l) | ptr l)
 = "mac#atsfs_inode2super_block"

fun sb2fat_sb (
 sb: &super_block
) : [l:agz] ($UN.viewout (fat_sb_info @ l) | ptr l)
 = "mac#atsfs_sb2fat_sb"

fun fat_sb2fat_mount_options (sbi: &fat_sb_info):
  [l:agz] ($UN.viewout (fat_mount_options @ l) | ptr l)
  = "mac#atsfs_fat_sb2fat_mount_options"

absview super_block_locked

fun lock_super (sb: &super_block): (super_block_locked | void)
  = "mac#atsfs_lock_super"

fun unlock_super (pf: super_block_locked | sb: &super_block):<fun1> void
  = "mac#atsfs_unlock_super"

fun dentry_get_d_name (dentry: &dentry, n: & int? >> int n): 
  #[n:pos] [l: agz]($UN.viewout (bytes (n) @ l) | ptr l)
  = "mac#atsfs_dentry_get_d_name"

(* ******************* ********************* *)

castfn ulint_of_usint (i: usint): ulint
castfn nblock_of_ulint (i: ulint): nblock

(* ******************* ********************* *)

fun uptr_plus_size1 {l: addr} {m:nat} (
  l: $Basics.uptr l, m: size_t m): $Basics.uptr (l + m)
= "mac#atsfs_uptr_plus_size1"
overload + with uptr_plus_size1


fun cmp_loff_lt {m,n: int} (l: loff_t m, r: loff_t n): bool (m < n) 
= "mac#atsfs_cmp_loff_lt"
overload < with cmp_loff_lt

fun cmp_loff_gt {m,n: int} (l: loff_t m, r: loff_t n): bool (m > n) 
= "mac#atsfs_cmp_loff_gt"
overload > with cmp_loff_gt

fun cmp_loff_gte {m,n: int} (l: loff_t m, r: loff_t n): bool (m >= n) 
= "mac#atsfs_cmp_loff_gte"
overload >= with cmp_loff_gte

fun arith_loff_plus_loff {m,n: int} (l: loff_t m, r: loff_t n): loff_t (m + n)
= "mac#atsfs_arith_loff_plus_loff"
overload + with arith_loff_plus_loff

fun arith_loff_minus_loff {m,n: int} (l: loff_t m, r: loff_t n): loff_t (m - n)
= "mac#atsfs_arith_loff_minus_loff"
overload - with arith_loff_minus_loff

fun arith_loff_minus_size {m,n: int} (l: loff_t m, r: size_t n): loff_t (m - n)
= "mac#atsfs_arith_loff_minus_size"
overload - with arith_loff_minus_size

fun arith_size_minus_loff {m,n: int} (l: size_t m, r: loff_t n): loff_t (m - n)
= "mac#atsfs_arith_size_minus_loff"
overload - with arith_size_minus_loff

castfn size1_of_loff1 {m: int| m >=0} (l: loff_t m): size_t (m)

castfn loff1_of_size1 {m: int} (l: size_t m): loff_t (m)

castfn loff1_of_int1 {i: int} (i: int i): loff_t i
castfn int1_of_loff1 {i: int} (i: loff_t i): int i

castfn ulint1_of_size1 {i: nat} (i: size_t i): ulint i

(* *********** ************ *)

(* temporary use *)
castfn int_of_size1 {i: nat} (i: size_t i): int
castfn int_of_loff1 {i: nat} (i: loff_t i): int

(* *********** ************ *)

fun loff_ldiv_loff {num,den: nat| den > 0} (
  num: loff_t num, den: loff_t den, 
  q: &loff_t ? >> loff_t q,
  r: &loff_t ? >> loff_t r): 
  #[q, r, x: nat | x + r == num; r < den] (
  MUL (den, q, x) | loff_t q)
= "mac#atsfs_loff_ldiv_loff"

fun loff_mod_loff {m,n: nat|n > 0} (
  l: loff_t m, r: loff_t n): [k: nat | k < n] loff_t k
= "mac#atsfs_loff_mod_loff"
overload mod with loff_mod_loff

fun BUG_ON {b: bool} (b: bool b): [b == true] void
= "mac#BUG_ON"

fun IS_ERROR {a:viewtype} {b: bool} (ptr: !opt_ptr_error (a, b)): bool b
  = "mac#atsfs_IS_ERROR"

fun d_instantiate {l:addr} {ino: int} (
  dentry: &dentry, inode: inode_ptr (ino, l)): void
  = "mac#atsfs_d_instantiate"




