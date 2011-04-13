
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

extern fun copy_block
  {l: addr} 
  {n: nat}
  {ofs: nat}   // offset in block
  {len: nat | len <= n; ofs + len <= blksz}(
  pfbuf: !bytes(n) @ l |
  sb: &super_block,
  p: $Basics.uptr l,
  pos: loff_t (ofs),
  len: size_t (len),
  nblk: nblock
  ): #[len1:int | len1 <= len] ssize_t (len1)

extern fun copy_cluster
  {l: addr} 
  {n: nat}
  {ofs: nat}   // offset in cluster
  {len: nat | len <= n; ofs + len <= clssz}(
  pfbuf: !bytes(n) @ l |
  sb: &super_block,
  p: $Basics.uptr l,
  pos: &loff_t (ofs),
  len: size_t (len),
  ncls: ncluster_valid
  ): #[len1:int | len1 <= len] ssize_t (len1)

extern fun copy_cluster_impl
  {pbuf: addr} 
  {n: nat}
  {ofs: nat}   // offset in cluster
  {len: nat | len <= n; ofs + len <= clssz} (
  pf_buf: !bytes(n) @ pbuf |
  sb: &super_block,
  pbuf: $Basics.uptr pbuf,
  ofs: &loff_t (ofs),
  len: size_t (len),
  ncls: ncluster_valid,
  err: &errno_t
  ): #[len1:nat | len1 <= len] size_t len1

extern fun copy_clusters_impl
  {pinode:addr}
  {pbuf: addr} 
  {n: nat}
  {ofs: nat | ofs < clssz}   // offset in current cluster
  {len: nat | len <= n}  // total length to be copied
  {accu: nat} (
  pf_inode_r: !inode_own @ pinode,
  pf_buf: !bytes(n) @ pbuf |
  sb: &super_block,
  pbuf: $Basics.uptr pbuf,
  ofs: loff_t (ofs),
  len: size_t (len),
  ncls: ncluster_norm,
  clssz: size_t (clssz),
  accu: size_t (accu),
  err: &errno_t  // last error
  // ): #[accu', err': int | accu' >= accu; accu' - accu <= len; err' <=0] 
  ): #[accu': int | accu' >= accu; accu' - accu <= len] 
  size_t (accu')

implement copy_clusters_impl
  {pinode}
  {pbuf} {n}
  {ofs} {len} 
  {accu} (
  pf_inode_r, pf_buf | sb, pbuf, ofs, len, ncls, clssz, accu, err) =
  if int_of (err) <> 0 then accu
  else let
    val len1 = size1_of_loff1 (clssz - ofs)
    stavar len1: int
    val len1 = min (len, len1): size_t len1
    val ret = copy_cluster_impl (pf_buf | sb, pbuf, ofs, len1, ncls, err)
    val accu = accu + ret
  in
    if int_of (err) <> 0 then accu
    else let
      val ncls' = get_next_cluster (ncls)
    in
      if ncls' = FAT_ENT_FREE || ncls' = FAT_ENT_BAD || ncls' = FAT_ENT_EOF
      then let
        val () = err := EIO
      in
        accu
      end
      else let 
        prval (pf_buf1, pf_buf2) = bytes_v_split {n} {len1} (pf_buf)
        val ret = copy_clusters_impl (pf_inode_r, pf_buf2 | 
          sb, pbuf + len1, loff1_of_int1 (0), len - len1, ncls', clssz, accu, err)
        prval () = pf_buf := bytes_v_unsplit (pf_buf1, pf_buf2)
      in
        ret
      end
    end
  end

implement fat_sync_read
 {l} {n} {ofs} (pfbuf | file , p , n , pos) = let
  val (v_inode | pinode) = file2inode_acquire (file)
  stavar filesz: int
  val filesz = (pinode->i_size): loff_t filesz
in
  if (pos > filesz) = true then let
    val () = file2inode_release (v_inode | pinode)
  in
    ssize1_of_int1 (~int_of (EIO))
  end else let
    val len = (filesz - pos)
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
    val pos_cls = pos mod loff1_of_size1 (clssz)
    
    val ncls = get_first_cluster (!pfnode)
    val () = BUG_ON (ncls <> FAT_ENT_FREE)
    var accu = 0: size_t 0
    var err = errno_of_int 0
    val ret = copy_clusters_impl (v_inode, pfbuf |
      !psb, p, pos_cls, len, ncls, clssz, accu, err) 

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

(*fun ncluster2block (sbi: &fat_sb_info, n: ncluster CKnorm): nblock*)
implement ncluster2block (sbi, n) = let
  val k = ulint_of_int (n - (FAT_START_ENT))  * 
    ulint_of_usint (sbi.sec_per_clus) + sbi.data_start
in
  nblock_of_ulint (k)
end


















