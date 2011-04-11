
//
// Author: Zhiqiang Ren (aren AT cs DOT bu DOT edu)
// Time: April 6th., 2011
//
/* ****** ****** */

#define ATS_DYNLOADFLAG 0


(* This must be staloaded first *)
staload "myheader.sats"

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
  val (v_inode | pinode) = file2inode_acquire (file)
  val filesz = (pinode->i_size): [sz:int] loff_t (sz)
in
  if pos > filesz then let
    val () = file2inode_release (v_inode | pinode)
  in
    $UN.cast {ssize_t(~1)} (~1)
  end else let
    val len = (filesz - n)

    val (vo_inode | pinode') = inode_own2inode (!pinode)
    prval (pf_inode, fpf_inode) = viewout_decode (vo_inode)

    val (vo_fnode | pfnode) = inode2fat_inode_own (!pinode)
    prval (pf_fnode, fpf_fnode) = viewout_decode (vo_fnode)

    val (vo_sb | psb) = inode2sb (!pinode)
    prval (pf_sb, fpf_sb) = viewout_decode (vo_sb)

    val (vo_fsb | pfsb) = sb2fat_sb (!psb)
    prval (pf_fsb, fpf_fsb) = viewout_decode (vo_fsb)
    
    extern fun copy_cluster
      {l: addr} 
      {n: nat}
      {ofs: nat}   // offset in cluster
      {len: nat | len <= n; ofs + len <= clssz}(
      pfbuf: !bytes(n) @ l |
      sb: &super_block,
      p: $Basics.uptr l,
      pos: &loff_t (ofs) >> loff_t (ofs + max(0, len1)),
      len: size_t (len),
      ncls: ncluster (CKnorm)
      ): #[len1:int | len1 <= len] ssize_t (len1)
      
    extern fun copy_clusters
      {l: addr} 
      {n: nat}
      {accu: nat}
      {len: nat | len <= n}
      {clsno: pos} (
      pf_mul: MUL (clssz, clsno, len),
      pfbuf: !bytes(n) @ l |
      sb: &super_block,
      p: $Basics.uptr l,
      accu: &loff_t (accu) >> loff_t (accu + len1),
      clsno: int clsno,
      ncls: ncluster (CKnorm)
      ): #[len1:nat | len1 <= len] void

    extern fun copy_block
      {l: addr} 
      {n: nat}
      {ofs: nat}   // offset in cluster
      {len: nat | len <= n; ofs + len <= blksz}(
      pfbuf: !bytes(n) @ l |
      sb: &super_block,
      p: $Basics.uptr l,
      pos: &loff_t (ofs) >> loff_t (ofs + max(0, len1)),
      len: size_t (len),
      ncls: ncluster (CKnorm)
      ): #[len1:int | len1 <= len] ssize_t (len1)

    val clssz = get_clustersize (!pfsb)
    prval () = clssz_pos ()
    var ofs_in_cls = $UN.cast{uint}(pos) mod clssz
    val a = 1: int 1

(*
    val len_in_cls = if ofs_in_cls + len > clssz then clssz - ofs_in_cls
                     else len
*)
(*
    val start_fcls = $UN.cast{uint}(pos) / $UN.cast{uint}(clssz)

    val first_pcls = get_first_cluster (!pfnode)
    var n1: int ?
    val start_pcls = get_nth_cluster (cls, int1_of_int($UN.cast{int}(no_cls)), n1)

    val len1 = copy_cluster (pfbuf | sb, p, ofs_in_cls, len_in_cls, start_pcls)
    // todo check the len1 < len_in_cls
    var accu
    

    // prval (pf1, pf2) = bytes_v_split {n} {j} (pf)
(*
    prval () = verify_constraint {n-j > 0} ()
*)
    val nleft = copy_to_user (pfbuf | p, !(pqtm+j), cnt_ul)
    // prval () = fpf (bytes_v_unsplit (pf1, pf2))
 *)   

    


    prval () = fpf_fnode (pf_fnode)
    prval () = fpf_sb (pf_sb)
    prval () = fpf_fsb (pf_fsb)
    prval () = fpf_inode (pf_inode)
    val () = file2inode_release (v_inode | pinode)
  in
    $UN.cast {ssize_t(0)} (0)
  end
end

(*fun ncluster2block (sbi: &fat_sb_info, n: ncluster CKnorm): nblock*)
implement ncluster2block (sbi, n) = let
  val k = ($UN.cast{uint}(n) - $UN.cast{uint}(FAT_START_ENT)) * 
    sbi.sec_per_clus + sbi.data_start
in
  $UN.cast{nblock}(k)
end


















