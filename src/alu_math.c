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
		n = alu_bit( part, NUM.from )
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
	uint_t tmp;
	alu_reg_t TMP;
	
	if ( raw )
	{
		size = HIGHEST( size, 1 );
		
		tmp = alu_get_reg_node( alu, size );
		
		if ( tmp )
		{
			alu_reg_init( alu, TMP, tmp, 0 );
			TMP.upto = size * UNIC_CHAR_BIT;
			
			alu_reg_mov( alu, TMP, NUM );
		
			(void)memcpy( raw, alu_data( alu, tmp ), size );
			
			alu_rem_reg_node( alu, &tmp );
			
			return 0;
		}
		
		if ( alu )
		{
			alu_error( alu_errno(alu) );
			return alu_errno(alu);
		}
		
		return alu_err_null_ptr("alu");
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
	int_t ret;
	uint_t tmp = 0;
	alu_reg_t TMP;
	
	size = HIGHEST( size, 1 );
	
	tmp = alu_get_reg_node( alu, size );
	
	if ( tmp )
	{
		
		alu_reg_init( alu, TMP, tmp, info );
		TMP.upto = size * UNIC_CHAR_BIT;

		(void)memcpy( alu_data( alu, tmp ), raw, size );
		
		ret = alu_reg_mov( alu, NUM, TMP );
		
		alu_rem_reg_node( alu, &tmp );
		
		return ret;
	}
	
	if ( alu )
	{
		alu_error( alu_errno(alu) );
		return alu_errno(alu);
	}
	
	return alu_err_null_ptr("alu");
}

int_t alu_set_raw( alu_t *alu, uint_t num, uintmax_t raw, uint_t info )
{
	alu_reg_t NUM;
	alu_reg_init( alu, NUM, num, 0 );
	return alu_reg_set_raw( alu, NUM, &raw, sizeof(uintmax_t), info );
}

int alu_reg_int2int( alu_t *alu, alu_reg_t DST, alu_reg_t SRC )
{
	alu_bit_t d, s;
	size_t limit;
	bool neg;
	
	if ( alu )
	{		
		d = alu_bit( (void*)alu_reg_data( alu, DST ), DST.from );
		s = alu_bit( (void*)alu_reg_data( alu, SRC ), SRC.from );
		
		limit = DST.from + LOWEST( DST.upto - DST.from, SRC.upto - SRC.from );
		
		while ( d.bit < limit )
		{
			*(d.ptr) &= ~(d.mask);
			*(d.ptr) |= SET1IF( *(s.ptr) & s.mask, d.mask );
			
			alu_bit_inc(&d);
			alu_bit_inc(&s);
		}
		
		neg = alu_reg_below0( alu, SRC );
		
		while ( d.bit < DST.upto )
		{
			*(d.ptr) &= ~(d.mask);
			*(d.ptr) |= SET1IF( neg, d.mask );
			
			alu_bit_inc(&d);
		}
		
		alu_errno(alu) = SET1IF
		(
			d.bit < (DST.from + (SRC.upto - SRC.from))
			, EOVERFLOW
		);
		
		return 0;
	}
	
	return alu_err_null_ptr("alu");
}

