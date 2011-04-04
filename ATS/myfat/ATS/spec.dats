staload "libats/SATS/ilistp.sats"

datasort inode = //
sortdef clist = ilist

absprop SUBLIST (clist, int, int, clist)
absprop REPL (clist, int, int, clist, clist)

absprop content_p (inode, clist)

dataprop READ (inode, int, int, clist) =
  {i: inode} {ofs, len: nat} {cs, ds: clist}
  READcons (i, ofs, len, ds) of (
  content_p (i, cs), 
  SUBLIST (cs, ofs, len, ds))

dataprop WRITE (inode, int, int, clist, clist) =
  {i: inode} {len, ofs: nat} {cs, ds, es: clist}
  WRITEcons (i, ofs, len, ds, es) of (
  content_p (i, cs),
  REPL (cs, ofs, len, ds, es))

praxi axi_repl_read {cs,ds,es,fs:clist} 
  {ofs, len: nat} (
  pf_repl: REPL (cs, ofs, len, ds, es),
  pf_sub: SUBLIST (es, ofs, len, fs)):
  SUBLIST (ds, 0, len, fs)

abstype strbuf (clist)
abstype Inode (inode)

extern fun inode_read {i: inode}
  {ofs, leni: nat} (inode: Inode (i), ofs: int ofs, leni: int leni):
  [ds: clist] [leno: int| leno <= leni] (
  READ (i, ofs, leno, ds) | strbuf (ds), int leno)

extern fun inode_write {i: inode} {cs, ds: clist} {ofs, leni: nat} (
  pf: content_p (i, cs) | inode: Inode i, buf: strbuf (ds), 
  ofs: int ofs, leni: int leni):
  [es: clist] [leno: int | leno <= leni](
  content_p (i, es), WRITE (i, ofs, leno, ds, es) | int leno)
  


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









