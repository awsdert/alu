#include "alu.h"
int_t alu__op1
(
	alu_t *alu
	, uint_t num
	, uint_t info
	, func_alu_reg_op1_t op1
)
{
	alu_reg_t NUM;
	
	alu_reg_init( alu, NUM, num, info );
	
	return op1( alu, NUM );
}

int_t alu__op2
(
	alu_t *alu
	, uint_t num
	, uint_t val
	, uint_t info
	, func_alu_reg_op2_t op2
)
{
	alu_reg_t NUM, VAL;
	
	alu_reg_init( alu, NUM, num, info );
	alu_reg_init( alu, VAL, val, info );
	
	return op2( alu, NUM, VAL );
}

int_t alu__op4
(
	alu_t *alu
	, uint_t num
	, uint_t val
	, uint_t reg
	, uint_t tmp
	, uint_t info
	, func_alu_reg_op4_t op4
)
{
	alu_reg_t NUM, VAL, REG, TMP;
	
	alu_reg_init( alu, NUM, num, info );
	alu_reg_init( alu, VAL, val, info );
	alu_reg_init( alu, REG, reg, info );
	alu_reg_init( alu, TMP, tmp, info );
	
	return op4( alu, NUM, VAL, REG, TMP );
}

int_t alu__shift
(
	alu_t *alu
	, uint_t num
	, uint_t val
	, func_alu_reg__shift_t _shift
	, func_alu_reg_shift_t shift
)
{
	alu_reg_t _num, _val;
	
	alu_reg_init( alu, _num, num, 0 );
	alu_reg_init( alu, _val, val, 0 );
	
	return shift( alu, _num, _val, _shift );
}

int_t alu___shift
(
	alu_t *alu
	, uint_t num
	, size_t bits
	, func_alu_reg__shift_t _shift
)
{
	int ret;
	uint_t tmp = 0;
	alu_reg_t _num, _tmp;
	
	ret = alu_get_reg_node( alu, &tmp, 0 );
	
	if ( ret == 0 )
	{
	
		alu_reg_init( alu, _num, num, 0 );
		alu_reg_init( alu, _tmp, tmp, 0 );
		
		ret = _shift( alu, _num, _tmp, bits );
		
		alu_rem_reg_node( alu, &tmp );
		
		return ret;
	}
	
	alu_error( ret );
	return ret;
}

int_t alu_reg_set( alu_t *alu, alu_reg_t num, bool fillwith )
{
	void *part;
	alu_bit_t n;
	
	/* Branching takes longer to complete so assume intended node is
	 * remainder of division, prevents segfault */
	 
	num.node %= alu_used( alu );
	part = alu_reg_data( alu, num );
	
	/* Force to either 1 or 0 */
	fillwith = !!fillwith;
	
	for
	(
		n = alu_bit_set_bit( part, num.from )
		; n.bit < num.upto
		; alu_bit_inc(&n)
	)
	{
		/* Set bit to 0 */
		*(n.ptr) &= ~(n.mask);
		
		/* Set bit based on fillwith */
		*(n.ptr) |= (fillwith * n.mask);
	}
	
	return 0;
}

int_t alu_set( alu_t *alu, uint_t num, bool fillwith )
{
	alu_reg_t _num;
	alu_reg_init( alu, _num, num, 0 );
	return alu_reg_set( alu, _num, fillwith );
}

int_t alu_reg_get_raw
(
	alu_t *alu
	, alu_reg_t src
	, void *raw
	, size_t size
	, uint_t info
)
{
	int ret;
	uint_t tmp;
	void *part;
	alu_reg_t TMP;
	
	if ( raw )
	{
		ret = alu_get_reg_node( alu, &tmp, size );
		
		if ( ret == 0 )
		{
			alu_reg_init( alu, TMP, tmp, info );
			TMP.upto = size * CHAR_BIT;
			
			alu_reg_mov( alu, TMP, src );
			
			part = alu_reg_data( alu, TMP );
			(void)memcpy( raw, part, size );
			
			alu_rem_reg_node( alu, &tmp );
			return 0;
		}
		
		alu_error( ret );
		return ret;
	}
		
	return alu_err_null_ptr("raw");
}

int_t alu_get_raw( alu_t *alu, uint_t num, size_t *raw )
{
	alu_reg_t _num;
	alu_reg_init( alu, _num, num, 0 );
	return alu_reg_get_raw( alu, _num, raw, sizeof(size_t), 0 );
}