int alu_reg_int2flt( alu_t *alu, alu_reg_t DST, alu_reg_t SRC )
{
	int ret;
	alu_reg_t DEXP, DMAN;
	alu_bit_t d, s;
	void *D;
	size_t dig, exp, bias;
	bool neg;
	
	if ( alu )
	{
		neg = alu_reg_below0( alu, SRC );
		dig = DST.upto - DST.from;
		
		alu_reg_init( alu, DEXP, DST.node, 0 );
		alu_reg_init( alu, DMAN, DST.node, 0 );
		
		dig = alu_man_dig(dig);
		
		DEXP.upto = DST.upto - 1;
		DMAN.upto = DEXP.from = DST.from + dig;
		DMAN.from = DST.from;
		
		bias = -1;
		bias <<= (DEXP.upto - DEXP.from);
		bias = ~bias;
		bias >>= 1;
		
		/* Set +/- */
		D = alu_reg_data( alu, DST );
		d = alu_bit( D, DEXP.upto );
		*(d.ptr) &= ~(d.mask);
		*(d.ptr) |= SET1IF( neg, d.mask );
		
		s = alu_reg_end_bit( alu, SRC );
		
		if ( s.bit >= bias )
		{
			/* Set Infinity */
			(void)alu_reg_set_max( alu, DEXP );
			/* Clear mantissa so not treated as NaN */
			(void)alu_reg_clr( alu, DMAN );
			return EOVERFLOW;
		}
		
		/* FIXME: I think there is a bug here */
		exp = bias + s.bit;
		ret = alu_reg_set_raw( alu, DEXP, &exp, sizeof(size_t), 0 );
		
		if ( ret != 0 )
		{
			alu_error( ret );
			return ret;
		}
		
		return alu_reg_int2int( alu, DMAN, SRC );
	}
	
	return alu_err_null_ptr("alu");
}

int alu_reg_flt2int( alu_t *alu, alu_reg_t DST, alu_reg_t SRC )
{
	int ret;
	uint_t tmp = 0;
	alu_reg_t DEXP, SEXP, SMAN, TMP;
	alu_bit_t d, s;
	size_t dlength, slength, dman_dig, sman_dig, exp, bias;
	bool Inf, neg;
	
	if ( alu )
	{
		dlength = DST.upto - DST.from;
		slength = SRC.upto - SRC.from;
		
		dman_dig = alu_man_dig(dlength);
		sman_dig = alu_man_dig(slength);
		
		alu_reg_init( alu, DEXP, DST.node, 0 );
		alu_reg_init( alu, SEXP, SRC.node, 0 );
		alu_reg_init( alu, SMAN, SRC.node, 0 );
		
		DEXP.upto = DST.upto - 1;
		DEXP.from = DST.from + dman_dig;
		SEXP.upto = SRC.upto - 1;
		SMAN.upto = SEXP.from = SRC.from + sman_dig;
		SMAN.from = SRC.from;
		
		bias = -1;
		bias <<= (SEXP.upto - SEXP.from);
		bias = ~bias;
		
		ret = alu_reg_get_raw( alu, SEXP, &exp, sizeof(size_t) );
		
		if ( ret != 0 )
		{
			alu_error( ret );
			return ret;
		}
		
		neg = alu_reg_below0( alu, SRC );
		Inf = (exp == bias);
			
		/* Set +/- */
		d = alu_bit( (void*)alu_reg_data( alu, DST ), DEXP.upto );
		*(d.ptr) &= ~(d.mask);
		*(d.ptr) |= (neg * d.mask);
		
		if ( Inf )
		{
			s = alu_reg_end_bit( alu, SMAN );
			
			/* NaN cannot be recorded by an integer, use 0 instead */
			if ( *(s.ptr) & s.mask )
				return alu_reg_clr( alu, DST );
			
			/* Infinity cannot be recorded by an integer,
			 * use min/max instead */
			(void)alu_reg_set_max( alu, DST );
			return neg ? alu_reg_not( alu, DST ) : 0;
		}
		
		/* Check if exponent is negative - meaning the number is between
		 * 0 & 1, if it is then just set the integer to 0 */
		bias >>= 1;
		
		if ( exp < bias )
			return alu_reg_clr( alu, DST );
		
		exp -= bias;
		
		/* Check if number is to big to fit in the integer */
		if ( (exp + sman_dig + 1) > dlength )
		{
			(void)alu_reg_set_max( alu, DST );
			return neg ? alu_reg_not( alu, DST ) : 0;
		}
		
		/* alu_reg_get_raw() will have retrieved and released a register,
		 * since that succeeded will end up with that register here as the
		 * register was not released from memory, only usage */
		
		/* Mantissas have an assumed bit,
		 * in this case it's value can only be 1 */
		dlength = 1;
		(void)alu_reg_set_raw( alu, DST, &dlength, 1, 0 );
		
		tmp = alu_get_reg_node( alu, 0 );
		alu_reg_init( alu, TMP, tmp, 0 );
		
		/* These cannot possibly fail at this point */
		(void)alu_reg__shl( alu, DST, TMP, sman_dig );
		(void)alu_reg__or( alu, DST, SMAN );
		(void)alu_reg__shl( alu, DST, TMP, exp );
		
		alu_rem_reg_node( alu, &tmp );
		
		return 0;
	}
	
	return alu_err_null_ptr("alu");
}

