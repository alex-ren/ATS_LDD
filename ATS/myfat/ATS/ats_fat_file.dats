
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
 {l} {n} {ofs} (pfbuf , file , p , n , pos) = let
  val (v_inode | pinode) = file2inode_acquire (file)
  val filesz = (pinode->i_size): [sz:int] loff_t (sz)
in
  if pos > filesz then let
    val () = file2inode_release (v_inode | pinode)
  in
    $UN.cast {ssize_t(0)} (0)
  end else let
    

    val (vo_fnode | pfnode) = inode2fat_inode_own (!pinode)
    prval (pf, fpf) = viewout_decode (vo_fnode)
    prval () = fpf (pf)
    val () = file2inode_release (v_inode | pinode)
  in
    $UN.cast {ssize_t(0)} (0)
  end
end





