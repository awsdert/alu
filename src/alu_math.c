#include "alu.h"
size_t alu_man_dig( size_t bits )
{
	return EITHER
	(
		bits < 6
		, 3
		, EITHER
		(
			bits < bitsof(float)
			, 5
			, EITHER
			(
				bits < bitsof(double)
				, FLT_MANT_DIG
				, EITHER
				(
					bits < bitsof(long double)
					, DBL_MANT_DIG
					, bits - 11
				)
			)
		)
	) - 1;
}
bool alu_reg_below0( alu_t *alu, alu_reg_t REG )
{
	if ( alu )
	{
		if ( REG.upto )
		{
			alu_bit_t r = alu_bit( (void*)alu_reg_data( alu, REG ), REG.upto - 1 );
			
			return IFBOTH( alu_reg_signed( REG ), *(r.ptr) & r.mask );
		}
		return 0;
	}
	
	(void)alu_err_null_ptr("alu");
	return 0;
}
int_t alu_reg_get_exponent( alu_t *alu, alu_reg_t SRC, size_t *dst )
{
	if ( dst )
	{
		alu_reg_t EXP;
		
		alu_reg_init_exponent( SRC, EXP );
		
		return alu_reg_get_raw( alu, EXP, dst, sizeof(size_t) );
	}
	return alu_err_null_ptr("dst");
}

int_t alu_reg_set_exponent( alu_t *alu, alu_reg_t DST, size_t src )
{
	alu_reg_t EXP;
	
	alu_reg_init_exponent( DST, EXP );
	
	return alu_reg_set_raw( alu, EXP, &src, sizeof(size_t), 0 );
}

size_t alu_reg_get_exponent_bias( alu_reg_t SRC )
{
	alu_reg_t EXP;
	size_t bias = UNIC_SIZE_C(~0);
	alu_reg_init_exponent( SRC, EXP );
	bias <<= (EXP.upto - EXP.from) - 1;
	return ~bias;
}

int_t alu__op1
(
	alu_t *alu
	, uint_t num
	, uint_t info
	, func_alu_reg_op1_t op1
)
{
	if ( alu )
	{
		alu_reg_t NUM;
		
		if ( info & ALU_INFO_FLOAT )
		{
			alu_reg_init_floating( alu, NUM, num );
		}
		else
		{
			alu_reg_init_unsigned( alu, NUM, num );
			NUM.info = info;
		}
		
		return op1( alu, NUM );
	}
	
	return alu_err_null_ptr("alu");
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
	if ( alu )
	{
		alu_reg_t NUM, VAL;
	
		if ( info & ALU_INFO_FLOAT )
		{
			alu_reg_init_floating( alu, NUM, num );
			alu_reg_init_floating( alu, VAL, val );
		}
		else
		{
			alu_reg_init_unsigned( alu, NUM, num );
			alu_reg_init_unsigned( alu, VAL, val );
			NUM.info = info;
			VAL.info = info;
		}
		
		return op2( alu, NUM, VAL );
	}
	
	return alu_err_null_ptr("alu");
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
	if ( alu )
	{
		alu_reg_t NUM, VAL;
		
		if ( info & ALU_INFO_FLOAT )
		{
			alu_reg_init_floating( alu, NUM, num );
			alu_reg_init_floating( alu, VAL, val );
		}
		else
		{
			alu_reg_init_unsigned( alu, NUM, num );
			alu_reg_init_unsigned( alu, VAL, val );
			NUM.info = info;
			VAL.info = info;
		}
		
		return op4( alu, NUM, VAL, reg, tmp );
	}
	
	return alu_err_null_ptr("alu");
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
		alu_reg_init_unsigned( alu, NUM, num );
		alu_reg_init_unsigned( alu, VAL, val );
		alu_reg_init_unsigned( alu, TMP, tmp );
		
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
		alu_reg_init_unsigned( alu, NUM, num );
		alu_reg_init_unsigned( alu, TMP, tmp );
		
		return _shift( alu, NUM, TMP, bits );
	}
	
	return alu_err_null_ptr("alu");
}

int_t alu_reg_set( alu_t *alu, alu_reg_t NUM, bool fillwith )
{
	if ( alu )
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
	
	return alu_err_null_ptr("alu");
}