int alu_reg_flt2flt( alu_t *alu, alu_reg_t DST, alu_reg_t SRC )
{
	int ret;
	alu_reg_t DEXP, DMAN, SEXP, SMAN;
	alu_bit_t d;
	size_t dlength, slength, dman_dig, sman_dig, exp, bias;
	bool Inf, neg;
	
	if ( alu )
	{
		dlength = DST.upto - DST.from;
		slength = SRC.upto - SRC.from;
		
		dman_dig = alu_man_dig(dlength);
		sman_dig = alu_man_dig(slength);
		
		alu_reg_init( alu, DEXP, DST.node, 0 );
		alu_reg_init( alu, DMAN, DST.node, 0 );
		alu_reg_init( alu, SEXP, SRC.node, 0 );
		alu_reg_init( alu, SMAN, SRC.node, 0 );
		
		DEXP.upto = DST.upto - 1;
		DMAN.upto = DEXP.from = DST.from + dman_dig;
		DMAN.from = DST.from;
		SEXP.upto = SRC.upto - 1;
		SMAN.upto = SEXP.from = SRC.from + sman_dig;
		SMAN.from = SRC.from;
		
		bias = -1;
		bias <<= (SEXP.upto - SEXP.from);
		bias = ~bias;
		
		ret = alu_reg_get_raw( alu, SEXP, &exp, sizeof(size_t) );
		
		if ( ret != 0 )
		{
			alu_error( ret );
			return ret;
		}
		
		neg = alu_reg_below0( alu, SRC );
		Inf = (exp == bias);
			
		/* Set +/- */
		d = alu_bit( (void*)alu_reg_data( alu, DST ), DEXP.upto );
		*(d.ptr) &= ~(d.mask);
		*(d.ptr) |= (neg * d.mask);
		
		/* Simplify code and pass on duties to another instance, added
		 * bonus of setting Infinity if >= Exponent limit */
		alu_reg_int2int( alu, DEXP, SEXP );
		
		dlength = DMAN.upto - DMAN.from;
		slength = SMAN.upto - SMAN.from;
		
		/* Make sure we copy the upper bits of the src mantissa */
		SMAN.from = SMAN.upto - LOWEST( dlength, slength );
		
		/* Whether it is NaN or a fittable number copying the mantissa
		 * is fine as long as we make sure it was not Infinity to begin
		 * with */
		return Inf
			? alu_reg_clr( alu, DMAN )
			: alu_reg_int2int( alu, DMAN, SMAN );
	}
	
	return alu_err_null_ptr("alu");
}