int_t alu_reg_set_raw
(
	alu_t *alu
	, alu_reg_t num
	, void *raw
	, size_t size
	, uint_t info
)
{
	int ret;
	uchar_t *part;
	uint_t _tmp = 0;
	alu_reg_t tmp;
	
	size = HIGHEST( size, 1 );
	
	ret = alu_get_reg_node( alu, &_tmp, size );
	
	if ( ret == 0 )
	{	
		alu_reg_init( alu, tmp, _tmp, info );
		tmp.upto = size * CHAR_BIT;

		part = alu_reg_data( alu, tmp );
		memcpy( part, raw, size );
		
		ret = alu_reg_mov( alu, num, tmp );
		
		alu_rem_reg_node( alu, &_tmp );
		return ret;
	}
	
	alu_error( ret );
	return ret;
}

int_t alu_set_raw( alu_t *alu, uint_t num, size_t raw, uint_t info )
{
	alu_reg_t _num;
	alu_reg_init( alu, _num, num, 0 );
	return alu_reg_set_raw( alu, _num, &raw, info, sizeof(size_t) );
}

int alu_reg_mov(
	alu_t *alu,
	alu_reg_t dst,
	alu_reg_t src
)
{
	int ret;
	void *D, *S;
	alu_bit_t d, s;
	size_t ndiff, vdiff, upto;
	bool neg, NaN = false, Inf = false;
	alu_reg_t DEXP, DMAN, SEXP, SMAN;
	
	ret = alu_reg_clr( alu, dst );
	
	if ( ret == 0 )
	{
		dst.node %= alu_used( alu );
		src.node %= alu_used( alu );
		
		D = alu_reg_data( alu, dst );
		S = alu_reg_data( alu, src );
		
		/* Check for +/- */
		neg = alu_reg_below0( alu, src );
		
		if ( alu_reg_floating( src ) )
		{
			alu_reg_init( alu, SEXP, src.node, 0 );
			alu_reg_init( alu, SMAN, src.node, 0 );
			
			SEXP.upto = src.upto - 1;
			SEXP.from = src.from + src.mant;
			SMAN.upto = src.from + src.mant;
			SMAN.from = src.from;
			
			/* Check if should set NaN or Infinity */
			DEXP.upto = SEXP.upto;
			DEXP.from = SEXP.from;
			alu_reg_set_max( alu, DEXP );
			Inf = (alu_reg_cmp( alu, SEXP, DEXP ) == 0);
			
			if ( alu_reg_floating( dst ) )
			{
				alu_reg_init( alu, DEXP, dst.node, 0 );
				
				DEXP.upto = dst.upto - 1;
				DEXP.from = dst.from + dst.mant;
				DMAN.upto = dst.from + dst.mant;
				DMAN.from = dst.from;
				
				/* Set +/- */
				d = alu_bit_set_bit( D, DEXP.upto );
				*(d.ptr) &= ~(d.mask);
				*(d.ptr) |= (neg * d.mask);
				
				/* Simplify code and pass on duties to another instance, added
				 * bonus of setting Infinity if >= Exponent limit */
				alu_reg_mov( alu, DEXP, SEXP );
				
				ndiff = DMAN.upto - DMAN.from;
				vdiff = SMAN.upto - SMAN.from;
				
				/* Make sure we copy the upper bits of the src mantissa */
				SMAN.from = SMAN.upto - LOWEST( ndiff, vdiff );
				
				/* Whether it is NaN or a fittable number copying the mantissa
				 * is fine as long as we make sure it was not Infinity to begin
				 * with */
				return Inf
					? alu_reg_clr( alu, DMAN )
					: alu_reg_mov( alu, DMAN, SMAN );
			}
			else if ( Inf )
			{
				/* Infinity is not something an integer can hold, default to
				 * max of the integer, if -Infinity the not operation will flip
				 * to min of the integer */
				(void)alu_reg_set_max( alu, dst );
				return neg ? alu_reg_not( alu, dst ) : 0;
			}
			
			/* Mantissa is always bigger than exponent so it is safe to use
			 * as the temporary copy of number to shift */
			alu_reg__shr( alu, DEXP, DMAN, 1 );
			
			/* Check if exponent is negative - meaning the number is between
			 * 0 & 1, if it is then just set the integer to 0 */
			ret = alu_reg_cmp( alu, SEXP, DEXP );
			
			if ( ret < 0 )
				return alu_reg_clr( alu, dst );
			
			DEXP.from = dst.from;
			DEXP.upto = dst.upto;
			ndiff = dst.upto - dst.from;
			alu_reg_set_raw( alu, DEXP, &(ndiff), sizeof(size_t), 0 );
			
			ret = alu_reg_cmp( alu, DEXP, SEXP );
			
			/* Check if number is to big to fit in integer */
			if ( ret >= 0 )
				return alu_reg_set_max( alu, dst );
			
			// FIXME: Finish implementing moving alu_fpn_t to alu_int_t/alu_uint_t
			// if ( alu_reg_cmp( alu, EXP, BIAS ) >= 0 )
			return ENOSYS;
		}
		
		ndiff = dst.upto - dst.from;
		vdiff = src.upto - src.from;
		
		d = alu_bit_set_bit( D, dst.from );
		s = alu_bit_set_bit( S, src.from );
		upto = dst.from + LOWEST( ndiff, vdiff );
		
		for ( ; d.bit < upto; alu_bit_inc(&d), alu_bit_inc(&s) )
		{
			*(d.ptr) &= ~(d.mask);
			*(d.ptr) |= SET1IF( *(s.ptr) & s.mask, s.mask );
		}
		
		for ( ; d.bit < dst.upto; alu_bit_inc(&d) )
		{
			*(d.ptr) &= ~(d.mask);
			*(d.ptr) |= SET1IF( neg, d.mask );
		}
		
		return 0;
	}
	
	alu_error( ret );
	return ret;
}

