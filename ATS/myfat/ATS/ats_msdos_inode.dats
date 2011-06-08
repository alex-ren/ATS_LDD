

//
// Author: Zhiqiang Ren (aren AT cs DOT bu DOT edu)
// Time: May 31st., 2011
//
/* ****** ****** */

#define ATS_DYNLOADFLAG 0

(* This must be staloaded first *)
staload "myheader.sats"

// for printk
staload "contrib/linux/linux/SATS/kernel.sats"

(* ********** ********** *)

staload "ats_msdos_inode.sats"
staload "ats_msdos_misc.sats"
staload "ats_fs_types.sats"
staload "ats_fat_dir.sats"
staload "ats_fat_inode.sats"

(* ********** ********** *)

staload UN = "prelude/SATS/unsafe.sats"
macdef viewout_decode = $UN.viewout_decode

(* ********** ********** *)

staload
UACC = "contrib/linux/asm/SATS/uaccess.sats"
macdef clear_user = $UACC.clear_user
macdef copy_to_user = $UACC.copy_to_user
macdef copy_from_user = $UACC.copy_from_user

(* ********** ********** *)

// fun msdos_create {l:agz} (pf_buf: !bytes($AFT.MSDOS_NAME)? @ l |
//   p_buf: ptr l, dir: & $AFT.inode_locked, dentry: & $AFT.dentry, 
//   mode: int, nd: & $AFT.nameidata): [i: int | i <= 0] $AFT.errno_t i 
implement msdos_create {p_buf} (pf_buf | p_buf, dir, dentry, modet, nd) = let
  // lock the super block
  val (vo_sb | psb) = inode_locked2super_block (dir)
  prval (pf_sb, fpf_sb) = viewout_decode (vo_sb)
  val (pf_sb_lock | ()) = lock_super (!psb)

  var len: int ?
  val (vo_d_name | d_name) = dentry_get_d_name (dentry, len)
  prval (pf_d_name, fpf_d_name) = viewout_decode (vo_d_name)

  // var !p_buf with pf_buf = @[byte][MSDOS_NAME]()

  // get mount options
  val (vo_sbi | psbi) = sb2fat_sb (!psb)
  prval (pf_sbi, fpf_sbi) = viewout_decode (vo_sbi)
  val (vo_mount_opt | p_mount_opt) = fat_sb2fat_mount_options (!psbi)
  prval (pf_mopt, fpf_mopt) = viewout_decode (vo_mount_opt)
  prval () = fpf_sbi (pf_sbi)  // no use of sbi any more

  val err = msdos_format_name (pf_d_name, pf_buf | d_name, len, p_buf, !p_mount_opt)
  prval () = fpf_mopt (pf_mopt)

  // viewdef V = bytes(MSDOS_NAME)? @ p_buf  // no use now
in
  if // :(pf_buf: V) (* specify the master type *) =>   // no use now
    int1_of_errno1 (err) <> 0 then let
    prval InsRight_v (pf) = pf_buf
    prval () = pf_buf := pf

    prval () = fpf_d_name (pf_d_name)

    val () = unlock_super (pf_sb_lock | !psb)
    prval () = fpf_sb (pf_sb)
  in err end
  else let
    prval InsLeft_v (pf) = pf_buf

    (* foo vs .foo conflict *)
    val is_hid = (d_name->[0] = byte_of_char ('.')) && 
                 (p_buf->[0] <> byte_of_char ('.'))
    prval () = fpf_d_name (pf_d_name)

    var sinfo: fat_slot_info?
    val (pf_opt_exist | err) = fat_scan (pf | dir, p_buf, sinfo)
  in
    if (int1_of_errno1 (err) = 0) then let
      prval None_v () = pf_opt_exist

      val () = fat_slot_info_clear (sinfo)
  
      prval () = pf_buf := pf
  
      val () = unlock_super (pf_sb_lock | !psb)
      prval () = fpf_sb (pf_sb)
    in
      error_neg (EINVAL)
    end else let
      prval Some_v (pf_not_exist) = pf_opt_exist

      var ts: timespec?
      val () = CURRENT_TIME_SEC (ts)
      val err = msdos_add_entry (pf_not_exist, pf |
         dir, p_buf, false, is_hid, FAT_ENT_FREE, ts, sinfo)
      prval () = pf_buf := pf
    in
      if (int1_of_errno1 (err) <> 0) then let
        (* error return *)
        val () = fat_slot_info_clear (sinfo)
  
        val () = unlock_super (pf_sb_lock | !psb)
        prval () = fpf_sb (pf_sb)
      in
        err
      end else let
        
        val (pf_opt_new_inode | inode_ptr_err) = 
          fat_build_inode (!psb, sinfo.de, sinfo.i_pos)

        val () = fat_slot_info_clear (sinfo)
        val is_ptr = IS_ERROR (inode_ptr_err)
      in
        if is_ptr = false then let
          val err = opt_ptr_err_bad (inode_ptr_err)
          prval None_v () = pf_opt_new_inode
    
          val () = unlock_super (pf_sb_lock | !psb)
          prval () = fpf_sb (pf_sb)
        in
          err
        end else let
          prval () = opt_ptr_err_good (inode_ptr_err)
          prval Some_v (pf_new_inode) = pf_opt_new_inode

          val () = inode_init_time (pf_new_inode | inode_ptr_err, ts)
          val () = d_instantiate (dentry, inode_ptr_err)

          val () = unlock_super (pf_sb_lock | !psb)
          prval () = fpf_sb (pf_sb)
        in
          errno1_of_int1 (0)
        end
      end
    end 
  end
end


  