int alu_reg_mov
(
	alu_t *alu,
	alu_reg_t DST,
	alu_reg_t SRC
)
{
	if ( alu_reg_floating( SRC ) )
	{
		if ( alu_reg_floating( DST ) )
			return alu_reg_flt2flt( alu, DST, SRC );
		return alu_reg_flt2int( alu, DST, SRC );
	}
	
	if ( alu_reg_floating( DST ) )
		return alu_reg_int2flt( alu, DST, SRC );
	
	return alu_reg_int2int( alu, DST, SRC );
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
	void *N;
	
	NUM.node %= alu_used( alu );
	
	N = alu_reg_data( alu, NUM );
	n = alu_bit( N, NUM.upto );
	b = n.bit;
	
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
	int ret = 0, a, b;
	alu_bit_t n = {0}, v = {0};
	size_t ndiff = 0, vdiff = 0;
	
	if ( alu )
	{
		NUM.node %= alu_used(alu);
		VAL.node %= alu_used(alu);
		
		ret = SET1IF( !NUM.node | !VAL.node, EINVAL );
			
		if ( ret )
		{
			alu_error( ret );
			
			if ( !NUM.node ) alu_puts( "NUM.node was 0" );
			
			if ( !VAL.node ) alu_puts( "VAL.node was 0" );
			
			return ret;
		}
		
		/* Check sign is same */
		
		a = alu_reg_below0( alu, NUM );
		b = alu_reg_below0( alu, VAL );
		
		ret = -(a - b);
		
		/* Get end bits */
		
		n = alu_reg_end_bit( alu, NUM );
		v = alu_reg_end_bit( alu, VAL );
		
		alu_bit_inc(&n);
		alu_bit_inc(&v);
		
		ndiff = SET1IF( ret == 0, n.bit - NUM.from );
		vdiff = SET1IF( ret == 0, v.bit - VAL.from );
		
		ret = SET2IF( ret == 0, (ndiff > vdiff) - (ndiff < vdiff), ret );
		
		/* Deal with different sized integers */
		
		vdiff = SET1IF( ret == 0, vdiff );

		while ( ndiff < vdiff )
		{
			vdiff--;
			alu_bit_dec(&v);
			
			b = !!( *(v.ptr) & v.mask );
			
			ret = a - b;
			
			vdiff = SET1IF( ret == 0, vdiff );
		}
		
		ndiff = SET1IF( ret == 0, ndiff );
		
		while ( ndiff > vdiff )
		{
			ndiff--;
			alu_bit_dec(&n);
			
			a = !!( *(n.ptr) & n.mask );
			
			ret = a - b;
			
			ndiff = SET1IF( ret == 0, ndiff );
		}
		
		/* Finally compare what matches bit alignment */
		while ( ndiff )
		{
			ndiff--;
			alu_bit_dec(&n);
			alu_bit_dec(&v);
			
			a = !!( *(n.ptr) & n.mask );
			b = !!( *(v.ptr) & v.mask );
			
			ret = a - b;
			ndiff = SET1IF( ret == 0, ndiff );
		}

		return ret;
	}
	
	return alu_err_null_ptr("alu");
}