int_t alu_mov( alu_t *alu, uint_t num, uint_t val )
{	
	alu_reg_t _num, _val;
	alu_reg_init( alu, _num, num, 0 );
	alu_reg_init( alu, _val, val, 0 );
	
	return alu_reg_mov( alu, _num, _val );
}

alu_bit_t alu_reg_end_bit( alu_t *alu, alu_reg_t num )
{
	alu_bit_t n;
	size_t b;
	void *part;
	
	num.node %= alu_used( alu );
	
	part = alu_reg_data( alu, num );
	n = alu_bit_set_bit( part, num.upto - 1 );
	b = n.bit * !( *(n.ptr) & n.mask );
	
	while ( b > num.from )
	{
		alu_bit_dec(&n);
		b = SET1IF( !( *(n.ptr) & n.mask ), n.bit );
	}
	
	return n;
}

alu_bit_t alu_end_bit( alu_t *alu, uint_t num )
{
	alu_reg_t _num;
	alu_reg_init( alu, _num, num, 0 );
	return alu_reg_end_bit( alu, _num );
}

int_t alu_reg_cmp(
	alu_t *alu
	, alu_reg_t num
	, alu_reg_t val
)
{
	int a, b, c = 0;
	alu_bit_t n = {0}, v = {0};
	size_t ndiff = 0, vdiff = 0;
	bool nNeg, vNeg;
	
	n = alu_reg_end_bit( alu, num );
	v = alu_reg_end_bit( alu, val );
	
	nNeg = alu_reg_signed( num ) & (n.bit == num.upto - 1);
	vNeg = alu_reg_signed( val ) & (v.bit == val.upto - 1);
	
	if ( nNeg != vNeg )
	{
		c = -1 + vNeg + vNeg;
		return c;
	}
	
	ndiff = n.bit - num.from;
	vdiff = v.bit - val.from;
	
	a = nNeg;
	b = vNeg;
	
	/* Deal with different sized integers */

	while ( ndiff < vdiff )
	{
		b = (*(v.ptr) & v.mask) ? 1 : 0;
		
		c = a - b;
		if ( c != 0 )
			return c;
		
		vdiff--;
		alu_bit_dec(&v);
	}
	
	while ( ndiff > vdiff )
	{
		a = (*(n.ptr) & n.mask) ? 1 : 0;
		
		c = a - b;
		if ( c != 0 )
			return c;
		
		ndiff--;
		alu_bit_dec(&n);
	}
	
	/* Finally compare what matches bit alignment */
	do
	{
		a = (*(n.ptr) & n.mask) ? 1 : 0;
		b = (*(v.ptr) & v.mask) ? 1 : 0;
		
		c = a - b;
		if ( c != 0 )
			return c;
			
		if ( !ndiff )
			break;
		
		ndiff--;
		alu_bit_dec(&n);
		alu_bit_dec(&v);
	}
	while ( 1 );
	
	return c;
}

int_t alu_reg_not( alu_t *alu, alu_reg_t num )
{
	alu_bit_t n, e;
	void *part;
	size_t i, stop, mask, mask_init, mask_last;
	
	if ( alu )
	{
		num.node %= alu_used( alu );
		part = alu_reg_data( alu, num );
		
		n = alu_bit_set_bit( part, num.from );
		e = alu_bit_set_bit( part, num.upto - 1 );
		
		mask = 0;
		mask = mask_init = mask_last = ~mask;
		
		mask_init <<= n.pos;
		mask_last <<= (bitsof(uintmax_t) - e.pos) - 1;
		
		stop = e.seg - n.seg;
		
		*(n.ptr) ^= SET2IF( stop, mask_init, mask_init & mask_last );
		
		for ( i = 1; i < stop; ++i ) n.ptr[i] = ~(n.ptr[i]);
		
		*(e.ptr) ^= SET1IF( stop, mask_last );
	
		return 0;
	}
	
	return alu_err_null_ptr( "alu" );
}

