staload "libats/SATS/ilistp.sats"

datasort file = //
sortdef clist = ilist

(* cannot out of bound *)
absprop SUBLIST (clist, int, int, clist)
(* cannot out of bound *)
absprop REPL (clist, int, clist, clist)

absprop content_p (file, clist)

(* read of length len *)
dataprop READ (file, int, int, clist) =
  {f: file} {ofs, len: int} {cs, ds: clist}
  READcons (f, ofs, len, ds) of (
  content_p (f, cs),
  SUBLIST (cs, ofs, len, ds))

(* write of length len *)
dataprop WRITE (file, int, clist, file) =
  {f1, f2: file} {ofs: nat} {cs, ds, es: clist}
  WRITEcons (f1, ofs, ds, f2) of (
  content_p (f1, cs),
  REPL (cs, ofs, ds, es),
  content_p (f2, es))

prfun lemma_write_read {f1,f2: file}{ds,es:clist} 
  {ofs, len: nat} (
  pf_w: WRITE (f1, ofs, ds, f2),
  pf_len: LENGTH (ds, len)):
  READ (f2, ofs, len, ds)

abstype strbuf (clist)
abstype File (file)

fun file_read {f: file} {cs: clist}
  {ofs, len, lenc: nat | ofs + len <= lenc} (
  pf_cs: content_p (f, cs), pf_len: LENGTH (cs, lenc) | 
  f: File (f), ofs: int ofs, len: int len):
  [ds: clist] (READ (f, ofs, len, ds) | strbuf (ds))

fun file_write {f1,f2: file} {cs, ds: clist} 
  {ofs, lenc, lend: nat | ofs + lenc <= lenc} (
  pf: content_p (f1, cs), pf_len: LENGTH (ds, lend) | 
  file: File f1, buf: strbuf (ds), ofs: int ofs):
  [f2:file] (WRITE (f1, ofs, ds, f2) | File f2)
  


////
datasort fid = // int
datasort fileid = // int

absview file (fid, ilist)
abstype File (fid)
abstype Inode (fileid)
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
absview fblock_mem (fileid, int(*fblk*), ilist)

absview file_block_map (fileid, int(*fblk*), int(*pblk*))

#define blk_sz 512

extern fun bwrite {blk(*block num*): int} {cs: ilist}
  (pf_len: LENGTH (cs, blk_sz) 
  | buf: !Buffer cs, blk: int blk): 
  (pblock_mem (blk, cs) | void)

extern prfun file_map_block {fblk, pblk: nat} {id:fileid} {cs: ilist}
  (vpf_pb: pblock_mem (pblk, cs), vpf_map: file_block_map (id, fblk, pblk)):
  fblock_mem (id, fblk, cs)

extern fun file_to_file {fid: fid} (f: File fid): [fileid: fileid] Inode fileid

abstype FAT
absview fat (ilist)