int_t alu_reg_not( alu_t *alu, alu_reg_t NUM )
{
	int ret;
	alu_bit_t n, e;
	void *part;
	size_t i, stop, mask, mask_init, mask_last;
	
	if ( alu )
	{
		NUM.node %= alu_used( alu );
		
		if ( !NUM.node )
		{
			ret = EINVAL;
			alu_error( ret );
			return ret;
		}
		
		part = alu_reg_data( alu, NUM );
		
		n = alu_bit( part, NUM.from );
		e = alu_bit( part, NUM.upto - 1 );
		
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
	
	n = alu_bit( part, NUM.from );
	
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
	
	n = alu_bit( part, NUM.from );
	
	for ( ; n.bit < NUM.upto; alu_bit_inc(&n) )
	{
		is1 = !!(*(n.ptr) & n.mask);
		*(n.ptr) &= ~(n.mask);
		*(n.ptr) |= SET1IF( !is1, n.mask );
		NUM.upto = SET1IF( !is1, NUM.upto );
	}
	
	return SET2IF( NUM.upto, EOVERFLOW, 0 );
}

int_t alu_reg_add(
	alu_t *alu
	, alu_reg_t NUM
	, alu_reg_t VAL
)
{
	int ret;
	bool carry = false, changed = false;
	alu_bit_t n, v = {0};
	size_t pos = 0;
	void *part;
	uint_t nodes[3], ncpy, vcpy, tmp;
	alu_reg_t NCPY, VCPY, NEXP, NMAN, VEXP, VMAN, TMP;
	
	if ( alu )
	{
		NUM.node %= alu_used( alu );
		VAL.node %= alu_used( alu );
		
		ret = SET1IF( !NUM.node | !VAL.node, EINVAL );
			
		if ( ret )
		{
			alu_error( ret );
			
			if ( !NUM.node ) alu_puts( "NUM.node was 0!" );
			
			if ( !VAL.node ) alu_puts( "VAL.node was 0!" );
			
			return ret;
		}
		
		if ( alu_reg_floating( NUM ) )
		{
			ret = alu_get_reg_nodes( alu, nodes, 3, 0 );
			
			if ( ret == 0 )
			{
				ncpy = nodes[0];
				vcpy = nodes[1];
				tmp = nodes[2];
				
				alu_reg_init( alu, NCPY, ncpy, ALU_INFO_FLOAT | ALU_INFO__SIGN );
				alu_reg_init( alu, VCPY, ncpy, ALU_INFO_FLOAT | ALU_INFO__SIGN );
				alu_reg_init( alu, TMP, tmp, 0 );
				
				/* Need NUM to be unchanged so can restore details later,
				 * having both floats the same size also makes math easier */
				ret = alu_reg_mov( alu, NCPY, NUM );
				
				/* VAL is supposed to be unchanged so use VCPY instead */
				ret = alu_reg_mov( alu, VCPY, VAL );
				
				alu_reg_init_mantissa( NCPY, NMAN );
				alu_reg_init_exponent( NCPY, NEXP );
				
				alu_reg_init_mantissa( VCPY, VMAN );
				alu_reg_init_exponent( VCPY, VEXP );
				
				alu_rem_reg_nodes( alu, nodes, 3 );
				
				/* TODO: Implement addition:
				 * https://www.youtube.com/watch?v=Pox8LzIHhR4 */
				ret = ENOSYS;
				alu_error(ret);
				return ret;
			}
			
			alu_error(ret);
			return ret;
		}
		
		pos = NUM.from + LOWEST( NUM.upto - NUM.from, VAL.upto - VAL.from );
		
		part = alu_reg_data( alu, NUM );
		n = alu_bit( part, NUM.from );
		
		part = alu_reg_data( alu, VAL );
		v = alu_bit( part, VAL.from );
		
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
		
		/* Prevent false ENODATA's */
		for
		(
			; v.bit < VAL.upto
			; alu_bit_inc(&v)
		)
		{
			if ( *(v.ptr) & v.mask )
			{
				changed = true;
				break;
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
	
	return alu_err_null_ptr("alu");
}

int_t alu_reg_add_raw( alu_t *alu, alu_reg_t NUM, void *raw, size_t size, uint_t info )
{
	int ret;
	uint_t tmp = alu_get_reg_node( alu, size );
	alu_reg_t TMP;
	
	if ( tmp )
	{
		alu_reg_init( alu, TMP, tmp, info );
		ret = alu_reg_set_raw( alu, TMP, raw, size, info );
		
		if ( ret == 0 )
			return alu_reg_add( alu, NUM, TMP );
			
		alu_error(ret);
		return ret;
	}
	
	if ( alu )
	{
		ret = alu_errno(alu);
		alu_error(ret);
		return ret;
	}
	
	return alu_err_null_ptr("alu");
}

int_t alu_reg_sub( alu_t *alu, alu_reg_t NUM, alu_reg_t VAL )
{
	bool carry = false, changed = false;
	alu_bit_t n, v;
	size_t pos = 0;
	void *part;
	
	NUM.node %= alu_used( alu );
	VAL.node %= alu_used( alu );
	
	pos = NUM.from + LOWEST( NUM.upto - NUM.from, VAL.upto - VAL.from );
	
	part = alu_reg_data( alu, NUM );
	n = alu_bit( part, NUM.from );
	
	part = alu_reg_data( alu, VAL );
	v = alu_bit( part, VAL.from );
	
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
	
	/* Prevent false reports of ENODATA */
	for
	(
		; v.bit < pos
		; alu_bit_inc(&v)
	)
	{
		if ( *(v.ptr) & v.mask )
		{
			changed = true;
			break;
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
	int ret;
	alu_bit_t n = {0}, v;
	size_t diff;
	
	if ( by )
	{
		diff = NUM.upto - NUM.from;
		
		/* Mantissa will use same bits but exponent will be greater */
		if ( alu_reg_floating( NUM ) )
		{
			NUM.upto--;
			NUM.from += alu_man_dig( diff );
			NUM.info = 0;
			
			alu_set_raw( alu, TMP.node, by, 0 );
			return alu_reg_add( alu, NUM, TMP );
		}
		
		if ( by >= diff )
			return alu_reg_clr( alu, NUM );
		
		NUM.node %= alu_used( alu );
		TMP.node %= alu_used( alu );
		
		ret = SET1IF( !NUM.node | !TMP.node, EINVAL );
		
		if ( ret )
		{
			alu_error( ret );
			return ret;
		}
		
		/* We have the register so might as well */
		ret = alu_reg_mov( alu, TMP, NUM );
		
		n = alu_bit( (void*)alu_reg_data( alu, NUM ), NUM.from );
		v = alu_bit( (void*)alu_reg_data( alu, TMP ), TMP.from );
		
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
	int ret;
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
			NUM.from += alu_man_dig( diff );
			NUM.info = 0;
			
			alu_set_raw( alu, TMP.node, by, 0 );
			return alu_reg_sub( alu, NUM, TMP );
		}
		
		neg = alu_reg_below0( alu, NUM );
		
		if ( by >= diff )
			return alu_reg_set( alu, NUM, neg );
		
		NUM.node %= alu_used( alu );
		TMP.node %= alu_used( alu );
		
		ret = SET1IF( !NUM.node | !TMP.node, EINVAL );
		
		if ( ret )
		{
			alu_error( ret );
			return ret;
		}
		
		/* We have the register so might as well */
		ret = alu_reg_mov( alu, TMP, NUM );
		
		if ( ret )
		{
			alu_error( ret );
			return ret;
		}
		
		part = alu_reg_data( alu, NUM );
		n = alu_bit( part, NUM.upto );
		
		part = alu_reg_data( alu, TMP );
		v = alu_bit( part, TMP.upto );
		
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
		NUM.node %= alu_used( alu );
		VAL.node %= alu_used( alu );
		TMP.node %= alu_used( alu );
		
		ret = SET1IF( !NUM.node | !VAL.node | !TMP.node, EINVAL );
		
		if ( ret )
		{
			alu_error( ret );
			return ret;
		}
		
		by = NUM.upto - NUM.from;
		
		alu_reg_set_raw( alu, TMP, &by, sizeof(size_t), 0 );
		
		cmp = alu_reg_cmp( alu, VAL, TMP );
		
		if ( cmp < 0 )
		{
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
		
		ret = SET1IF( !NUM.node | !VAL.node | !CPY.node | !TMP.node, EINVAL );
		
		if ( ret )
		{
			alu_error( ret );
			return ret;
		}
		
		alu_reg_clr( alu, NUM );
		
		V = alu_reg_data( alu, VAL );
		v = alu_bit( V, VAL.from );
		
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
	
	alu_error( ret );
	return ret;
}

int_t alu_reg_mul
(
	alu_t *alu
	, alu_reg_t NUM
	, alu_reg_t VAL
)
{
	int ret;
	uint_t nodes[2];
	alu_reg_t CPY, TMP;
	
	ret = alu_get_reg_nodes( alu, nodes, 2, 0 );
	
	if ( ret == 0 )
	{
		alu_reg_init( alu, CPY, nodes[0], 0 );
		alu_reg_init( alu, TMP, nodes[1], 0 );
		
		ret = alu_reg_multiply( alu, NUM, VAL, CPY, TMP );
		
		alu_rem_reg_nodes( alu, nodes, 2 );
		
		return ret;
	}
	
	alu_error( ret );
	return ret;
}

int_t alu_reg_neg( alu_t *alu, alu_reg_t NUM )
{
	int ret = alu_reg_not( alu, NUM );
	
	if ( ret == 0 )
		return alu_reg_inc( alu, NUM );
		
	alu_error( ret );
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
		
		ret = SET1IF( !NUM.node | !VAL.node | !REM.node | !TMP.node, EINVAL );
		
		if ( ret )
		{
			alu_error( ret );
			
			if ( !NUM.node )
				alu_puts("NUM.node was 0");
				
			if ( !VAL.node )
				alu_puts("VAL.node was 0");
				
			if ( !REM.node )
				alu_puts("REM.node was 0");
				
			if ( !TMP.node )
				alu_puts("TMP.node was 0");
			
			return ret;
		}
		
		nNeg = alu_reg_below0( alu, NUM );
		vNeg = alu_reg_below0( alu, VAL );
		
		if ( nNeg )
			alu_reg_neg( alu, NUM );
		
		if ( vNeg )
			alu_reg_neg( alu, VAL );
		
		(void)alu_reg_int2int( alu, REM, NUM );
		(void)alu_reg_clr( alu, NUM );
		
		N = alu_reg_data( alu, NUM );
		
		SEG = REM;
		
		n = alu_reg_end_bit( alu, REM );
		SEG.upto = SEG.from = n.bit + 1;
		n = alu_bit( N, NUM.from );
		
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
		
		if ( SEG.from > REM.from )
			alu_reg__shl( alu, NUM, TMP, (SEG.from - REM.from) );
		
		if ( nNeg != vNeg )
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
		
		REM.upto = NUM.upto;
		REM.from = NUM.from;
		
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
		
		REM.upto = NUM.upto;
		REM.from = NUM.from;
		
		ret = alu_reg_divide( alu, NUM, VAL, REM, TMP );
		(void)alu_reg_int2int( alu, NUM, REM );
		alu_rem_reg_nodes( alu, nodes, 2 );
		return ret;
	}
	
	alu_error( ret );
	return ret;
}

int_t alu_reg__rol( alu_t *alu, alu_reg_t NUM, alu_reg_t TMP, size_t by )
{
	int ret;
	alu_bit_t n = {0}, v;
	void *num_part, *tmp_part;
	
	if ( alu )
	{
		if ( !by )
			return 0;
			
		NUM.node %= alu_used( alu );
		TMP.node %= alu_used( alu );
		
		ret = SET1IF( !NUM.node | !TMP.node, EINVAL );
		
		if ( ret )
		{
			alu_error( ret );
			return ret;
		}
		
		by %= (NUM.upto - NUM.from);
		
		num_part = alu_reg_data( alu, NUM );
		tmp_part = alu_reg_data( alu, TMP );
		
		alu_reg_mov( alu, TMP, NUM );
		
		n = alu_bit( num_part, NUM.upto );
		v = alu_bit( tmp_part, TMP.upto - by );
		
		while ( v.bit > TMP.from )
		{
			alu_bit_dec(&v);
			alu_bit_dec(&n);
			
			*(n.ptr) &= ~(n.mask);
			*(n.ptr) |= (n.mask * !!(*(v.ptr) & v.mask));
		}
		
		v = alu_bit( tmp_part, TMP.upto );
		while ( n.bit > NUM.from )
		{
			alu_bit_dec(&v);
			alu_bit_dec(&n);
			
			*(n.ptr) &= ~(n.mask);
			*(n.ptr) |= (n.mask * !!(*(v.ptr) & v.mask));
		}
		
		return 0;
	}
	
	return alu_err_null_ptr("alu");
}

int_t alu_reg__ror( alu_t *alu, alu_reg_t NUM, alu_reg_t TMP, size_t by )
{
	int ret;
	alu_bit_t n, v;
	void *num_part, *tmp_part;
	
	if ( by )
	{	
		
		
		NUM.node %= alu_used( alu );
		TMP.node %= alu_used( alu );
		
		ret = SET1IF( !NUM.node | !TMP.node, EINVAL );
		
		if ( ret )
		{
			alu_error( ret );
			return ret;
		}
		
		by %= (NUM.upto - NUM.from);
		
		num_part = alu_reg_data( alu, NUM );
		tmp_part = alu_reg_data( alu, TMP );
		
		alu_reg_mov( alu, TMP, NUM );
		
		n = alu_bit( num_part, NUM.from );
		v = alu_bit( tmp_part, TMP.from + by );
		
		while ( v.bit < TMP.upto )
		{	
			*(n.ptr) &= ~(n.mask);
			*(n.ptr) |= SET1IF( *(v.ptr) & v.mask, n.mask );

			alu_bit_inc(&v);
			alu_bit_inc(&n);
		}
		
		v = alu_bit( tmp_part, TMP.from );
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
	size_t by;
	
	if ( alu )
	{
		NUM.node %= alu_used( alu );
		VAL.node %= alu_used( alu );
		TMP.node %= alu_used( alu );
		
		ret = SET1IF( !NUM.node | !VAL.node | !TMP.node, EINVAL );
		
		if ( ret )
		{
			alu_error( ret );
			return ret;
		}
		
		by = NUM.upto - NUM.from;
		
		alu_reg_set_raw( alu, TMP, &by, sizeof(size_t), 0 );
		
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
	int ret;
	alu_bit_t n, v;
	size_t pos = 0;
	void *part;
	
	if ( alu )
	{
	
		NUM.node %= alu_used( alu );
		VAL.node %= alu_used( alu );
		
		ret = SET1IF( !NUM.node | !VAL.node, EINVAL );
			
		if ( ret )
		{
			alu_error( ret );
			return ret;
		}
		
		pos = alu_lowest_upto( NUM, VAL );
		
		part = alu_reg_data( alu, NUM );
		n = alu_bit( part, NUM.from );
		
		part = alu_reg_data( alu, VAL );
		v = alu_bit( part, VAL.from );
		
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
	
	return alu_err_null_ptr("alu");
}

int_t alu_reg__or( alu_t *alu, alu_reg_t NUM, alu_reg_t VAL )
{
	int ret;
	alu_bit_t n, v;
	size_t pos = 0;
	void *part;
	
	if ( alu )
	{
		NUM.node %= alu_used( alu );
		VAL.node %= alu_used( alu );
	
		ret = SET1IF( !NUM.node | !VAL.node, EINVAL );
		
		if ( ret )
		{
			alu_error( ret );
			return ret;
		}
	
		pos = alu_lowest_upto( NUM, VAL );
		
		part = alu_reg_data( alu, NUM );
		n = alu_bit( part, NUM.from );
		
		part = alu_reg_data( alu, VAL );
		v = alu_bit( part, VAL.from );
		
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
	
	return alu_err_null_ptr("alu");
}

int_t alu_reg_xor( alu_t *alu, alu_reg_t NUM, alu_reg_t VAL )
{
	int ret;
	alu_bit_t n, v;
	size_t pos = 0;
	void *part;
	
	if ( alu )
	{
		NUM.node %= alu_used( alu );
		VAL.node %= alu_used( alu );
		
		ret = SET1IF( !NUM.node | !VAL.node, EINVAL );
		
		if ( ret )
		{
			alu_error( ret );
			return ret;
		}
	
		pos = alu_lowest_upto( NUM, VAL );
		
		part = alu_reg_data( alu, NUM );
		n = alu_bit( part, NUM.from );
		part = alu_reg_data( alu, VAL );
		v = alu_bit( part, VAL.from );
		
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
	
	return alu_err_null_ptr("alu");
}
