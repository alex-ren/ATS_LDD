//
// Author: Zhiqiang Ren (aren AT cs DOT bu DOT edu)
// Time: May 28th., 2011
//
/* ****** ****** */

#define ATS_STALOADFLAG 0

staload AFT = "ats_fs_types.sats"
staload AFC = "ats_fat_cache.sats"

absviewtype msdos_dir_entry_ptr(l:addr) = $extype "struct msdos_dir_entry *"

viewtypedef fat_slot_info = $extype_struct "fat_slot_info_struct" of {
  empty = empty
  ,de = [l:agz] msdos_dir_entry_ptr (l)
  ,bh = [lbuf:agz] $AFC.bufferheadptr lbuf
  ,i_pos = $AFT.loff_t
}

absview no_such_name

fun fat_scan {l:agz} (pf_name: !bytes ($AFT.MSDOS_NAME) @ l |
  dir: & $AFT.inode_locked, name: ptr l, sinfo: &fat_slot_info ? >> fat_slot_info): 
  [e: nat] (option_v (no_such_name, e <> 0) |  $AFT.errno_t e)

fun fat_slot_info_clear (sinfo: &fat_slot_info >> fat_slot_info ?): void


