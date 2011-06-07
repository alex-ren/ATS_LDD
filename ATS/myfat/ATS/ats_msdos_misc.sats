//
// Author: Zhiqiang Ren (aren AT cs DOT bu DOT edu)
// Time: April 6th., 2011
//
(* ************** ************* *)

#define ATS_STALOADFLAG 0

staload AFT = "ats_fs_types.sats"
staload AFD = "ats_fat_dir.sats"

fun CURRENT_TIME_SEC (ts: & $AFT.timespec? >> $AFT.timespec): void

fun msdos_format_name {n:pos} {l1, l2:agz} (
  pf_name: !bytes (n) @ l1,
  pf_buf: ! bytes($AFT.MSDOS_NAME)? @ l2 >> 
    disj_v (bytes($AFT.MSDOS_NAME) @ l2, 
            bytes($AFT.MSDOS_NAME)? @ l2, 
            e)
  |
  name: ptr l1,
  n: int n,
  buf: ptr l2,
  mount_options: & $AFT.fat_mount_options
  ): #[e:int | e <= 0] $AFT.errno_t e
  = "ats_msdos_format_name"

fun msdos_add_entry {l: agz} (
  pf_not_exist: $AFD.no_such_name, 
  pf_name: !bytes($AFT.MSDOS_NAME) @ l
  | dir: & $AFT.inode_locked, 
  msdos_name: ptr l,
  is_dir: bool,
  is_hid: bool,
  cluster: $AFT.ncluster_valid,
  ts: & $AFT.timespec,
  sinfo: & $AFD.fat_slot_info): [e: int| e <=0] $AFT.errno_t e
  = "ats_msdos_add_entry"
  