int_t alu_reg_inc( alu_t *alu, alu_reg_t num )
{
	bool is1;
	alu_bit_t n;
	void *part;
	
	num.node %= alu_used( alu );
	part = alu_reg_data( alu, num );
	
	n = alu_bit_set_bit( part, num.from );
	
	for ( ; n.bit < num.upto; alu_bit_inc(&n) )
	{
		is1 = !!(*(n.ptr) & n.mask);
		*(n.ptr) &= ~(n.mask);
		*(n.ptr) |= SET1IF( !is1, n.mask );
		num.upto = SET1IF( is1, num.upto );
	}
	
	return SET2IF( num.upto, EOVERFLOW, 0 );
}

int_t alu_reg_dec( alu_t *alu, alu_reg_t num )
{
	bool is1;
	alu_bit_t n;
	void *part;
	size_t mask = 0;
	mask = ~mask;
	
	num.node %= alu_used( alu );
	part = alu_reg_data( alu, num );
	
	n = alu_bit_set_bit( part, num.from );
	
	for ( ; n.bit < num.upto; alu_bit_inc(&n) )
	{
		is1 = !!(*(n.ptr) & n.mask);
		*(n.ptr) &= ~(n.mask);
		*(n.ptr) |= SET1IF( !is1, n.mask );
		num.upto = SET1IF( !is1, num.upto );
	}
	
	return SET2IF( num.upto, EOVERFLOW, 0 );
}

int_t alu_reg_add( alu_t *alu, alu_reg_t num, alu_reg_t val )
{
	bool carry = false, changed = false;
	alu_bit_t n, v = {0};
	size_t pos = 0;
	void *part;
	
	num.node %= alu_used( alu );
	val.node %= alu_used( alu );
	
	pos = alu_lowest_upto( num, val );
	
	part = alu_reg_data( alu, num );
	n = alu_bit_set_bit( part, num.from );
	
	part = alu_reg_data( alu, val );
	v = alu_bit_set_bit( part, val.from );
	
	for
	(
		; n.bit < pos
		; alu_bit_inc(&n), alu_bit_inc(&v)
	)
	{
		if ( carry )
		{
			if ( (*(n.ptr) & n.mask) )
				*(n.ptr) ^= n.mask;
			else
			{
				*(n.ptr) |= n.mask;
				carry = false;
			}
		}
		
		if ( *(v.ptr) & v.mask )
		{
			changed = true;
			if ( *(n.ptr) & n.mask )
			{
				*(n.ptr) ^= n.mask;
				carry = true;
			}
			else
				*(n.ptr) |= n.mask;
		}
	}
	
	if ( carry )
	{
		for
		(
			; n.bit < num.upto
			; alu_bit_inc(&n)
		)
		{
			if ( (*(n.ptr) & n.mask) )
				*(n.ptr) ^= n.mask;
			else
			{
				*(n.ptr) |= n.mask;
				carry = false;
				break;
			}
		}
	}
	
	return changed ? (carry ? EOVERFLOW : 0) : ENODATA;
}

int_t alu_reg_add_raw( alu_t *alu, alu_reg_t num, void *raw, size_t size )
{
	bool carry = false, changed = false;
	alu_bit_t n, v = {0};
	size_t pos = 0;
	void *part;
	
	num.node %= alu_used( alu );
	
	pos = LOWEST( size * CHAR_BIT, num.upto );
	
	part = alu_reg_data( alu, num );
	n = alu_bit_set_bit( part, num.from );
	v = alu_bit_set_bit( raw, 0 );
	
	for
	(
		; n.bit < pos
		; alu_bit_inc(&n), alu_bit_inc(&v)
	)
	{
		if ( carry )
		{
			if ( (*(n.ptr) & n.mask) )
				*(n.ptr) ^= n.mask;
			else
			{
				*(n.ptr) |= n.mask;
				carry = false;
			}
		}
		
		if ( *(v.ptr) & v.mask )
		{
			changed = true;
			if ( *(n.ptr) & n.mask )
			{
				*(n.ptr) ^= n.mask;
				carry = true;
			}
			else
				*(n.ptr) |= n.mask;
		}
	}
	
	if ( carry )
	{
		for
		(
			; n.bit < num.upto
			; alu_bit_inc(&n)
		)
		{
			if ( (*(n.ptr) & n.mask) )
				*(n.ptr) ^= n.mask;
			else
			{
				*(n.ptr) |= n.mask;
				carry = false;
				break;
			}
		}
	}
	
	return changed ? (carry ? EOVERFLOW : 0) : ENODATA;
}

