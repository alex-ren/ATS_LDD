
staload "libats/SATS/ilistp.sats"

datasort inode = //
sortdef clist = ilist

(* cannot out of bound *)
absprop SUBLIST (clist, int, int, clist)
(* can just write part of the list *)
absprop REPL (clist, int, int, clist, clist)

absprop content_p (inode, clist)

(* read of length len *)
dataprop READ (inode, int, int, clist) =
  {i: inode} {ofs, len: nat} {cs, ds: clist}
  READcons (i, ofs, len, ds) of (
  content_p (i, cs), 
  SUBLIST (cs, ofs, len, ds))

(* write of length len *)
dataprop WRITE (inode, int, int, clist, inode) =
  {i1, i2: inode} {len, ofs: nat} {cs, ds, es: clist}
  WRITEcons (i1, ofs, len, ds, i2) of (
  content_p (i1, cs),
  REPL (cs, ofs, len, ds, es),
  content_p (i2, es))

prfun lemma_write_read {i1,i2: inode}{ds,es:clist} 
  {ofs, len: nat} (
  pf_len: LENGTH (ds, len),
  pf_w: WRITE (i1, ofs, len, ds, i2)):
  READ (i2, ofs, len, ds)

abstype strbuf (clist)
abstype Inode (inode)

fun inode_read {i: inode}
  {ofs, leni: nat} (inode: Inode (i), ofs: int ofs, leni: int leni):
  [ds: clist] [leno: int| leno <= leni] (
  READ (i, ofs, leno, ds) | strbuf (ds), int leno)

fun inode_write {i1,i2: inode} {cs, ds: clist} 
  {ofs, leni, lend: nat | leni <= lend} (
  pf: content_p (i1, cs), pf_len: LENGTH (ds, lend) | 
  inode: Inode i1, buf: strbuf (ds), 
  ofs: int ofs, leni: int leni):
  [i2:inode] [es: clist] [leno: int | leno <= leni](
  content_p (i2, es), WRITE (i1, ofs, leno, ds, i2) | int leno, Inode i2)
  


////
datasort fid = // int
datasort inodeid = // int

absview file (fid, ilist)
abstype File (fid)
abstype Inode (inodeid)
absviewtype Buffer (ilist)

// ds is part of cs
absprop PART (ilist(*cs*), ilist(*ds*), int(*ofs*), int(*len*))

// replace part of cs with part of ds to get es
absprop REPL (ilist(*cs*), ilist(*ds*), int(*ofs*), int(*len*), ilist(*es*))

extern fun read_file {fid:fid} {off,len,flen:nat | off+len <= flen}
  {cs: ilist}
  (pf_fl: LENGTH (cs, flen), vpf_f_cs: !file (fid, cs) 
  | f: File (fid)):
  [ds: ilist][len': int | len' <= len] (LENGTH (ds, len'), PART (cs, ds, off, len') 
  | Buffer (ds))

extern fun write_file {fid:fid} {off: nat}{len: pos}
  {cs, ds: ilist}
  (vpf_f_cs: file (fid, cs), pf_len: LENGTH (ds, len) 
  | f: File (fid), buf: !Buffer (ds), len: int len):
  [es: ilist] [len': int | len' <= len] (REPL (cs, ds, off, len', es), file (fid, es) 
  | int len')

absview pblock_mem (int(*pblk*), ilist)
absview fblock_mem (inodeid, int(*fblk*), ilist)

absview inode_block_map (inodeid, int(*fblk*), int(*pblk*))

#define blk_sz 512

extern fun bwrite {blk(*block num*): int} {cs: ilist}
  (pf_len: LENGTH (cs, blk_sz) 
  | buf: !Buffer cs, blk: int blk): 
  (pblock_mem (blk, cs) | void)

extern prfun inode_map_block {fblk, pblk: nat} {id:inodeid} {cs: ilist}
  (vpf_pb: pblock_mem (pblk, cs), vpf_map: inode_block_map (id, fblk, pblk)):
  fblock_mem (id, fblk, cs)

extern fun file_to_inode {fid: fid} (f: File fid): [inodeid: inodeid] Inode inodeid

abstype FAT
absview fat (ilist)









