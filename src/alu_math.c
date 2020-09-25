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
	, uint_t tmp
	, func_alu_reg__shift_t _shift
	, func_alu_reg_shift_t shift
)
{
	alu_reg_t NUM, VAL, TMP;
	
	if ( alu )
	{
		alu_reg_init( alu, NUM, num, 0 );
		alu_reg_init( alu, VAL, val, 0 );
		alu_reg_init( alu, TMP, tmp, 0 );
		
		return shift( alu, NUM, VAL, TMP, _shift );
	}
	
	return alu_err_null_ptr("alu");
}

int_t alu___shift
(
	alu_t *alu
	, uint_t num
	, uint_t tmp
	, size_t bits
	, func_alu_reg__shift_t _shift
)
{
	alu_reg_t NUM, TMP;
	
	if ( alu )
	{
		alu_reg_init( alu, NUM, num, 0 );
		alu_reg_init( alu, TMP, tmp, 0 );
		
		return _shift( alu, NUM, TMP, bits );
	}
	
	return alu_err_null_ptr("alu");
}

int_t alu_reg_set( alu_t *alu, alu_reg_t NUM, bool fillwith )
{
	void *part;
	alu_bit_t n;
	
	/* Branching takes longer to complete so assume intended node is
	 * remainder of division, prevents segfault */
	 
	NUM.node %= alu_used( alu );
	part = alu_reg_data( alu, NUM );
	
	/* Force to either 1 or 0 */
	fillwith = !!fillwith;
	
	for
	(
		n = alu_bit_set_bit( part, NUM.from )
		; n.bit < NUM.upto
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
	, alu_reg_t NUM
	, void *raw
	, size_t size
)
{
	int ret;
	uint_t tmp;
	alu_reg_t TMP;
	
	if ( raw )
	{
		size = HIGHEST( size, 1 );
		
		ret = alu_get_reg_node( alu, &tmp, size );
		
		if ( ret == 0 )
		{
			alu_reg_init( alu, TMP, tmp, 0 );
			TMP.upto = size * UNIC_CHAR_BIT;
			
			alu_print_reg( "TMP", alu, TMP, 1, 0, 0 );
			alu_reg_mov( alu, TMP, NUM );
		
			(void)memcpy( raw, alu_data( alu, tmp ), size );
			
			alu_rem_reg_node( alu, &tmp );
			return 0;
		}
		
		alu_error( ret );
		return ret;
	}
		
	return alu_err_null_ptr("raw");
}

int_t alu_get_raw( alu_t *alu, uint_t num, uintmax_t *raw )
{
	alu_reg_t NUM;
	alu_reg_init( alu, NUM, num, 0 );
	return alu_reg_get_raw( alu, NUM, raw, sizeof(uintmax_t) );
}

int_t alu_reg_set_raw
(
	alu_t *alu
	, alu_reg_t NUM
	, void *raw
	, size_t size
	, uint_t info
)
{
	int ret;
	uint_t tmp;
	alu_reg_t TMP;
	
	size = HIGHEST( size, 1 );
	
	ret = alu_get_reg_node( alu, &tmp, size );
	
	if ( ret == 0 )
	{	
		alu_reg_init( alu, TMP, tmp, info );
		TMP.upto = size * UNIC_CHAR_BIT;

		(void)memcpy( alu_data( alu, tmp ), raw, size );
		
		ret = alu_reg_mov( alu, NUM, TMP );
		
		alu_rem_reg_node( alu, &tmp );
		return ret;
	}
	
	alu_error( ret );
	return ret;
}

int_t alu_set_raw( alu_t *alu, uint_t num, uintmax_t raw, uint_t info )
{
	alu_reg_t NUM;
	alu_reg_init( alu, NUM, num, 0 );
	return alu_reg_set_raw( alu, NUM, &raw, sizeof(uintmax_t), info );
}

int alu_reg_mov
(
	alu_t *alu,
	alu_reg_t DST,
	alu_reg_t SRC
)
{
	int ret;
	void *D, *S;
	alu_bit_t d, s;
	size_t ndiff, vdiff, upto;
	bool neg, Inf = false;
	alu_reg_t DEXP, DMAN, SEXP, SMAN;
	
	ret = alu_reg_clr( alu, DST );
	
	if ( ret == 0 )
	{
		DST.node %= alu_used( alu );
		SRC.node %= alu_used( alu );
		
		D = alu_reg_data( alu, DST );
		S = alu_reg_data( alu, SRC );
		
		/* Check for +/- */
		neg = alu_reg_below0( alu, SRC );
		
		if ( alu_reg_floating( SRC ) )
		{
			alu_reg_init( alu, SEXP, SRC.node, 0 );
			alu_reg_init( alu, SMAN, SRC.node, 0 );
			
			SEXP.upto = SRC.upto - 1;
			SEXP.from = SRC.from + SRC.mant;
			SMAN.upto = SRC.from + SRC.mant;
			SMAN.from = SRC.from;
			
			/* Check if should set NaN or Infinity */
			DEXP.upto = SEXP.upto;
			DEXP.from = SEXP.from;
			alu_reg_set_max( alu, DEXP );
			Inf = (alu_reg_cmp( alu, SEXP, DEXP ) == 0);
			
			if ( alu_reg_floating( DST ) )
			{
				alu_reg_init( alu, DEXP, DST.node, 0 );
				
				DEXP.upto = DST.upto - 1;
				DEXP.from = DST.from + DST.mant;
				DMAN.upto = DST.from + DST.mant;
				DMAN.from = DST.from;
				
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
				(void)alu_reg_set_max( alu, DST );
				return neg ? alu_reg_not( alu, DST ) : 0;
			}
			
			/* Mantissa is always bigger than exponent so it is safe to use
			 * as the temporary copy of number to shift */
			alu_reg__shr( alu, DEXP, DMAN, 1 );
			
			/* Check if exponent is negative - meaning the number is between
			 * 0 & 1, if it is then just set the integer to 0 */
			ret = alu_reg_cmp( alu, SEXP, DEXP );
			
			if ( ret < 0 )
				return alu_reg_clr( alu, DST );
			
			DEXP.from = DST.from;
			DEXP.upto = DST.upto;
			ndiff = DST.upto - DST.from;
			alu_reg_set_raw( alu, DEXP, &(ndiff), sizeof(size_t), 0 );
			
			ret = alu_reg_cmp( alu, DEXP, SEXP );
			
			/* Check if number is to big to fit in integer */
			if ( ret >= 0 )
				return alu_reg_set_max( alu, DST );
			
			// FIXME: Finish implementing moving alu_fpn_t to alu_int_t/alu_uint_t
			// if ( alu_reg_cmp( alu, EXP, BIAS ) >= 0 )
			return ENOSYS;
		}
		
		ndiff = DST.upto - DST.from;
		vdiff = SRC.upto - SRC.from;
		
		d = alu_bit_set_bit( D, DST.from );
		s = alu_bit_set_bit( S, SRC.from );
		upto = DST.from + LOWEST( ndiff, vdiff );
		
		for ( ; d.bit < upto; alu_bit_inc(&d), alu_bit_inc(&s) )
		{
			*(d.ptr) &= ~(d.mask);
			*(d.ptr) |= SET1IF( *(s.ptr) & s.mask, d.mask );
		}
		
		for ( ; d.bit < DST.upto; alu_bit_inc(&d) )
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

alu_bit_t alu_reg_end_bit( alu_t *alu, alu_reg_t NUM )
{
	alu_bit_t n;
	size_t b;
	void *part;
	
	NUM.node %= alu_used( alu );
	
	part = alu_reg_data( alu, NUM );
	n = alu_bit_set_bit( part, NUM.upto - 1 );
	b = n.bit * !( *(n.ptr) & n.mask );
	
	while ( b > NUM.from )
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
	, alu_reg_t NUM
	, alu_reg_t VAL
)
{
	int a, b, c = 0;
	alu_bit_t n = {0}, v = {0};
	size_t ndiff = 0, vdiff = 0;
	bool nNeg, vNeg;
	
	n = alu_reg_end_bit( alu, NUM );
	v = alu_reg_end_bit( alu, VAL );
	
	nNeg = alu_reg_signed( NUM ) & (n.bit == NUM.upto - 1);
	vNeg = alu_reg_signed( VAL ) & (v.bit == VAL.upto - 1);
	
	if ( nNeg != vNeg )
	{
		c = -1 + vNeg + vNeg;
		return c;
	}
	
	ndiff = n.bit - NUM.from;
	vdiff = v.bit - VAL.from;
	
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

int_t alu_reg_not( alu_t *alu, alu_reg_t NUM )
{
	alu_bit_t n, e;
	void *part;
	size_t i, stop, mask, mask_init, mask_last;
	
	if ( alu )
	{
		NUM.node %= alu_used( alu );
		part = alu_reg_data( alu, NUM );
		
		n = alu_bit_set_bit( part, NUM.from );
		e = alu_bit_set_bit( part, NUM.upto - 1 );
		
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

int_t alu_reg_inc( alu_t *alu, alu_reg_t NUM )
{
	bool is1;
	alu_bit_t n;
	void *part;
	
	NUM.node %= alu_used( alu );
	part = alu_reg_data( alu, NUM );
	
	n = alu_bit_set_bit( part, NUM.from );
	
	for ( ; n.bit < NUM.upto; alu_bit_inc(&n) )
	{
		is1 = !!(*(n.ptr) & n.mask);
		*(n.ptr) &= ~(n.mask);
		*(n.ptr) |= SET1IF( !is1, n.mask );
		NUM.upto = SET1IF( is1, NUM.upto );
	}
	
	return SET2IF( NUM.upto, EOVERFLOW, 0 );
}

int_t alu_reg_dec( alu_t *alu, alu_reg_t NUM )
{
	bool is1;
	alu_bit_t n;
	void *part;
	size_t mask = 0;
	mask = ~mask;
	
	NUM.node %= alu_used( alu );
	part = alu_reg_data( alu, NUM );
	
	n = alu_bit_set_bit( part, NUM.from );
	
	for ( ; n.bit < NUM.upto; alu_bit_inc(&n) )
	{
		is1 = !!(*(n.ptr) & n.mask);
		*(n.ptr) &= ~(n.mask);
		*(n.ptr) |= SET1IF( !is1, n.mask );
		NUM.upto = SET1IF( !is1, NUM.upto );
	}
	
	return SET2IF( NUM.upto, EOVERFLOW, 0 );
}

int_t alu_reg_add( alu_t *alu, alu_reg_t NUM, alu_reg_t VAL )
{
	bool carry = false, changed = false;
	alu_bit_t n, v = {0};
	size_t pos = 0;
	void *part;
	
	NUM.node %= alu_used( alu );
	VAL.node %= alu_used( alu );
	
	pos = alu_lowest_upto( NUM, VAL );
	
	part = alu_reg_data( alu, NUM );
	n = alu_bit_set_bit( part, NUM.from );
	
	part = alu_reg_data( alu, VAL );
	v = alu_bit_set_bit( part, VAL.from );
	
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
			; n.bit < NUM.upto
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

int_t alu_reg_add_raw( alu_t *alu, alu_reg_t NUM, void *raw, size_t size )
{
	bool carry = false, changed = false;
	alu_bit_t n, v = {0};
	size_t pos = 0;
	void *part;
	
	NUM.node %= alu_used( alu );
	
	pos = LOWEST( size * CHAR_BIT, NUM.upto );
	
	part = alu_reg_data( alu, NUM );
	n = alu_bit_set_bit( part, NUM.from );
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
			; n.bit < NUM.upto
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

int_t alu_reg_sub( alu_t *alu, alu_reg_t NUM, alu_reg_t VAL )
{
	bool carry = false, changed = false;
	alu_bit_t n, v;
	size_t pos = 0;
	void *part;
	
	NUM.node %= alu_used( alu );
	VAL.node %= alu_used( alu );
	
	pos = alu_lowest_upto( NUM, VAL );
	
	part = alu_reg_data( alu, NUM );
	n = alu_bit_set_bit( part, NUM.from );
	
	part = alu_reg_data( alu, VAL );
	v = alu_bit_set_bit( part, VAL.from );
	
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
			; n.bit < NUM.upto
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

int_t alu_reg__shl( alu_t *alu, alu_reg_t NUM, alu_reg_t TMP, size_t by )
{
	alu_bit_t n = {0}, v;
	size_t diff;
	
	if ( by )
	{
		diff = NUM.upto - NUM.from;
		
		/* Mantissa will use same bits but exponent will be greater */
		if ( alu_reg_floating( NUM ) )
		{
			NUM.upto--;
			NUM.from += NUM.mant;
			NUM.mant = 0;
			NUM.info = 0;
			
			alu_set_raw( alu, TMP.node, by, 0 );
			return alu_reg_add( alu, NUM, TMP );
		}
		
		if ( by >= diff )
			return alu_reg_clr( alu, NUM );
		
		NUM.node %= alu_used( alu );
		TMP.node %= alu_used( alu );
		
		/* We have the register so might as well */
		alu_reg_mov( alu, TMP, NUM );
		
		n = alu_bit_set_bit( (void*)alu_reg_data( alu, NUM ), NUM.from );
		v = alu_bit_set_bit( (void*)alu_reg_data( alu, TMP ), TMP.from );
		
		while ( by )
		{
			--by;
			*(n.ptr) &= ~(n.mask);
			alu_bit_inc(&n);
		}
		
		while ( n.bit < NUM.upto )
		{
			*(n.ptr) &= ~(n.mask);
			*(n.ptr) |= SET1IF( *(v.ptr) & v.mask, n.mask );
			
			alu_bit_inc(&n);
			alu_bit_inc(&v);
		}
	}
	
	return 0;
}

int_t alu_reg__shr( alu_t *alu, alu_reg_t NUM, alu_reg_t TMP, size_t by )
{
	alu_bit_t n, v;
	void *part;
	bool neg;
	size_t diff;
	
	if ( by )
	{
		diff = NUM.upto - NUM.from;
		
		/* Mantissa will use same bits but exponent will be lesser */
		if ( alu_reg_floating( NUM ) )
		{
			NUM.upto--;
			NUM.from += NUM.mant;
			NUM.mant = 0;
			NUM.info = 0;
			
			alu_set_raw( alu, TMP.node, by, 0 );
			return alu_reg_sub( alu, NUM, TMP );
		}
		
		neg = alu_reg_below0( alu, NUM );
		
		if ( by >= diff )
			return alu_reg_set( alu, NUM, neg );
		
		NUM.node %= alu_used( alu );
		TMP.node %= alu_used( alu );
		TMP.from = NUM.from;
		TMP.upto = NUM.upto;
		
		/* We have the register so might as well */
		alu_reg_mov( alu, TMP, NUM );
		
		part = alu_reg_data( alu, NUM );
		n = alu_bit_set_bit( part, NUM.upto );
		
		part = alu_reg_data( alu, TMP );
		v = alu_bit_set_bit( part, TMP.upto );
		
		while ( by )
		{
			--by;
			alu_bit_dec(&n);
			
			*(n.ptr) &= ~(n.mask);
			*(n.ptr) |= SET1IF( neg, n.mask );
		}
		
		while ( n.bit > NUM.from )
		{
			alu_bit_dec(&n);
			alu_bit_dec(&v);
			
			*(n.ptr) &= ~(n.mask);
			*(n.ptr) |= SET1IF( *(v.ptr) & v.mask, n.mask );
		}
	}
	
	return 0;
}

int_t alu_reg__shift
(
	alu_t *alu
	, alu_reg_t NUM
	, alu_reg_t VAL
	, alu_reg_t TMP
	, func_alu_reg__shift_t _shift
)
{
	int ret, cmp;
	size_t by;
	
	if ( alu )
	{
		by = NUM.upto - NUM.from;
		
		NUM.node %= alu_used( alu );
		VAL.node %= alu_used( alu );
		TMP.node %= alu_used( alu );
		
		alu_uint_set_raw( alu, TMP.node, by );
		
		cmp = alu_reg_cmp( alu, VAL, TMP );
		
		if ( cmp < 0 )
		{
			alu_print_reg( "NUM", alu, NUM, 1, 0, 0 );
			alu_reg_get_raw( alu, VAL, &by, sizeof(size_t) );
			ret = _shift( alu, NUM, TMP, by );
		}
		else
		{
			ret = _shift( alu, NUM, TMP, -1 );
		}
		
		return ret;
	}
	
	return alu_err_null_ptr("alu");
}


int_t alu_reg_multiply
(
	alu_t *alu
	, alu_reg_t NUM
	, alu_reg_t VAL
	, alu_reg_t CPY
	, alu_reg_t TMP
)
{
	int ret = alu_reg_mov( alu, CPY, NUM );
	bool carry = false, caught;
	alu_bit_t v;
	size_t p;
	void *V;
	
	if ( ret == 0 )
	{
		NUM.node %= alu_used( alu );
		VAL.node %= alu_used( alu );
		CPY.node %= alu_used( alu );
		TMP.node %= alu_used( alu );
		
		alu_reg_clr( alu, NUM );
		
		V = alu_reg_data( alu, VAL );
		v = alu_bit_set_bit( V, VAL.from );
		
		for ( p = v.bit; v.bit < VAL.upto; alu_bit_inc(&v) )
		{
			if ( *(v.ptr) & v.mask )
			{	
				(void)alu_reg__shl( alu, CPY, TMP, v.bit - p );
				ret = alu_reg_add( alu, NUM, CPY );
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
	, alu_reg_t NUM
	, alu_reg_t VAL
)
{
	int ret = 0;
	uint_t nodes[2] = {0};
	alu_reg_t CPY, TMP;
	
	if ( ret == 0 )
	{
		alu_reg_init( alu, CPY, nodes[0], 0 );
		alu_reg_init( alu, TMP, nodes[2], 0 );
		
		ret = alu_reg_multiply( alu, NUM, VAL, CPY, TMP );
		
		alu_rem_reg_nodes( alu, nodes, 2 );
		
		return ret;
	}
	
	alu_error(ret);
	return ret;
}

int_t alu_reg_neg( alu_t *alu, alu_reg_t NUM )
{
	int ret = alu_reg_not( alu, NUM );
	
	if ( ret == 0 )
		return alu_reg_inc( alu, NUM );
		
	alu_error(ret);
	return ret;
}

int_t alu_reg_divide
(
	alu_t *alu
	, alu_reg_t NUM
	, alu_reg_t VAL
	, alu_reg_t REM
	, alu_reg_t TMP
)
{
	int ret;
	alu_reg_t SEG;
	alu_bit_t n;
	size_t bits = 0;
	bool nNeg, vNeg;
	void *N;
	
	if ( alu )
	{	
		NUM.node %= alu_used( alu );
		VAL.node %= alu_used( alu );
		REM.node %= alu_used( alu );
		TMP.node %= alu_used( alu );
		
		nNeg = alu_reg_below0( alu, NUM );
		vNeg = alu_reg_below0( alu, VAL );
		
		if ( nNeg )
			alu_reg_neg( alu, NUM );
		
		if ( vNeg )
			alu_reg_neg( alu, VAL );
		
		(void)alu_reg_mov( alu, REM, NUM );
		(void)alu_reg_clr( alu, NUM );
		
		N = alu_reg_data( alu, NUM );
		
		SEG = REM;
		
		n = alu_reg_end_bit( alu, REM );
		SEG.upto = SEG.from = n.bit + 1;
		n = alu_bit_set_bit( N, NUM.from );
		
		for ( ; alu_reg_cmp( alu, REM, VAL ) >= 0; ++bits )
		{
			SEG.from--;
			if ( alu_reg_cmp( alu, SEG, VAL ) >= 0 )
			{
				ret = alu_reg_sub( alu, SEG, VAL );
				
				if ( ret == ENODATA )
					break;
				
				alu_reg__shl( alu, NUM, TMP, bits );
				*(n.ptr) |= n.mask;
				bits = 0;
			}
		}
		
		if ( bits )
			alu_reg__shl( alu, NUM, TMP, bits );
		
		if ( ret != ENODATA && nNeg != vNeg )
			alu_reg_neg( alu, NUM );
		
		if ( nNeg )
			alu_reg_neg( alu, REM );
		
		if ( vNeg )
			alu_reg_neg( alu, VAL );
		
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
		(void)alu_reg_mov( alu, NUM, REM );
		alu_rem_reg_nodes( alu, nodes, 2 );
		return ret;
	}
	
	alu_error( ret );
	return ret;
}

int_t alu_reg__rol( alu_t *alu, alu_reg_t NUM, alu_reg_t TMP, size_t by )
{
	alu_bit_t n = {0}, v;
	void *num_part, *tmp_part;
	
	if ( !by )
		return 0;
	
	by %= (NUM.upto - NUM.from);
		
	NUM.node %= alu_used( alu );
	TMP.node %= alu_used( alu );
	
	TMP.info = NUM.info;
	TMP.from = NUM.from;
	TMP.upto = NUM.upto;
	
	num_part = alu_reg_data( alu, NUM );
	tmp_part = alu_reg_data( alu, TMP );
	
	alu_reg_mov( alu, TMP, NUM );
	
	n = alu_bit_set_bit( num_part, NUM.upto );
	v = alu_bit_set_bit( tmp_part, TMP.upto - by );
	
	while ( v.bit > TMP.from )
	{
		alu_bit_dec(&v);
		alu_bit_dec(&n);
		
		*(n.ptr) &= ~(n.mask);
		*(n.ptr) |= (n.mask * !!(*(v.ptr) & v.mask));
	}
	
	v = alu_bit_set_bit( tmp_part, TMP.upto );
	while ( n.bit > NUM.from )
	{
		alu_bit_dec(&v);
		alu_bit_dec(&n);
		
		*(n.ptr) &= ~(n.mask);
		*(n.ptr) |= (n.mask * !!(*(v.ptr) & v.mask));
	}
	
	return 0;
}

int_t alu_reg__ror( alu_t *alu, alu_reg_t NUM, alu_reg_t TMP, size_t by )
{
	alu_bit_t n, v;
	void *num_part, *tmp_part;
	
	if ( by )
	{	
		by %= (NUM.upto - NUM.from);
		
		NUM.node %= alu_used( alu );
		TMP.node %= alu_used( alu );
		
		TMP.info = NUM.info;
		TMP.from = NUM.from;
		TMP.upto = NUM.upto;
		
		num_part = alu_reg_data( alu, NUM );
		tmp_part = alu_reg_data( alu, TMP );
		
		alu_reg_mov( alu, TMP, NUM );
		
		n = alu_bit_set_bit( num_part, NUM.from );
		v = alu_bit_set_bit( tmp_part, TMP.from + by );
		
		while ( v.bit < TMP.upto )
		{	
			*(n.ptr) &= ~(n.mask);
			*(n.ptr) |= SET1IF( *(v.ptr) & v.mask, n.mask );

			alu_bit_inc(&v);
			alu_bit_inc(&n);
		}
		
		v = alu_bit_set_bit( tmp_part, TMP.from );
		while ( n.bit < NUM.upto )
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
	, alu_reg_t NUM
	, alu_reg_t VAL
	, alu_reg_t TMP
	, func_alu_reg__shift_t _shift
)
{
	int ret, cmp;
	uintmax_t by = 0, ndiff = NUM.upto - NUM.from;
	
	if ( alu )
	{
		alu_uint_set_raw( alu, TMP.node, ndiff );
		cmp = alu_reg_cmp( alu, VAL, TMP );
		
		if ( cmp < 0 )
		{
			alu_uint_get_raw( alu, VAL.node, &by );
			ret = _shift( alu, NUM, TMP, by );
		}
		else
		{
			ret = _shift( alu, NUM, TMP, -1 );
		}
		
		return ret;
	}
	
	return alu_err_null_ptr("alu");
}

size_t alu_lowest_upto( alu_reg_t NUM, alu_reg_t VAL )
{
	size_t ndiff, vdiff;
	
	ndiff = (NUM.upto - NUM.from);
	vdiff = (VAL.upto - VAL.from);
		
	return NUM.from + LOWEST(ndiff,vdiff);
}

int_t alu_reg_and( alu_t *alu, alu_reg_t NUM, alu_reg_t VAL )
{
	alu_bit_t n, v;
	size_t pos = 0;
	void *part;
	
	NUM.node %= alu_used( alu );
	VAL.node %= alu_used( alu );
	
	pos = alu_lowest_upto( NUM, VAL );
	
	part = alu_reg_data( alu, NUM );
	n = alu_bit_set_bit( part, NUM.from );
	
	part = alu_reg_data( alu, VAL );
	v = alu_bit_set_bit( part, VAL.from );
	
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
	
	while ( n.bit < NUM.upto )
	{
		*(n.ptr) &= ~(n.mask);
		alu_bit_inc(&n);
	}
	
	return 0;
}

int_t alu_reg__or( alu_t *alu, alu_reg_t NUM, alu_reg_t VAL )
{
	alu_bit_t n, v;
	size_t pos = 0;
	void *part;
	
	NUM.node %= alu_used( alu );
	VAL.node %= alu_used( alu );
	
	pos = alu_lowest_upto( NUM, VAL );
	
	part = alu_reg_data( alu, NUM );
	n = alu_bit_set_bit( part, NUM.from );
	
	part = alu_reg_data( alu, VAL );
	v = alu_bit_set_bit( part, VAL.from );
	
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

int_t alu_reg_xor( alu_t *alu, alu_reg_t NUM, alu_reg_t VAL )
{
	alu_bit_t n, v;
	size_t pos = 0;
	void *part;
	
	NUM.node %= alu_used( alu );
	VAL.node %= alu_used( alu );
	
	pos = alu_lowest_upto( NUM, VAL );
	
	part = alu_reg_data( alu, NUM );
	n = alu_bit_set_bit( part, NUM.from );
	part = alu_reg_data( alu, VAL );
	v = alu_bit_set_bit( part, VAL.from );
	
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