int_t alu_reg_sub( alu_t *alu, alu_reg_t num, alu_reg_t val )
{
	bool carry = false, changed = false;
	alu_bit_t n, v;
	size_t pos = 0;
	void *part;
	
	num.node %= alu_used( alu );
	val.node %= alu_used( alu );
	
	pos = alu_lowest_upto( num, val );
	
	part = alu_reg_data( alu, num );
	n = alu_bit_set_bit( part, num.from );
	
	part = alu_reg_data( alu, val );
	v = alu_bit_set_bit( part, val.from );
	
	for
	(
		; n.bit < pos
		; alu_bit_inc(&n), alu_bit_inc(&v)
	)
	{
		if ( carry )
		{
			if ( (*(n.ptr) & n.mask) )
			{
				*(n.ptr) ^= n.mask;
				carry = false;
			}
			else
				*(n.ptr) |= n.mask;
		}
		
		if ( *(v.ptr) & v.mask )
		{
			changed = true;
			if ( *(n.ptr) & n.mask )
				*(n.ptr) ^= n.mask;
			else
			{
				*(n.ptr) |= n.mask;
				carry = true;
			}
		}
	}
	
	if ( carry )
	{
		for
		(
			; n.bit < num.upto
			; alu_bit_inc(&n)
		)
		{
			if ( (*(n.ptr) & n.mask) )
			{
				*(n.ptr) ^= n.mask;
				carry = false;
				break;
			}
			else
				*(n.ptr) |= n.mask;
		}
	}
	
	return changed ? (carry ? EOVERFLOW : 0) : ENODATA;
}

int_t alu_reg__shl( alu_t *alu, alu_reg_t num, alu_reg_t tmp, size_t by )
{
	alu_bit_t n = {0}, v;
	void *part;
	size_t diff, mant;
	
	if ( by )
	{
		diff = num.upto - num.from;
		
		/* Mantissa will use same bits but exponent will be greater */
		if ( alu_reg_floating( num ) )
		{
			mant = alu_man_dig( diff );
			num.upto--;
			num.from += mant;
			
			alu_set_raw( alu, tmp.node, by, 0 );
			return alu_reg_add( alu, num, tmp );
		}
		
		if ( by >= diff )
		{
			alu_reg_clr( alu, num );
			return 0;
		}
		
		num.node %= alu_used( alu );
		tmp.node %= alu_used( alu );
		tmp.from = num.from;
		tmp.upto = num.upto;
		
		/* We have the register so might as well */
		alu_reg_mov( alu, tmp, num );
		alu_reg_clr( alu, num );
		
		part = alu_reg_data( alu, num );
		n = alu_bit_set_bit( part, num.from + by );
		
		part = alu_reg_data( alu, tmp );
		v = alu_bit_set_bit( part, tmp.from );
		
		while ( n.bit < num.upto )
		{	
			*(n.ptr) |= SET1IF( *(v.ptr) & v.mask, n.mask );
			
			alu_bit_inc(&n);
			alu_bit_inc(&v);
		}
	}
	
	return 0;
}

int_t alu_reg__shr( alu_t *alu, alu_reg_t num, alu_reg_t tmp, size_t by )
{
	alu_bit_t n, v;
	void *part;
	bool neg;
	size_t diff, mant;
	
	if ( by )
	{
		diff = num.upto - num.from;
		
		/* Mantissa will use same bits but exponent will be lesser */
		if ( alu_reg_floating( num ) )
		{
			mant = alu_man_dig( diff );
			num.upto--;
			num.from += mant;
			
			alu_set_raw( alu, tmp.node, by, 0 );
			return alu_reg_sub( alu, num, tmp );
		}
		
		neg = alu_reg_below0( alu, num );
		
		if ( by >= diff )
		{
			alu_reg_set( alu, num, neg );
			return 0;
		}
		
		num.node %= alu_used( alu );
		tmp.node %= alu_used( alu );
		tmp.from = num.from;
		tmp.upto = num.upto;
		
		/* We have the register so might as well */
		alu_reg_mov( alu, tmp, num );
		alu_reg_clr( alu, num );
		
		part = alu_reg_data( alu, num );
		n = alu_bit_set_bit( part, num.upto );
		
		part = alu_reg_data( alu, tmp );
		v = alu_bit_set_bit( part, tmp.upto );
		
		while ( by )
		{
			--by;
			alu_bit_dec(&n);
			
			*(n.ptr) |= SET1IF( neg, n.mask );
		}
		
		while ( n.bit > num.from )
		{
			alu_bit_dec(&n);
			alu_bit_dec(&v);
			
			*(n.ptr) |= SET1IF( *(v.ptr) & v.mask, n.mask );
		}
	}
	
	return 0;
}

