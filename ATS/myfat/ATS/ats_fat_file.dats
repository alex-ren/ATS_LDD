
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
    val len = filesz - n

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
    val ofs_cls = $UN.cast{uint}(pos) mod clssz
    val no_cls = $UN.cast{uint}(pos) / $UN.cast{uint}(clssz)

    val cls = get_first_cluster (!pfnode)
    var n1: int ?
    val cls = get_nth_cluster (cls, int1_of_int($UN.cast{int}(no_cls)), n1)

    


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


















