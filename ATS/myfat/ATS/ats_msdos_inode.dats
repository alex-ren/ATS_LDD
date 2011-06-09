

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
  val () = printk (KERN_INFO "atsfs: msdos_create 0001\n", @())
  // lock the super block
  val (vo_sb | psb) = inode_locked2super_block (dir)
  val () = printk (KERN_INFO "atsfs: msdos_create 0002\n", @())
  prval (pf_sb, fpf_sb) = viewout_decode (vo_sb)
  val (pf_sb_lock | ()) = lock_super (!psb)
  val () = printk (KERN_INFO "atsfs: msdos_create 0003\n", @())

  var len: int ?
  val (vo_d_name | d_name) = dentry_get_d_name (dentry, len)
  val () = printk (KERN_INFO "atsfs: msdos_create 0004\n", @())
  prval (pf_d_name, fpf_d_name) = viewout_decode (vo_d_name)

  // var !p_buf with pf_buf = @[byte][MSDOS_NAME]()

  // get mount options
  val (vo_sbi | psbi) = sb2fat_sb (!psb)
  prval (pf_sbi, fpf_sbi) = viewout_decode (vo_sbi)
  val (vo_mount_opt | p_mount_opt) = fat_sb2fat_mount_options (!psbi)
  val () = printk (KERN_INFO "atsfs: msdos_create 0006\n", @())
  prval (pf_mopt, fpf_mopt) = viewout_decode (vo_mount_opt)
  prval () = fpf_sbi (pf_sbi)  // no use of sbi any more

  val err = msdos_format_name (pf_d_name, pf_buf | d_name, len, p_buf, !p_mount_opt)
  val () = printk (KERN_INFO "atsfs: msdos_create 0007\n", @())
  prval () = fpf_mopt (pf_mopt)

  // viewdef V = bytes(MSDOS_NAME)? @ p_buf  // no use now
in
  if // :(pf_buf: V) (* specify the master type *) =>   // no use now
    int1_of_errno1 (err) <> 0 then let
    val () = printk (KERN_INFO "atsfs: msdos_create 0010\n", @())
    prval InsRight_v (pf) = pf_buf
    prval () = pf_buf := pf

    prval () = fpf_d_name (pf_d_name)

    val () = unlock_super (pf_sb_lock | !psb)
    prval () = fpf_sb (pf_sb)
  in err end
  else let
    prval InsLeft_v (pf) = pf_buf
    val () = printk (KERN_INFO "atsfs: msdos_create 0020\n", @())

    (* foo vs .foo conflict *)
    val is_hid = (d_name->[0] = byte_of_char ('.')) && 
                 (p_buf->[0] <> byte_of_char ('.'))
    prval () = fpf_d_name (pf_d_name)

    var sinfo: fat_slot_info?
    val (pf_opt_exist | err) = fat_scan (pf | dir, p_buf, sinfo)
    val () = printk (KERN_INFO "atsfs: msdos_create 0021\n", @())
  in
    if (int1_of_errno1 (err) = 0) then let
      val () = printk (KERN_INFO "atsfs: msdos_create 0030\n", @())
      prval None_v () = pf_opt_exist

      val () = fat_slot_info_clear (sinfo)
  
      prval () = pf_buf := pf
  
      val () = unlock_super (pf_sb_lock | !psb)
      prval () = fpf_sb (pf_sb)
    in
      error_neg (EINVAL)
    end else let
      prval Some_v (pf_not_exist) = pf_opt_exist
      val () = printk (KERN_INFO "atsfs: msdos_create 0041\n", @())

      var ts: timespec?
      val () = CURRENT_TIME_SEC (ts)
      val () = printk (KERN_INFO "atsfs: msdos_create 0042\n", @())
      val err = msdos_add_entry (pf_not_exist, pf |
         dir, p_buf, false, is_hid, FAT_ENT_FREE, ts, sinfo)
      val () = printk (KERN_INFO "atsfs: msdos_create 0043\n", @())
      prval () = pf_buf := pf
    in
      if (int1_of_errno1 (err) <> 0) then let
        val () = printk (KERN_INFO "atsfs: msdos_create 0050\n", @())
        (* error return *)
        val () = fat_slot_info_clear (sinfo)
  
        val () = printk (KERN_INFO "atsfs: msdos_create 0051\n", @())
        val () = unlock_super (pf_sb_lock | !psb)
        val () = printk (KERN_INFO "atsfs: msdos_create 0052\n", @())
        prval () = fpf_sb (pf_sb)
      in
        err
      end else let
        
        val () = printk (KERN_INFO "atsfs: msdos_create 0061\n", @())
        val (pf_opt_new_inode | inode_ptr_err) = 
          fat_build_inode (!psb, sinfo.de, sinfo.i_pos)

        val () = printk (KERN_INFO "atsfs: msdos_create 0062\n", @())
        val () = fat_slot_info_clear (sinfo)
        val () = printk (KERN_INFO "atsfs: msdos_create 0063\n", @())
        val is_err = IS_ERROR (inode_ptr_err)
      in
        if is_err = true then let
          val () = printk (KERN_INFO "atsfs: msdos_create 0070\n", @())
          val err = opt_ptr_err_iserr (inode_ptr_err)
          val () = printk (KERN_INFO "atsfs: msdos_create 0071\n", @())
          prval None_v () = pf_opt_new_inode
    
          val () = unlock_super (pf_sb_lock | !psb)
          val () = printk (KERN_INFO "atsfs: msdos_create 0072\n", @())
          prval () = fpf_sb (pf_sb)
        in
          err
        end else let
          val () = printk (KERN_INFO "atsfs: msdos_create 0081\n", @())
          prval () = opt_ptr_err_isptr (inode_ptr_err)
          prval Some_v (pf_new_inode) = pf_opt_new_inode

          val () = inode_init_time (pf_new_inode | inode_ptr_err, ts)
          val () = printk (KERN_INFO "atsfs: msdos_create 0082\n", @())
          val () = d_instantiate (dentry, inode_ptr_err)
          val () = printk (KERN_INFO "atsfs: msdos_create 0083\n", @())

          val () = unlock_super (pf_sb_lock | !psb)
          val () = printk (KERN_INFO "atsfs: msdos_create 0084\n", @())
          prval () = fpf_sb (pf_sb)
        in
          errno1_of_int1 (0)
        end
      end
    end 
  end
end


  