int_t alu_reg__shift
(
	alu_t *alu
	, alu_reg_t num
	, alu_reg_t val
	, func_alu_reg__shift_t _shift
)
{
	int ret, cmp;
	uint_t _tmp = 0;
	alu_reg_t tmp;
	uintmax_t by = 0, ndiff = num.upto - num.from;
	
	ret = alu_get_reg_node( alu, &_tmp, 0 );
	
	if ( ret == 0 )
	{
		num.node %= alu_used( alu );
		val.node %= alu_used( alu );
		alu_reg_init( alu, tmp, _tmp, 0 );
		
		alu_uint_set_raw( alu, tmp.node, ndiff );
		cmp = alu_reg_cmp( alu, val, tmp );
		
		if ( cmp < 0 )
		{
			alu_uint_get_raw( alu, val.node, &by );
			ret = _shift( alu, num, tmp, by );
		}
		else
		{
			ret = _shift( alu, num, tmp, -1 );
		}
		
		alu_rem_reg_node( alu, &_tmp );
		return ret;
	}
	
	alu_error( ret );
	return ret;
}


int_t alu_reg_multiply
(
	alu_t *alu
	, alu_reg_t num
	, alu_reg_t val
	, alu_reg_t cpy
	, alu_reg_t tmp
)
{
	int ret = alu_reg_mov( alu, cpy, num );
	bool carry = false, caught;
	alu_bit_t v;
	size_t p;
	void *V;
	
	if ( ret == 0 )
	{
		num.node %= alu_used( alu );
		val.node %= alu_used( alu );
		cpy.node %= alu_used( alu );
		tmp.node %= alu_used( alu );
		
		alu_reg_clr( alu, num );
		
		V = alu_reg_data( alu, val );
		v = alu_bit_set_bit( V, val.from );
		
		for ( p = v.bit; v.bit < val.upto; alu_bit_inc(&v) )
		{
			if ( *(v.ptr) & v.mask )
			{	
				(void)alu_reg__shl( alu, cpy, tmp, v.bit - p );
				ret = alu_reg_add( alu, num, cpy );
				p = v.bit;
				
				carry = (carry | (ret == EOVERFLOW));
				caught = ((ret == EOVERFLOW) | (ret == ENODATA) | (ret == 0));
				
				if ( !caught )
				{
					alu_error( ret );
					return ret;
				}
			}
		}
	
		return carry ? EOVERFLOW : 0;
	}
	
	alu_error(ret);
	return ret;
}

int_t alu_reg_mul
(
	alu_t *alu
	, alu_reg_t num
	, alu_reg_t val
)
{
	int ret = 0;
	uint_t nodes[2] = {0};
	alu_reg_t cpy, tmp;
	
	if ( ret == 0 )
	{
		alu_reg_init( alu, cpy, nodes[0], 0 );
		alu_reg_init( alu, tmp, nodes[2], 0 );
		
		ret = alu_reg_multiply( alu, num, val, cpy, tmp );
		
		alu_rem_reg_nodes( alu, nodes, 2 );
		
		return ret;
	}
	
	alu_error(ret);
	return ret;
}

int_t alu_reg_neg( alu_t *alu, alu_reg_t num )
{
	int ret = alu_reg_not( alu, num );
	
	if ( ret == 0 )
		return alu_reg_inc( alu, num );
		
	alu_error(ret);
	return ret;
}

