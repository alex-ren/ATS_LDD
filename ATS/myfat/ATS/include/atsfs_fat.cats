/************************************************************************/
/*                                                                      */
/*                         Applied Type System                          */
/*                                                                      */
/*                              Hongwei Xi                              */
/*                                                                      */
/************************************************************************/

/*
** ATS - Unleashing the Potential of Types!
**
** Copyright (C) 2002-2011 Hongwei Xi.
**
** ATS is free software;  you can  redistribute it and/or modify it under
** the terms of the GNU LESSER GENERAL PUBLIC LICENSE as published by the
** Free Software Foundation; either version 2.1, or (at your option)  any
** later version.
** 
** ATS is distributed in the hope that it will be useful, but WITHOUT ANY
** WARRANTY; without  even  the  implied  warranty  of MERCHANTABILITY or
** FITNESS FOR A PARTICULAR PURPOSE.  See the  GNU General Public License
** for more details.
** 
** You  should  have  received  a  copy of the GNU General Public License
** along  with  ATS;  see the  file COPYING.  If not, please write to the
** Free Software Foundation,  51 Franklin Street, Fifth Floor, Boston, MA
** 02110-1301, USA.
*/

/* ****** ****** */

/* author: Zhiqiang Ren (aren AT cs DOT bu DOT edu) */

/* ****** ****** */

#ifndef ATSFS_FS_CATS
#define ATSFS_FS_CATS


ATSinline()
ats_bool_type
atspre_eq_bool1_bool1 (ats_bool_type b1, ats_bool_type b2) {
    return (b1 == b2) ;
}

ATSinline ()
ats_bool_type 
atspre_add_bool1_bool1(ats_bool_type b1, ats_bool_type b2)
{
    return b1 + b2;
}

ATSinline()
ats_bool_type
atspre_add_bool_bool (
  ats_bool_type b1, ats_bool_type b2
) {
  if (b1) { return ats_true_bool ; } else { return b2 ; }
} // end of [atspre_add_bool_bool]

ATSinline()
ats_bool_type
atspre_mul_bool_bool (
  ats_bool_type b1, ats_bool_type b2
) {
  if (b1) { return b2 ; } else { return ats_false_bool ; }
} // end of [atspre_mul_bool_bool]


ATSinline()
ats_bool_type
atspre_eq_byte_byte
  (ats_byte_type b1, ats_byte_type b2) { return (b1 == b2) ; }
// end of [atspre_eq_byte_byte]

ATSinline()
ats_bool_type
atspre_neq_byte_byte
  (ats_byte_type b1, ats_byte_type b2) { return (b1 != b2) ; }
// end of [atspre_neq_byte_byte]


#endif


