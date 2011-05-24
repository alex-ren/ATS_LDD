
//
// Author: Zhiqiang Ren (aren AT cs DOT bu DOT edu)
// Time: April 6th., 2011
//
/* ****** ****** */

#define ATS_DYNLOADFLAG 0


(* This must be staloaded first *)
staload "myheader.sats"

staload "linux/SATS/kernel.sats"

(* ***** ***** ***** *)
staload "ats_fat_file.sats"
staload UN = "prelude/SATS/unsafe.sats"
macdef viewout_decode = $UN.viewout_decode


staload
UACC = "contrib/linux/asm/SATS/uaccess.sats"
macdef clear_user = $UACC.clear_user
macdef copy_to_user = $UACC.copy_to_user
macdef copy_from_user = $UACC.copy_from_user

implement fat_sync_read
 {l} {n} {ofs} (pfbuf | file , p , n , pos) = let

  val () = printk (KERN_INFO "fat_sync_read\n", @())

  val (v_inode | pinode) = file2inode_acquire (file)
  stavar filesz: int
  val filesz = (pinode->i_size): loff_t filesz
in
  if (pos >= filesz) = true then let
    val () = file2inode_release (v_inode | pinode)
  in
    ssize1_of_int1 (~int_of (EIO))
  end else let
    stavar len: int
    val len = (filesz - pos): loff_t len
    val len = min (size1_of_loff1 (len), n)

    (* get all the views needed *)
    val (vo_inode | pinode') = inode_own2inode (!pinode)
    prval (pf_inode, fpf_inode) = viewout_decode (vo_inode)

    val (vo_fnode | pfnode) = inode2fat_inode_own (!pinode)
    prval (pf_fnode, fpf_fnode) = viewout_decode (vo_fnode)

    val (vo_sb | psb) = inode2sb (!pinode)
    prval (pf_sb, fpf_sb) = viewout_decode (vo_sb)

    val (vo_sbi | psbi) = sb2fat_sb (!psb)
    prval (pf_sbi, fpf_sbi) = viewout_decode (vo_sbi)
    // end of [getting view]

    val clssz = get_clustersize (!psbi)
    prval () = clssz_pos ()

    var cls_ofs: loff_t
    var cls_num: loff_t
    val (xx | yy) = loff_ldiv_loff (pos, loff1_of_size1 (clssz), cls_num, cls_ofs)

    val cls_first = get_first_cluster (!pfnode)
    // val () = BUG_ON (cls_first <> FAT_ENT_FREE)
    
    var nth_cls: int
    val ncls = get_nth_cluster (!psb, cls_first, int1_of_loff1 (cls_num), nth_cls)
  in
    if loff1_of_int1 (nth_cls) < cls_num then let
      (* return all the views *)
      prval () = fpf_fnode (pf_fnode)
      prval () = fpf_sb (pf_sb)
      prval () = fpf_sbi (pf_sbi)
      prval () = fpf_inode (pf_inode)
      // end of [return views]
  
      val () = file2inode_release (v_inode | pinode)
    in
      ssize1_of_int1 (~int_of (EIO))
    end else let
      var accu = 0: size_t 0
      var err = errno_of_int 0
      val (pf_ret | ret) = copy_clusters_impl (v_inode, pfbuf |
        !psb, p, cls_ofs, len, ncls, clssz, accu, err) 
  
      val () = pos := pos + loff1_of_size1 (ret)
  
      (* return all the views *)
      prval () = fpf_fnode (pf_fnode)
      prval () = fpf_sb (pf_sb)
      prval () = fpf_sbi (pf_sbi)
      prval () = fpf_inode (pf_inode)
      // end of [return views]
  
      val () = file2inode_release (v_inode | pinode)
    in
      if ret > 0 then ssize1_of_size1 (ret)
      else ssize1_of_int1 (~int_of_errno (err))
    end
  end
end

// fun copy_clusters_impl
//   {pinode:addr}
//   {pbuf: addr} 
//   {n: nat}
//   {ofs: nat | ofs < clssz}   // offset in current cluster
//   {len: pos | len <= n}  // total length to be copied
//   {accu: nat}
//   {e: nat} (
//   pf_inode_r: !inode_own @ pinode,
//   pf_buf: !bytes(n) @ pbuf |
//   sb: &super_block,
//   pbuf: $Basics.uptr pbuf,
//   ofs: loff_t (ofs),
//   len: size_t (len),
//   ncls: ncluster_norm,
//   clssz: size_t (clssz),
//   accu: size_t (accu),
//   err: &(errno_t e) >> errno_t  // last error
//   ): #[accu': int | accu' >= accu; accu' - accu <= len] 
//   (error_ret (e, accu' - accu) | size_t (accu'))
implement copy_clusters_impl
  {pinode}
  {pbuf} {n}
  {ofs} {len} 
  {accu} {e} (
  pf_inode_r, pf_buf | sb, pbuf, ofs, len, ncls, clssz, accu, err) =
  if int_of (err) <> 0 then (error_ret_pos () | accu)
  else let
    val len1 = size1_of_loff1 (clssz - ofs)
    stavar len1: int
    val len1 = min (len, len1): size_t len1
    val ret = copy_cluster_impl (pf_inode_r, pf_buf | sb, pbuf, ofs, len1, ncls, err)
    val accu = accu + ret
  in
    if int_of (err) <> 0 then (error_ret_any | accu)
    else let
      var nxt_cls: ncluster?
      val ret_err = get_next_cluster (sb, ncls, nxt_cls)
    in
      if int_of (ret_err) <> 0 then let
        prval () = opt_unnone {ncluster_notbad} (nxt_cls)
      in
        (error_ret_any | accu) 
      end else let
        prval () = opt_unsome {ncluster_notbad} (nxt_cls)
      in
        if nxt_cls = FAT_ENT_FREE || nxt_cls = FAT_ENT_EOF
        then let
          val () = err := EIO
        in
          (error_ret_any | accu)
        end
        else if len - len1 > 0 then let 
          prval (pf_buf1, pf_buf2) = bytes_v_split {n} {len1} (pf_buf)
          val (pf_ret | ret) = copy_clusters_impl (pf_inode_r, pf_buf2 | 
                sb, pbuf + len1, loff1_of_int1 (0), 
                len - len1, nxt_cls, clssz, accu, err)
          prval () = pf_buf := bytes_v_unsplit (pf_buf1, pf_buf2)
        in
          (error_ret_any | ret)
        end else
          (error_ret_any | accu)
      end
    end
  end

implement copy_cluster_impl
  {pinode}
  {pbuf} 
  {n}
  {ofs}   // offset in cluster
  {len} 
  {e} (
  pf_inode_r,
  pf_buf |
  sb,
  pbuf,
  ofs,
  len,
  ncls,
  err
  ) =
  if int_of (err) <> 0 then size1_of_int1 0
  else let
    val (vo_sbi | psbi) = sb2fat_sb (sb)
    prval (pf_sbi, fpf_sbi) = viewout_decode (vo_sbi)
    val pnblk = ncluster2block (!psbi, ncls)
    prval () = fpf_sbi (pf_sbi)

    val blksz = get_blocksize (sb)
    prval () = blksz_pos ()

    var blk_ofs: loff_t
    var blk_num: loff_t
    val (_ | _) = loff_ldiv_loff (ofs, loff1_of_size1 (blksz), blk_num, blk_ofs)

    val pnblk = pnblk + blk_num

    val retsz = copy_phyblocks_impl (
      pf_inode_r, pf_buf | sb, pbuf, blk_ofs, len, pnblk, blksz, 0, err)
  in
    retsz
  end

implement copy_phyblocks_impl
  {pinode}
  {pbuf} 
  {n}
  {ofs}   // offset in current block
  {len}  // total length to be copied
  {accu} 
  {e} (
  pf_inode_r,
  pf_buf |
  sb,
  pbuf,
  ofs,
  len,
  pnblk,  // starting number of physical block
  blksz,
  accu,
  err  // last error
  ) =
  if int_of (err) <> 0 then accu
  else let
    val len1 = size1_of_loff1 (blksz - ofs)
    stavar len1: int
    val len1 = min (len, len1): size_t len1
    val ret = copy_phyblock_impl (pf_inode_r, pf_buf | sb, pbuf, ofs, len1, pnblk, err)
    val accu = accu + ret
  in
    if len > len1 then let
      val pnblk' = pnblk + loff1_of_int1 (1)
      prval (pf_buf1, pf_buf2) = bytes_v_split {n} {len1} (pf_buf)
      val ret = copy_phyblocks_impl (pf_inode_r, pf_buf2 | 
        sb, pbuf + len1, loff1_of_int1 (0), len - len1, pnblk', blksz, accu, err)
      prval () = pf_buf := bytes_v_unsplit (pf_buf1, pf_buf2)
    in
      ret
    end else accu
  end

implement copy_phyblock_impl
  {pinode}
  {pbuf} 
  {n}
  {ofs}   // offset in cluster
  {len} (
  pf_inode_r,
  pf_buf |
  sb,
  pbuf,
  ofs,
  len,
  nblk,
  err
  ) = let
  var bh: bufferheadptr (null) ?
  val p = sbread (sb, nblk, bh)
in
  if p > null then let
    prval () = opt_unsome (bh)
    val (vo | pblock) = bufferheadptr_get_buf (bh)
    prval (pf, fpf) = viewout_decode (vo)
    
    prval (pf1, pf2) = bytes_v_split {blksz}{ofs} (pf)
    val nleft = copy_to_user (pf_buf | 
      pbuf, !(pblock + int1_of_loff1 (ofs)), ulint1_of_size1 (len))
    prval pf = bytes_v_unsplit (pf1, pf2)
    val () = err := (if size1_of_ulint1 (nleft) < len then EIO else err)

    prval () = fpf (pf)
    val () = bufferheadptr_free (bh)
  in
    size1_of_ulint1 (nleft)
  end else let
    val () = err := EIO
    prval () = opt_unnone (bh)
  in
    size1_of_int1 (0)
  end
end

(*fun ncluster2block (sbi: &fat_sb_info, n: ncluster CKnorm): nblock*)
implement ncluster2block (sbi, n) = let
  val k = ulint_of_int (n - (FAT_START_ENT))  * 
    ulint_of_usint (sbi.sec_per_clus) + sbi.data_start
in
  nblock_of_ulint (k)
end

// fun get_nth_cluster {n:nat} (
//  cls: ncluster_norm, n: int n, n1: &int? >> int n1
// ) : #[n1:nat | n1 <= n] ncluster_norm
implement get_nth_cluster {n} (sb, cls, n, n1) = let
  fun loop {n, n1: nat} (sb: &super_block,
    cls: ncluster_norm, n: int n, n1: &int n1>> int n2
  ): #[n2: nat | n2 >= n1; n2 <= n1 + n] ncluster_norm =
    if n = 0 then cls
    else let
      var nxt_cls: ncluster?
      val ret_err = get_next_cluster (sb, cls, nxt_cls)
    in
      if int_of (ret_err) <> 0 then let
        prval () = opt_unnone {ncluster_notbad} (nxt_cls)
      in
        cls
      end else let
        prval () = opt_unsome {ncluster_notbad} (nxt_cls)
      in
        if nxt_cls = FAT_ENT_FREE || nxt_cls = FAT_ENT_EOF then cls
        else let
          val () = n1 := n1 + 1
        in
          loop (sb, cls, n - 1, n1)
        end
      end
    end

  val () = n1 := 0
in
  loop (sb, cls, n, n1)
end

       