int_t alu_reg_divide
(
	alu_t *alu
	, alu_reg_t num
	, alu_reg_t val
	, alu_reg_t rem
	, alu_reg_t tmp
)
{
	int ret;
	alu_reg_t seg;
	alu_bit_t n;
	size_t bits = 0;
	bool nNeg, vNeg;
	void *N;
	
	if ( alu )
	{	
		num.node %= alu_used( alu );
		val.node %= alu_used( alu );
		rem.node %= alu_used( alu );
		tmp.node %= alu_used( alu );
		
		nNeg = alu_reg_below0( alu, num );
		vNeg = alu_reg_below0( alu, val );
		
		if ( nNeg )
			alu_reg_neg( alu, num );
		
		if ( vNeg )
			alu_reg_neg( alu, val );
		
		(void)alu_reg_mov( alu, rem, num );
		(void)alu_reg_clr( alu, num );
		
		N = alu_reg_data( alu, num );
		
		seg = rem;
		
		n = alu_reg_end_bit( alu, rem );
		seg.upto = seg.from = n.bit + 1;
		n = alu_bit_set_bit( N, num.from );
		
		for ( ; alu_reg_cmp( alu, rem, val ) >= 0; ++bits )
		{
			seg.from--;
			if ( alu_reg_cmp( alu, seg, val ) >= 0 )
			{
				ret = alu_reg_sub( alu, seg, val );
				
				if ( ret == ENODATA )
					break;
				
				alu_reg__shl( alu, num, tmp, bits );
				*(n.ptr) |= n.mask;
				bits = 0;
			}
		}
		
		if ( seg.from > rem.from )
			alu_reg__shl( alu, num, tmp, bits );
		
		if ( ret != ENODATA && nNeg != vNeg )
			alu_reg_neg( alu, num );
		
		if ( nNeg )
			alu_reg_neg( alu, rem );
		
		if ( vNeg )
			alu_reg_neg( alu, val );
		
		return ret;
	}
	
	return alu_err_null_ptr("alu");
}

int_t alu_reg_div
(
	alu_t *alu
	, alu_reg_t NUM
	, alu_reg_t VAL
)
{
	int ret = 0;
	uint_t nodes[2] = {0};
	alu_reg_t REM, TMP;
	
	ret = alu_get_reg_nodes( alu, nodes, 2, 0 );
	
	if ( ret == 0 )
	{
		alu_reg_init( alu, REM, nodes[0], 0 );
		alu_reg_init( alu, TMP, nodes[1], 0 );
		ret = alu_reg_divide( alu, NUM, VAL, REM, TMP );
		alu_rem_reg_nodes( alu, nodes, 2 );
		return ret;
	}
	
	alu_error( ret );
	return ret;
}

int_t alu_reg_rem
(
	alu_t *alu
	, alu_reg_t NUM
	, alu_reg_t VAL
)
{
	int ret = 0;
	uint_t nodes[2] = {0};
	alu_reg_t REM, TMP;
	
	ret = alu_get_reg_nodes( alu, nodes, 2, 0 );
	
	if ( ret == 0 )
	{
		alu_reg_init( alu, REM, nodes[0], 0 );
		alu_reg_init( alu, TMP, nodes[1], 0 );
		ret = alu_reg_divide( alu, NUM, VAL, REM, TMP );
		(void)alu_reg_mov( alu, NUM, TMP );
		alu_rem_reg_nodes( alu, nodes, 2 );
		return ret;
	}
	
	alu_error( ret );
	return ret;
}

int_t alu_reg__rol( alu_t *alu, alu_reg_t num, alu_reg_t tmp, size_t by )
{
	alu_bit_t n = {0}, v;
	void *num_part, *tmp_part;
	
	if ( !by )
		return 0;
	
	by %= (num.upto - num.from);
		
	num.node %= alu_used( alu );
	tmp.node %= alu_used( alu );
	
	tmp.info = num.info;
	tmp.from = num.from;
	tmp.upto = num.upto;
	
	num_part = alu_reg_data( alu, num );
	tmp_part = alu_reg_data( alu, tmp );
	
	alu_reg_mov( alu, tmp, num );
	
	n = alu_bit_set_bit( num_part, num.upto );
	v = alu_bit_set_bit( tmp_part, tmp.upto - by );
	
	while ( v.bit > tmp.from )
	{
		alu_bit_dec(&v);
		alu_bit_dec(&n);
		
		*(n.ptr) &= ~(n.mask);
		*(n.ptr) |= (n.mask * !!(*(v.ptr) & v.mask));
	}
	
	v = alu_bit_set_bit( tmp_part, tmp.upto );
	while ( n.bit > num.from )
	{
		alu_bit_dec(&v);
		alu_bit_dec(&n);
		
		*(n.ptr) &= ~(n.mask);
		*(n.ptr) |= (n.mask * !!(*(v.ptr) & v.mask));
	}
	
	return 0;
}

