//
// Author: Zhiqiang Ren (aren AT cs DOT bu DOT edu)
// Time: May 20th., 2011
//

#ifdef ATS_FAT_FILE_H
#define ATS_FAT_FILE_H


#include <linux/math64.h>  // for do_div

ATSinline ()
int atsfs_cmp_loff_lt(loff_t left, loff_t right)
{
    return left < right;
} 

ATSinline ()
int atsfs_cmp_loff_gt(loff_t left, loff_t right)
{
    return left > right;
}

ATSinline ()
int atsfs_cmp_loff_gte(loff_t left, loff_t right)
{
    return left >= right;
}

ATSinline ()
loff_t atsfs_arith_loff_plus_loff(loff_t left, loff_t right)
{
    return left + right;
}

ATSinline ()
loff_t atsfs_arith_loff_minus_loff(loff_t left, loff_t right)
{
    return left - right;
}

ATSinline ()
loff_t atsfs_arith_loff_minus_size(loff_t l, size_t s)
{
    return l - s;
}

ATSinline ()
loff_t atsfs_arith_size_minus_loff(size_t s, loff_t l)
{
    return s - l;
}

ATSinline ()
loff_t atsfs_loff_ldiv_loff(loff_t num, loff_t den, loff_t *q, loff_t *r)
{
    // have to use do_div, otherwise gcc would generate __divdi3 for "/"
    // and __moddi3 for "%"

    loff_t num_temp = num;
    *r = do_div(num_temp, den);  // special usage of do_div
    *q = num_temp;

    // int num1 = (int)num;
    // int den1 = (int)den;
    // *q = num1 / den1;
    // *r = num1 % den1;
    return *q;
}

ATSinline ()
loff_t atsfs_loff_mod_loff(loff_t m, loff_t n)
{
    return m % n;
}

ATSinline ()
ats_ptr_type atsfs_uptr_plus_size1(ats_ptr_type l, ats_size_type m)
{
    return l + m;
}

ATSinline ()
sector_t atsfs_nblock_plus_loff_t(sector_t nblk, loff_t k)
{
    return nblk + k;
}


#endif