int_t alu_set( alu_t *alu, uint_t num, bool fillwith )
{
	if ( alu )
	{
		alu_reg_t NUM;
		alu_reg_init_unsigned( alu, NUM, num );
		return alu_reg_set( alu, NUM, fillwith );
	}
	return alu_err_null_ptr("alu");
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
			alu_reg_init_unsigned( alu, TMP, tmp );
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
	if ( alu )
	{
		alu_reg_t NUM;
		alu_reg_init_unsigned( alu, NUM, num );
		return alu_reg_get_raw( alu, NUM, raw, sizeof(uintmax_t) );
	}
	return alu_err_null_ptr("alu");
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
		if ( info & ALU_INFO_FLOAT )
		{
			alu_reg_init_floating( alu, TMP, tmp );
		}
		else
		{
			alu_reg_init_unsigned( alu, TMP, tmp );
			TMP.info = info;
		}
		
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
	alu_reg_init_unsigned( alu, NUM, num );
	return alu_reg_set_raw( alu, NUM, &raw, sizeof(uintmax_t), info );
}

int alu_reg_int2int( alu_t *alu, alu_reg_t DST, alu_reg_t SRC )
{
	
	if ( alu )
	{
		alu_bit_t
			d = alu_bit( (void*)alu_reg_data( alu, DST ), DST.from )
			, s = alu_bit( (void*)alu_reg_data( alu, SRC ), SRC.from );
		size_t limit;
		bool neg;
		
		limit = DST.from + LOWEST( DST.upto - DST.from, SRC.upto - SRC.from );
		
		while ( d.bit < limit )
		{
			*(d.ptr) &= ~(d.mask);
			*(d.ptr) |= IFTRUE( *(s.ptr) & s.mask, d.mask );
			
			alu_bit_inc(&d);
			alu_bit_inc(&s);
		}
		
		neg = alu_reg_below0( alu, SRC );
		
		while ( d.bit < DST.upto )
		{
			*(d.ptr) &= ~(d.mask);
			*(d.ptr) |= IFTRUE( neg, d.mask );
			
			alu_bit_inc(&d);
		}
		
		alu_errno(alu) = IFTRUE
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
	if ( alu )
	{
		int ret;
		alu_reg_t EXP, MAN;
		alu_bit_t d, s;
		void *D;
		size_t exp, bias, b;
		bool neg = alu_reg_below0( alu, SRC );
		
		if ( neg )
		{
			alu_reg_neg( alu, SRC );
		}
		
		s = alu_reg_end_bit( alu, SRC );
		b = s.bit - SRC.from;
		
		/* +0 should look like an integer 0 */
		if ( !b && !(*(s.ptr) & s.mask) )
		{
			/* Put back the way we received it */
			if ( neg )
			{
				alu_reg_neg( alu, SRC );
			}
			
			return alu_reg_clr( alu, DST );
		}
		
		alu_reg_init_exponent( DST, EXP );
		alu_reg_init_mantissa( DST, MAN );
		
		/* Set +/- */
		D = alu_reg_data( alu, DST );
		d = alu_bit( D, EXP.upto );
		*(d.ptr) &= ~(d.mask);
		*(d.ptr) |= IFTRUE( neg, d.mask );
		
		bias = alu_reg_get_exponent_bias(DST);
		
		if ( b >= bias )
		{
			/* Put back the way we received it */
			if ( neg )
			{
				alu_reg_neg( alu, SRC );
			}
			
			/* Set Infinity */
			exp = (bias << 1) | 1;
			(void)alu_reg_set_exponent( alu, DST, exp );
			/* Clear mantissa so not treated as NaN */
			(void)alu_reg_clr( alu, MAN );
			return EOVERFLOW;
		}
		
		exp = (bias + b);
		ret = alu_reg_set_exponent( alu, DST, exp );
		
		if ( ret != 0 )
		{
			/* Put back the way we received it */
			if ( neg )
			{
				alu_reg_neg( alu, SRC );
			}
			
			alu_error( ret );
			return ret;
		}
		
		SRC.upto = s.bit;
		SRC.from = SRC.upto - LOWEST( b, MAN.upto - MAN.from );
		MAN.from = MAN.upto - LOWEST( b, MAN.upto - MAN.from );
		
		(void)alu_reg_int2int( alu, MAN, SRC );
		
		/* Round to nearest */
		if ( SRC.from > s.bit - b )
		{
			s = alu_bit
			(
				(void*)alu_reg_data( alu, SRC )
				, (SRC.upto - b) - 1
			);
			
			d = alu_bit
			(
				(void*)alu_reg_data( alu, MAN )
				, MAN.from
			);
			
			*(d.ptr) |= IFTRUE( *(s.ptr) & s.mask, d.mask );
		}
		
		/* Put back the way we received it */
		if ( neg )
		{
			alu_reg_neg( alu, SRC );
		}
		
		return 0;
	}
	
	return alu_err_null_ptr("alu");
}

int alu_reg_flt2int( alu_t *alu, alu_reg_t DST, alu_reg_t SRC )
{	
	if ( alu )
	{
		int ret;
		uint_t tmp;
		alu_reg_t DEXP, SEXP, SMAN, TMP;
		alu_bit_t d, s;
		size_t dlength, slength, sman_dig, exp
			, bias = alu_reg_get_exponent_bias(SRC);
		bool Inf, neg;
		
		dlength = DST.upto - DST.from;
		slength = SRC.upto - SRC.from;
		
		sman_dig = alu_man_dig(slength);
		
		alu_reg_init_exponent( DST, DEXP );
		alu_reg_init_exponent( SRC, SEXP );
		alu_reg_init_mantissa( SRC, SMAN );
		
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
		alu_reg_init_unsigned( alu, TMP, tmp );
		
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
	if ( alu )
	{
		int ret;
		alu_reg_t DEXP, DMAN, SEXP, SMAN;
		alu_bit_t d;
		void *D;
		size_t dlen, slen, exp, dbias, sbias, inf;
		bool neg;
		
		ret = alu_reg_get_exponent( alu, SRC, &exp );
		
		if ( ret != 0 )
		{
			alu_error( ret );
			return ret;
		}
		
		alu_reg_init_exponent( DST, DEXP );
		alu_reg_init_exponent( SRC, SEXP );
		alu_reg_init_mantissa( DST, DMAN );
		alu_reg_init_mantissa( SRC, SMAN );
		
		dlen = DMAN.upto - DMAN.from;
		slen = SMAN.upto - SMAN.from;
		D = (void*)alu_reg_data( alu, DST );
		
		/* Set +/- */
		neg = alu_reg_below0( alu, SRC );
		d = alu_bit( D, DST.upto -1 );
		*(d.ptr) &= ~(d.mask);
		*(d.ptr) |= IFTRUE( neg, d.mask );
		
		/* Make sure we refer to upper bits of both mantissa's if they're of
		 * different lengths */
		if ( dlen > slen )
		{
			DMAN.from = DMAN.upto - slen;
			
			/* Clear out useless bits, we'll worry about recuring digits once
			 * we have working arithmetic */
			d = alu_bit( D, DMAN.from );
			while ( d.bit > DST.from )
			{
				alu_bit_dec(&d);
				*(d.ptr) &= ~(d.mask);
			}
		}
		else if ( slen > dlen )
		{
			SMAN.from = SMAN.upto - dlen;
		}
		
		dbias = alu_reg_get_exponent_bias( DST );
		sbias = alu_reg_get_exponent_bias( SRC );
		
		inf = (sbias << 1) | 1;
		
		if ( exp >= inf )
		{
			inf = (dbias << 1) | 1;
			inf = IFTRUE( exp >= inf, inf );
			(void)alu_reg_set_exponent( alu, DST, inf );
			
			/* If SRC was +/-NaN then this will set that */
			return alu_reg_int2int( alu, DMAN, SMAN );
		}
		
		exp -= sbias;
		exp += dbias;
		inf = (dbias << 1) | 1;
		
		if ( exp >= inf )
		{
			/* Beyond what DST can handle, default to +/-Infinity */
			(void)alu_reg_set_exponent( alu, DST, inf );
			return alu_reg_clr( alu, DMAN );
		}
		
		(void)alu_reg_set_exponent( alu, DST, exp );
		return alu_reg_int2int( alu, DMAN, SMAN );
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
	if ( alu )
	{
		alu_reg_t NUM, VAL;
		
		alu_reg_init_unsigned( alu, NUM, num );
		alu_reg_init_unsigned( alu, VAL, val );
		
		return alu_reg_mov( alu, NUM, VAL );
	}
	
	return alu_err_null_ptr("alu");
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
		b = IFTRUE( !( *(n.ptr) & n.mask ), n.bit );
	}
	
	return n;
}

alu_bit_t alu_end_bit( alu_t *alu, uint_t num )
{
	alu_bit_t n = {0};
	
	if ( alu )
	{
		alu_reg_t NUM;
		alu_reg_init_unsigned( alu, NUM, num );
		return alu_reg_end_bit( alu, NUM );
	}
	
	(void)alu_err_null_ptr("alu");
	return n;
}

int_t alu_reg_cmp(
	alu_t *alu
	, alu_reg_t NUM
	, alu_reg_t VAL
)
{	
	if ( alu )
	{
		int ret, a, b;
		alu_bit_t n, v;
		size_t ndiff, vdiff;
		void *N, *V;
		
		NUM.node %= alu_used(alu);
		VAL.node %= alu_used(alu);
			
		if ( !NUM.node || !VAL.node )
		{
			alu_error( EDOM );
			
			if ( !NUM.node ) alu_puts( "NUM.node was 0" );
			
			if ( !VAL.node ) alu_puts( "VAL.node was 0" );
			
			a = !!(NUM.node);
			b = !!(VAL.node);
			
			return a - b;
		}
		
		/* Check sign is same */
		
		a = alu_reg_below0( alu, NUM );
		b = alu_reg_below0( alu, VAL );
		
		ret = -(a - b);
		
		if ( ret )
			return ret;
			
		if ( alu_reg_floating( NUM ) )
		{
			if ( alu_reg_floating( VAL ) )
			{
				alu_reg_t NEXP, VEXP, NMAN, VMAN;
				
				alu_reg_init_exponent( NUM, NEXP );
				alu_reg_init_exponent( VAL, VEXP );
				
				ret = alu_reg_cmp( alu, NEXP, VEXP );
				
				if ( ret )
					return ret;
				
				alu_reg_init_mantissa( NUM, NMAN );
				alu_reg_init_mantissa( VAL, VMAN );
				
				return alu_reg_cmp( alu, NMAN, VMAN );
			}
			
			alu->block.fault = EINVAL;
			alu_error( EINVAL );
			return 0;
		}
		
		/* Get end bits */
		
		N = alu_reg_data( alu, NUM );
		V = alu_reg_data( alu, VAL );
		
		n = alu_bit( N, NUM.upto );
		v = alu_bit( V, VAL.upto );
		
		ndiff = NUM.upto - NUM.from;
		vdiff = VAL.upto - VAL.from;
		
		/* Deal with different sized integers */

		while ( vdiff > ndiff )
		{
			alu_bit_dec(&v);
			b = !!( *(v.ptr) & v.mask );
			ret = a - b;
			if ( ret )
				return ret;
			--vdiff;
		}
		
		while ( ndiff > vdiff )
		{	
			alu_bit_dec(&n);
			a = !!( *(n.ptr) & n.mask );
			ret = a - b;
			if ( ret )
				return ret;
			--ndiff;
		}
		
		/* Finally compare what matches bit alignment */
		while ( ndiff )
		{
			alu_bit_dec(&n);
			alu_bit_dec(&v);
			a = !!( *(n.ptr) & n.mask );
			b = !!( *(v.ptr) & v.mask );
			ret = a - b;
			if ( ret )
				return ret;
			--ndiff;
		}

		return ret;
	}
	
	return alu_err_null_ptr("alu");
}

int_t alu_reg_not( alu_t *alu, alu_reg_t NUM )
{
	if ( alu )
	{
		alu_bit_t n, e;
		void *part;
		size_t i, stop, mask, mask_init, mask_last;
		
		NUM.node %= alu_used( alu );
		
		if ( !NUM.node )
		{
			int_t ret = EINVAL;
			alu_error( ret );
			return ret;
		}
		
		part = alu_reg_data( alu, NUM );
		
		n = alu_bit( part, NUM.from );
		e = alu_bit( part, NUM.upto - 1 );
		
		mask = 0;
		mask_init = mask_last = ~mask;
		
		mask_init <<= n.pos;
		mask_last <<= (bitsof(uintmax_t) - e.pos) - 1;
		
		stop = e.seg - n.seg;
		
		*(n.ptr) ^= EITHER( stop, mask_init, mask_init & mask_last );
		
		for ( i = 1; i < stop; ++i ) n.ptr[i] = ~(n.ptr[i]);
		
		*(e.ptr) ^= IFTRUE( stop, mask_last );
	
		return 0;
	}
	
	return alu_err_null_ptr( "alu" );
}

int_t alu_reg_inc( alu_t *alu, alu_reg_t NUM )
{
	alu_bit_t n;
	void *part;
	
	NUM.node %= alu_used( alu );
	part = alu_reg_data( alu, NUM );
	
	n = alu_bit( part, NUM.from );
	
	for ( ; n.bit < NUM.upto; alu_bit_inc(&n) )
	{
		bool is1 = !!(*(n.ptr) & n.mask);
		*(n.ptr) &= ~(n.mask);
		*(n.ptr) |= IFTRUE( !is1, n.mask );
		NUM.upto = IFTRUE( is1, NUM.upto );
	}
	
	return EITHER( NUM.upto, EOVERFLOW, 0 );
}

int_t alu_reg_dec( alu_t *alu, alu_reg_t NUM )
{
	alu_bit_t n;
	void *part;
	
	NUM.node %= alu_used( alu );
	part = alu_reg_data( alu, NUM );
	
	n = alu_bit( part, NUM.from );
	
	for ( ; n.bit < NUM.upto; alu_bit_inc(&n) )
	{
		bool is1 = !!(*(n.ptr) & n.mask);
		*(n.ptr) &= ~(n.mask);
		*(n.ptr) |= IFTRUE( !is1, n.mask );
		NUM.upto = IFTRUE( !is1, NUM.upto );
	}
	
	return EITHER( NUM.upto, EOVERFLOW, 0 );
}

int_t alu_match_exponents
(
	alu_t *alu
	, uint_t num
	, uint_t val
	, uint_t tmp
)
{
	if ( alu )
	{
		int_t ret;
		alu_reg_t NUM, VAL, MAN, TMP;
		size_t nexp;
			
		alu_reg_init_floating( alu, NUM, num );
		alu_reg_init_floating( alu, VAL, val );
		alu_reg_init_unsigned( alu, TMP, tmp );
		
		NUM.upto = alu_Nbits(alu);
		VAL.upto = alu_Nbits(alu);
		
		ret = alu_reg_get_exponent( alu, NUM, &nexp );
		
		if ( ret == 0 )
		{
			size_t vexp, diff;
			bool truncated = false;
			
			(void)alu_reg_get_exponent( alu, VAL, &vexp );
			
			if ( nexp > vexp )
			{
				if ( vexp )
				{
					diff = nexp - vexp;
					alu_reg_init_mantissa( VAL, MAN );
					
					/* Match exponent and align mantissa */
					(void)alu_reg_set_exponent( alu, VAL, nexp );
					alu->block.fault = 0;
					alu_reg__shr( alu, MAN, TMP, diff );
					truncated = (alu_errno(alu) == ERANGE);
					
					/* Insert assumed bit back into position */
					if ( diff < MAN.upto )
					{
						void *V = alu_reg_data( alu, VAL );
						alu_bit_t v = alu_bit( V, MAN.upto - diff );
						*(v.ptr) |= v.mask;
					}
				}
			}
			else if ( vexp > nexp )
			{
				if ( nexp )
				{
					diff = vexp - nexp;
					
					alu_reg_init_mantissa( NUM, MAN );
					
					(void)alu_reg_set_exponent( alu, NUM, vexp );
					alu->block.fault = 0;
					alu_reg__shr( alu, MAN, TMP, diff );
					truncated = (alu_errno(alu) == ERANGE);
					
					/* Insert assumed bit back into position */
					if ( diff < MAN.upto )
					{
						void *N = alu_reg_data( alu, NUM );
						alu_bit_t n = alu_bit( N, MAN.upto - diff );
						*(n.ptr) |= n.mask;
					}
				}
			}
			
			alu->block.fault = IFTRUE(truncated,ERANGE);
			
			return 0;
		}
		
		alu_error(ret);
		return ret;
	}
	
	return alu_err_null_ptr("alu");
}

int_t alu_reg_addition(
	alu_t *alu
	, alu_reg_t NUM
	, alu_reg_t VAL
	, uint_t cpy
	, uint_t tmp
)
{	
	if ( alu )
	{
		int ret;
		bool carry = false, changed = false;
		alu_bit_t n, v;
		size_t pos;
		void *part;
		alu_reg_t CPY, CEXP, CMAN, TEXP, TMAN, TMP;
		
		NUM.node %= alu_used( alu );
		VAL.node %= alu_used( alu );
		
		ret = IFTRUE( !NUM.node || !VAL.node, EINVAL );
			
		if ( ret )
		{
			alu_error( ret );
			
			if ( !NUM.node ) alu_puts( "NUM.node was 0!" );
			
			if ( !VAL.node ) alu_puts( "VAL.node was 0!" );
			
			return ret;
		}
		
		if ( alu_reg_floating( VAL ) )
		{
			alu_reg_init_floating( alu, TMP, tmp );
			
			TMP.upto = alu_Nbits(alu);
			
			/* VAL is supposed to be unchanged so use VCPY instead */
			ret = alu_reg_mov( alu, TMP, VAL );
			
			if ( ret == 0 )
			{	
				alu_reg_init_floating( alu, CPY, cpy );
				
				CPY.upto = alu_Nbits(alu);
				
				/* Need NUM to be unchanged so can restore details later,
				* having both floats the same size also makes math easier,
				* also no chance of failure here as the previous move
				* succeeded in getting the same temporary registers this
				* one will be looking for */
				(void)alu_reg_mov( alu, CPY, NUM );
				
				uint_t temp = alu_get_reg_node( alu, 0 );
		
				if ( temp )
				{					
					ret = alu_match_exponents( alu, cpy, tmp, temp );
					
					alu_rem_reg_node( alu, &temp );
					
					if ( ret == 0 )
					{
						bool truncated = (alu_errno(alu) == ERANGE);
						uint_t nodes[2];
						
						ret = alu_get_reg_nodes( alu, nodes, 2, 0 );
						
						if ( ret == 0 )
						{
							alu_reg_t _CMAN, _TMAN;
							size_t bias, exp;
							
							alu_reg_init_unsigned( alu, _CMAN, nodes[0] );
							alu_reg_init_unsigned( alu, _TMAN, nodes[1] );
							
							alu_reg_init_exponent( CPY, CEXP );
							alu_reg_init_exponent( TMP, TEXP );
							
							alu_reg_init_mantissa( CPY, CMAN );
							alu_reg_init_mantissa( TMP, TMAN );
							
							alu_reg_int2int( alu, _CMAN, CMAN );
							alu_reg_int2int( alu, _TMAN, TMAN );
							
							(void)alu_reg_get_exponent( alu, CPY, &exp );
							bias = alu_reg_get_exponent_bias( CPY );
							
							exp -= bias;
							if ( (ssize_t)exp < 0 )
							{
								exp = -exp;
							}
							
							n = alu_bit
							(
								(void*)alu_reg_data( alu, _CMAN )
								, exp
							);
							
							*(n.ptr) |= n.mask;
							
							v = alu_bit
							(
								(void*)alu_reg_data( alu, _TMAN )
								, exp
							);
							
							*(v.ptr) |= v.mask;
							
							/* Not possible to get an EOVERFLOW from these */
							ret = alu_reg_add( alu, _CMAN, _TMAN );
							
							switch ( ret )
							{
							case 0: case ENODATA: break;
							default:
								alu_rem_reg_nodes( alu, nodes, 2 );
								alu_error(ret);
								return ret;
							}
							
							n = alu_reg_end_bit( alu, _CMAN );
							exp = bias + n.bit;
						
							ret = alu_reg_set_exponent( alu, CPY, exp );
							
							/* No chance of failure at this point */
							(void)alu_reg_mov( alu, NUM, CPY );
							
							alu_rem_reg_nodes( alu, nodes, 2 );
							return EITHER( truncated, ERANGE, ret );
						}
					}
				}
				else
				{
					ret = alu_errno(alu);
				}
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

int_t alu_reg_add(
	alu_t *alu
	, alu_reg_t NUM
	, alu_reg_t VAL
)
{
	uint_t nodes[2];
	int_t ret = alu_get_reg_nodes( alu, nodes, 2, 0 );
	
	if ( ret == 0 )
	{
		ret = alu_reg_addition( alu, NUM, VAL, nodes[0], nodes[1] );
		alu_rem_reg_nodes( alu, nodes, 2 );
		
		switch (ret)
		{
		case 0: case ENODATA: case EOVERFLOW: break;
		default:
			alu_error(ret);
		}
	}
	else
	{
		alu_error(ret);
	}
	
	return ret;
}

int_t alu_reg_add_raw( alu_t *alu, alu_reg_t NUM, void *raw, size_t size, uint_t info )
{
	int ret;
	uint_t tmp = alu_get_reg_node( alu, size );
	alu_reg_t TMP;
	
	if ( tmp )
	{
		if ( info & ALU_INFO_FLOAT )
		{
			alu_reg_init_floating( alu, TMP, tmp );
		}
		else
		{
			alu_reg_init_unsigned( alu, TMP, tmp );
			TMP.info = info;
		}
		
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
	if ( by )
	{
		size_t diff = NUM.upto - NUM.from;
			
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
		
		if ( alu )
		{
			int ret;
			alu_bit_t n, v;
			
			NUM.node %= alu_used( alu );
			TMP.node %= alu_used( alu );
			
			ret = IFTRUE( !NUM.node || !TMP.node, EINVAL );
			
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
				
				alu->block.fault = EITHER( *(n.ptr) & n.mask, ERANGE, alu->block.fault );
				
				*(n.ptr) &= ~(n.mask);
				alu_bit_inc(&n);
			}
			
			while ( n.bit < NUM.upto )
			{
				*(n.ptr) &= ~(n.mask);
				*(n.ptr) |= IFTRUE( *(v.ptr) & v.mask, n.mask );
				
				alu_bit_inc(&n);
				alu_bit_inc(&v);
			}
			
			return 0;
		}
		
		return alu_err_null_ptr("alu");
	}
	
	return 0;
}

int_t alu_reg__shr( alu_t *alu, alu_reg_t NUM, alu_reg_t TMP, size_t by )
{	
	if ( by )
	{
		size_t diff = NUM.upto - NUM.from;
		
		/* Mantissa will use same bits but exponent will be lesser */
		if ( alu_reg_floating( NUM ) )
		{
			NUM.upto--;
			NUM.from += alu_man_dig( diff );
			NUM.info = 0;
			
			alu_set_raw( alu, TMP.node, by, 0 );
			return alu_reg_sub( alu, NUM, TMP );
		}
		
		if ( alu )
		{
			int ret;
			alu_bit_t n, v;
			void *part;
			bool neg = alu_reg_below0( alu, NUM );
		
			if ( by >= diff )
				return alu_reg_set( alu, NUM, neg );
		
			NUM.node %= alu_used( alu );
			TMP.node %= alu_used( alu );
			
			ret = IFTRUE( !NUM.node || !TMP.node, EINVAL );
			
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
				
				alu->block.fault = EITHER( *(n.ptr) & n.mask, ERANGE, alu->block.fault );
				
				*(n.ptr) &= ~(n.mask);
				*(n.ptr) |= IFTRUE( neg, n.mask );
			}
			
			while ( n.bit > NUM.from )
			{
				alu_bit_dec(&n);
				alu_bit_dec(&v);
				
				*(n.ptr) &= ~(n.mask);
				*(n.ptr) |= IFTRUE( *(v.ptr) & v.mask, n.mask );
			}
			
			return 0;
		}
		
		return alu_err_null_ptr("alu");
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
	if ( alu )
	{
		int ret, cmp;
		size_t by;
		
		NUM.node %= alu_used( alu );
		VAL.node %= alu_used( alu );
		TMP.node %= alu_used( alu );
		
		ret = IFTRUE( !NUM.node || !VAL.node || !TMP.node, EINVAL );
		
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
	, uint_t cpy
	, uint_t tmp
)
{
	if ( alu )
	{
		int ret;
		alu_reg_t CPY;
		
		if ( alu_reg_floating(VAL) )
		{
			/* Not yet supported */
			return ENOSYS;
		}
		
		if ( alu_reg_floating(NUM) )
		{
			/* Not yet supported */
			return ENOSYS;
		}
		
		cpy %= alu_used( alu );
		alu_reg_init_unsigned( alu, CPY, cpy );
		CPY.info = NUM.info;
		
		ret = alu_reg_mov( alu, CPY, NUM );
		
		if ( ret == 0 )
		{
			bool carry = false, caught;
			alu_reg_t TMP;
			alu_bit_t v;
			size_t p;
			void *V;
			
			NUM.node %= alu_used( alu );
			VAL.node %= alu_used( alu );
			
			tmp %= alu_used( alu );
			alu_reg_init_unsigned( alu, TMP, tmp );
			
			ret = IFTRUE( !NUM.node || !VAL.node || !CPY.node || !TMP.node, EINVAL );
			
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
	
	return alu_err_null_ptr("alu");
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
	
	ret = alu_get_reg_nodes( alu, nodes, 2, 0 );
	
	if ( ret == 0 )
	{
		ret = alu_reg_multiply( alu, NUM, VAL, nodes[0], nodes[1] );
		
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
	, uint_t rem
	, uint_t tmp
)
{	
	if ( alu )
	{	
		int ret;
		alu_reg_t SEG, REM, TMP;
		alu_bit_t n;
		size_t bits = 0;
		bool nNeg, vNeg;
		void *N;
		
		NUM.node %= alu_used( alu );
		VAL.node %= alu_used( alu );
		
		rem %= alu_used( alu );
		alu_reg_init_unsigned( alu, REM, rem );
		REM.info = NUM.info;
		
		tmp %= alu_used( alu );
		alu_reg_init_unsigned( alu, TMP, tmp );
		
		ret = IFTRUE( !NUM.node || !VAL.node || !REM.node || !TMP.node, EINVAL );
		
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
		
		for ( ; SEG.from > REM.from; ++bits )
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
	
	ret = alu_get_reg_nodes( alu, nodes, 2, 0 );
	
	if ( ret == 0 )
	{	
		ret = alu_reg_divide( alu, NUM, VAL, nodes[0], nodes[1] );
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
	int ret, tmpret;
	uint_t nodes[2];
	
	ret = alu_get_reg_nodes( alu, nodes, 2, 0 );
	
	if ( ret == 0 )
	{
		alu_reg_t REM;
		
		if ( NUM.info & ALU_INFO_FLOAT )
		{
			alu_reg_init_floating( alu, REM, nodes[0] );
		}
		else
		{
			alu_reg_init_unsigned( alu, REM, nodes[0] );
			REM.info = NUM.info;
		}
		
		ret = alu_reg_divide( alu, NUM, VAL, nodes[0], nodes[1] );
		
		switch ( ret )
		{
		case 0: case ENODATA: case EOVERFLOW:
			tmpret = ret;
			ret = alu_reg_mov( alu, NUM, REM );
			if ( ret == 0 )
				ret = tmpret;
			break;
		default:
			alu_error(ret);
		}
		alu_rem_reg_nodes( alu, nodes, 2 );
		return ret;
	}
	
	alu_error( ret );
	return ret;
}

int_t alu_reg__rol( alu_t *alu, alu_reg_t NUM, alu_reg_t TMP, size_t by )
{	
	if ( alu )
	{
		int ret;
		alu_bit_t n, v;
		void *num_part, *tmp_part;
		
		if ( !by )
			return 0;
			
		NUM.node %= alu_used( alu );
		TMP.node %= alu_used( alu );
		
		ret = IFTRUE( !NUM.node || !TMP.node, EINVAL );
		
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
	if ( by )
	{
		int ret;
		alu_bit_t n, v;
		void *num_part, *tmp_part;
				
		NUM.node %= alu_used( alu );
		TMP.node %= alu_used( alu );
		
		ret = IFTRUE( !NUM.node || !TMP.node, EINVAL );
		
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
			*(n.ptr) |= IFTRUE( *(v.ptr) & v.mask, n.mask );

			alu_bit_inc(&v);
			alu_bit_inc(&n);
		}
		
		v = alu_bit( tmp_part, TMP.from );
		while ( n.bit < NUM.upto )
		{
			*(n.ptr) &= ~(n.mask);
			*(n.ptr) |= IFTRUE( *(v.ptr) & v.mask, n.mask );
			
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
	if ( alu )
	{
		int ret, cmp;
		size_t by;
		
		NUM.node %= alu_used( alu );
		VAL.node %= alu_used( alu );
		TMP.node %= alu_used( alu );
		
		ret = IFTRUE( !NUM.node || !VAL.node || !TMP.node, EINVAL );
		
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
	if ( alu )
	{
		int ret;
		alu_bit_t n, v;
		size_t pos;
		void *part;
	
		NUM.node %= alu_used( alu );
		VAL.node %= alu_used( alu );
		
		ret = IFTRUE( !NUM.node || !VAL.node, EINVAL );
			
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
	if ( alu )
	{
		int ret;
		alu_bit_t n, v;
		size_t pos;
		void *part;
		
		NUM.node %= alu_used( alu );
		VAL.node %= alu_used( alu );
	
		ret = IFTRUE( !NUM.node || !VAL.node, EINVAL );
		
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
	if ( alu )
	{
		int ret;
		alu_bit_t n, v;
		size_t pos;
		void *part;
		
		NUM.node %= alu_used( alu );
		VAL.node %= alu_used( alu );
		
		ret = IFTRUE( !NUM.node || !VAL.node, EINVAL );
		
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