int_t alu_reg__ror( alu_t *alu, alu_reg_t num, alu_reg_t tmp, size_t by )
{
	alu_bit_t n, v;
	void *num_part, *tmp_part;
	
	if ( by )
	{	
		by %= (num.upto - num.from);
		
		num.node %= alu_used( alu );
		tmp.node %= alu_used( alu );
		
		tmp.info = num.info;
		tmp.from = num.from;
		tmp.upto = num.upto;
		
		num_part = alu_reg_data( alu, num );
		tmp_part = alu_reg_data( alu, tmp );
		
		alu_reg_mov( alu, tmp, num );
		
		n = alu_bit_set_bit( num_part, num.from );
		v = alu_bit_set_bit( tmp_part, tmp.from + by );
		
		while ( v.bit < tmp.upto )
		{	
			*(n.ptr) &= ~(n.mask);
			*(n.ptr) |= SET1IF( *(v.ptr) & v.mask, n.mask );

			alu_bit_inc(&v);
			alu_bit_inc(&n);
		}
		
		v = alu_bit_set_bit( tmp_part, tmp.from );
		while ( n.bit < num.upto )
		{
			*(n.ptr) &= ~(n.mask);
			*(n.ptr) |= SET1IF( *(v.ptr) & v.mask, n.mask );
			
			alu_bit_inc(&v);
			alu_bit_inc(&n);
		}
	}
	
	return 0;
}

int_t alu_reg__rotate
(
	alu_t *alu
	, alu_reg_t num
	, alu_reg_t val
	, func_alu_reg__shift_t _shift
)
{
	int ret, cmp;
	uint_t _tmp = 0;
	alu_reg_t tmp;
	uintmax_t by = 0, ndiff = num.upto - num.from;
	
	ret = alu_get_reg_node( alu, &_tmp, 0 );
	
	if ( ret == 0 )
	{
		alu_reg_init( alu, tmp, _tmp, 0 );
	
		alu_uint_set_raw( alu, tmp.node, ndiff );
		cmp = alu_reg_cmp( alu, val, tmp );
		
		if ( cmp < 0 )
		{
			alu_uint_get_raw( alu, val.node, &by );
			ret = _shift( alu, num, tmp, by );
		}
		else
		{
			ret = _shift( alu, num, tmp, -1 );
		}
		
		alu_rem_reg_node( alu, &_tmp );
		return ret;
	}
	
	alu_error( ret );
	return ret;
}

size_t alu_lowest_upto( alu_reg_t num, alu_reg_t val )
{
	size_t ndiff, vdiff;
	
	ndiff = (num.upto - num.from);
	vdiff = (val.upto - val.from);
		
	return num.from + LOWEST(ndiff,vdiff);
}

int_t alu_reg_and( alu_t *alu, alu_reg_t num, alu_reg_t val )
{
	alu_bit_t n, v;
	size_t pos = 0;
	void *part;
	
	num.node %= alu_used( alu );
	val.node %= alu_used( alu );
	
	pos = alu_lowest_upto( num, val );
	
	part = alu_reg_data( alu, num );
	n = alu_bit_set_bit( part, num.from );
	
	part = alu_reg_data( alu, val );
	v = alu_bit_set_bit( part, val.from );
	
	for
	(
		; n.bit < pos
		; alu_bit_inc(&n), alu_bit_inc(&v)
	)
	{
		/* TODO: Do branchless version of this */
		if ( !(*(v.ptr) & v.mask) )
			*(n.ptr) &= ~(n.mask);
	}
	
	while ( n.bit < num.upto )
	{
		*(n.ptr) &= ~(n.mask);
		alu_bit_inc(&n);
	}
	
	return 0;
}

int_t alu_reg__or( alu_t *alu, alu_reg_t num, alu_reg_t val )
{
	alu_bit_t n, v;
	size_t pos = 0;
	void *part;
	
	num.node %= alu_used( alu );
	val.node %= alu_used( alu );
	
	pos = alu_lowest_upto( num, val );
	
	part = alu_reg_data( alu, num );
	n = alu_bit_set_bit( part, num.from );
	
	part = alu_reg_data( alu, val );
	v = alu_bit_set_bit( part, val.from );
	
	for
	(
		; n.bit < pos
		; alu_bit_inc(&n), alu_bit_inc(&v)
	)
	{
		*(n.ptr) |= (*(v.ptr) & v.mask) ? n.mask : UNIC_SIZE_C(0);
	}
	
	return 0;
}

int_t alu_reg_xor( alu_t *alu, alu_reg_t num, alu_reg_t val )
{
	alu_bit_t n, v;
	size_t pos = 0;
	void *part;
	
	num.node %= alu_used( alu );
	val.node %= alu_used( alu );
	
	pos = alu_lowest_upto( num, val );
	
	part = alu_reg_data( alu, num );
	n = alu_bit_set_bit( part, num.from );
	part = alu_reg_data( alu, val );
	v = alu_bit_set_bit( part, val.from );
	
	for
	(
		; n.bit < pos
		; alu_bit_inc(&n), alu_bit_inc(&v)
	)
	{
		*(n.ptr) ^= (*(v.ptr) & v.mask) ? n.mask : UNIC_SIZE_C(0);
	}
	
	return 0;
}
