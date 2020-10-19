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

bool_t alup_below0( alup_t _PTR )
{
	if ( _PTR.data && alup___signed(_PTR) )
	{
		alub_t sign = alub( _PTR.data, _PTR.upto - 1 );
		return !!(*(sign.ptr) & sign.mask);
	}
	
	return false;
}

bool_t alur_below0( alu_t *alu, alur_t REG )
{
	if ( alu )
	{
		if ( REG.upto )
		{
			alub_t r = alub( (void*)alur_data( alu, REG ), REG.upto - 1 );
			
			return IFBOTH( alur___signed( REG ), *(r.ptr) & r.mask );
		}
		return 0;
	}
	
	(void)alu_err_null_ptr("alu");
	return 0;
}

int_t alur_get_exponent( alu_t *alu, alur_t SRC, size_t *dst )
{
	if ( dst )
	{
		alur_t EXP;
		
		alur_init_exponent( SRC, EXP );
		
		return alur_get_raw( alu, EXP, dst, sizeof(size_t) );
	}
	return alu_err_null_ptr("dst");
}

int_t alur_set_exponent( alu_t *alu, alur_t DST, size_t src )
{
	alur_t EXP;
	
	alur_init_exponent( DST, EXP );
	
	return alur_set_raw( alu, EXP, &src, sizeof(size_t), 0 );
}

size_t alur_get_exponent_bias( alur_t SRC )
{
	alur_t EXP;
	size_t bias = UNIC_SIZE_C(~0);
	alur_init_exponent( SRC, EXP );
	bias <<= (EXP.upto - EXP.from) - 1;
	return ~bias;
}

int_t alu__op1
(
	alu_t *alu
	, uint_t num
	, uint_t info
	, func_alur_op1_t op1
)
{
	if ( alu )
	{
		alur_t NUM;
		
		if ( info & ALU_INFO_FLOAT )
		{
			alur_init_floating( alu, NUM, num );
		}
		else
		{
			alur_init_unsigned( alu, NUM, num );
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
	, func_alur_op2_t op2
)
{
	if ( alu )
	{
		alur_t NUM, VAL;
	
		if ( info & ALU_INFO_FLOAT )
		{
			alur_init_floating( alu, NUM, num );
			alur_init_floating( alu, VAL, val );
		}
		else
		{
			alur_init_unsigned( alu, NUM, num );
			alur_init_unsigned( alu, VAL, val );
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
	, func_alur_op4_t op4
)
{
	if ( alu )
	{
		alur_t NUM, VAL;
		
		if ( info & ALU_INFO_FLOAT )
		{
			alur_init_floating( alu, NUM, num );
			alur_init_floating( alu, VAL, val );
		}
		else
		{
			alur_init_unsigned( alu, NUM, num );
			alur_init_unsigned( alu, VAL, val );
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
	, func_alur__shift_t _shift
	, func_alur_shift_t shift
)
{
	alur_t NUM, VAL, TMP;
	
	if ( alu )
	{
		alur_init_unsigned( alu, NUM, num );
		alur_init_unsigned( alu, VAL, val );
		alur_init_unsigned( alu, TMP, tmp );
		
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
	, func_alur__shift_t _shift
)
{
	alur_t NUM, TMP;
	
	if ( alu )
	{
		alur_init_unsigned( alu, NUM, num );
		alur_init_unsigned( alu, TMP, tmp );
		
		return _shift( alu, NUM, TMP, bits );
	}
	
	return alu_err_null_ptr("alu");
}

int_t alur_set( alu_t *alu, alur_t NUM, bool fillwith )
{
	if ( alu )
	{
		void *part;
		alub_t n;
		
		/* Branching takes longer to complete so assume intended node is
		 * remainder of division, prevents segfault */
		 
		NUM.node %= alu_used( alu );
		part = alur_data( alu, NUM );
		
		/* Force to either 1 or 0 */
		fillwith = !!fillwith;
		
		for
		(
			n = alub( part, NUM.from )
			; n.bit < NUM.upto
			; alub_inc(&n)
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
		alur_t NUM;
		alur_init_unsigned( alu, NUM, num );
		return alur_set( alu, NUM, fillwith );
	}
	return alu_err_null_ptr("alu");
}

int_t alur_get_raw
(
	alu_t *alu
	, alur_t NUM
	, void *raw
	, size_t size
)
{
	uint_t tmp;
	alur_t TMP;
	
	if ( raw )
	{
		size = HIGHEST( size, 1 );
		
		tmp = alu_get_reg_node( alu, size );
		
		if ( tmp )
		{
			alur_init_unsigned( alu, TMP, tmp );
			TMP.upto = size * UNIC_CHAR_BIT;
			
			alur_mov( alu, TMP, NUM );
		
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
		alur_t NUM;
		alur_init_unsigned( alu, NUM, num );
		return alur_get_raw( alu, NUM, raw, sizeof(uintmax_t) );
	}
	return alu_err_null_ptr("alu");
}

int_t alur_set_raw
(
	alu_t *alu
	, alur_t NUM
	, void *raw
	, size_t size
	, uint_t info
)
{
	int_t ret;
	uint_t tmp = 0;
	alur_t TMP;
	
	size = HIGHEST( size, 1 );
	
	tmp = alu_get_reg_node( alu, size );
	
	if ( tmp )
	{
		if ( info & ALU_INFO_FLOAT )
		{
			alur_init_floating( alu, TMP, tmp );
		}
		else
		{
			alur_init_unsigned( alu, TMP, tmp );
			TMP.info = info;
		}
		
		TMP.upto = size * UNIC_CHAR_BIT;

		(void)memcpy( alu_data( alu, tmp ), raw, size );
		
		ret = alur_mov( alu, NUM, TMP );
		
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
	alur_t NUM;
	alur_init_unsigned( alu, NUM, num );
	return alur_set_raw( alu, NUM, &raw, sizeof(uintmax_t), info );
}

int_t alup_mov_int2int( alup_t _DST, alup_t _SRC )
{
	if ( _DST.data )
	{
		alub_t d = alub( _DST.data, _DST.from );
		
		if ( _SRC.data )
		{
			alub_t s = alub( _SRC.data, _SRC.from );
			size_t stop =
				_DST.from + LOWEST( _DST.upto - d.bit, _SRC.upto - s.bit );
			bool neg;
			
			while ( d.bit < stop )
			{
				*(d.ptr) &= ~(d.mask);
				*(d.ptr) |= IFTRUE( *(s.ptr) & s.mask, d.mask );
				
				alub_inc(&d);
				alub_inc(&s);
			}
			
			neg = alup_below0( _SRC );
			
			while ( d.bit < _DST.upto )
			{
				*(d.ptr) &= ~(d.mask);
				*(d.ptr) |= IFTRUE( neg, d.mask );
				
				alub_inc(&d);
			}
			
			return IFTRUE
			(
				d.bit < (_DST.from + (_SRC.upto - _SRC.from))
				, EOVERFLOW
			);
		}
		else
		{
			for ( ; d.bit < _DST.upto; alub_inc(&d) )
				*(d.ptr) &= ~(d.mask);
		}
		
		return 0;
	}
	
	return alu_err_null_ptr("_DST.data");
}

int_t alur_mov_int2int( alu_t *alu, alur_t DST, alur_t SRC )
{
	if ( alu )
	{
		int_t ret;
		alup_t _DST, _SRC;
		
		alup_init_register( alu, _DST, DST );
		alup_init_register( alu, _SRC, SRC );
		
		ret = alu->block.fault = alup_mov_int2int( _DST, _SRC );
		if ( !ret || ret == EOVERFLOW )
			return 0;
		return alu_errno(alu);
	}
	
	return alu_err_null_ptr("alu");
}

int alur_mov_int2flt( alu_t *alu, alur_t DST, alur_t SRC )
{	
	if ( alu )
	{
		int ret;
		alur_t EXP, MAN;
		alub_t d, s;
		void *D;
		size_t exp, bias, b;
		bool neg = alur_below0( alu, SRC );
		
		if ( neg )
		{
			alur_neg( alu, SRC );
		}
		
		s = alur_end_bit( alu, SRC );
		b = s.bit - SRC.from;
		
		/* +0 should look like an integer 0 */
		if ( !b && !(*(s.ptr) & s.mask) )
		{
			/* Put back the way we received it */
			if ( neg )
			{
				alur_neg( alu, SRC );
			}
			
			return alur_clr( alu, DST );
		}
		
		alur_init_exponent( DST, EXP );
		alur_init_mantissa( DST, MAN );
		
		/* Set +/- */
		D = alur_data( alu, DST );
		d = alub( D, EXP.upto );
		*(d.ptr) &= ~(d.mask);
		*(d.ptr) |= IFTRUE( neg, d.mask );
		
		bias = alur_get_exponent_bias(DST);
		
		if ( b >= bias )
		{
			/* Put back the way we received it */
			if ( neg )
			{
				alur_neg( alu, SRC );
			}
			
			/* Set Infinity */
			exp = (bias << 1) | 1;
			(void)alur_set_exponent( alu, DST, exp );
			/* Clear mantissa so not treated as NaN */
			(void)alur_clr( alu, MAN );
			return EOVERFLOW;
		}
		
		exp = (bias + b);
		ret = alur_set_exponent( alu, DST, exp );
		
		if ( ret != 0 )
		{
			/* Put back the way we received it */
			if ( neg )
			{
				alur_neg( alu, SRC );
			}
			
			alu_error( ret );
			return ret;
		}
		
		SRC.upto = s.bit;
		SRC.from = SRC.upto - LOWEST( b, MAN.upto - MAN.from );
		MAN.from = MAN.upto - LOWEST( b, MAN.upto - MAN.from );
		
		(void)alur_mov_int2int( alu, MAN, SRC );
		
		/* Round to nearest */
		if ( SRC.from > s.bit - b )
		{
			s = alub
			(
				(void*)alur_data( alu, SRC )
				, (SRC.upto - b) - 1
			);
			
			d = alub
			(
				(void*)alur_data( alu, MAN )
				, MAN.from
			);
			
			*(d.ptr) |= IFTRUE( *(s.ptr) & s.mask, d.mask );
		}
		
		/* Put back the way we received it */
		if ( neg )
		{
			alur_neg( alu, SRC );
		}
		
		return 0;
	}
	
	return alu_err_null_ptr("alu");
}

int alur_mov_flt2int( alu_t *alu, alur_t DST, alur_t SRC )
{	
	if ( alu )
	{
		int ret;
		uint_t tmp;
		alur_t DEXP, SEXP, SMAN, TMP;
		alub_t d, s;
		size_t dlength, slength, sman_dig, exp
			, bias = alur_get_exponent_bias(SRC);
		bool Inf, neg;
		
		dlength = DST.upto - DST.from;
		slength = SRC.upto - SRC.from;
		
		sman_dig = alu_man_dig(slength);
		
		alur_init_exponent( DST, DEXP );
		alur_init_exponent( SRC, SEXP );
		alur_init_mantissa( SRC, SMAN );
		
		ret = alur_get_raw( alu, SEXP, &exp, sizeof(size_t) );
		
		if ( ret != 0 )
		{
			alu_error( ret );
			return ret;
		}
		
		neg = alur_below0( alu, SRC );
		Inf = (exp == bias);
			
		/* Set +/- */
		d = alub( (void*)alur_data( alu, DST ), DEXP.upto );
		*(d.ptr) &= ~(d.mask);
		*(d.ptr) |= (neg * d.mask);
		
		if ( Inf )
		{
			s = alur_end_bit( alu, SMAN );
			
			/* NaN cannot be recorded by an integer, use 0 instead */
			if ( *(s.ptr) & s.mask )
				return alur_clr( alu, DST );
			
			/* Infinity cannot be recorded by an integer,
			 * use min/max instead */
			(void)alur_set_max( alu, DST );
			return neg ? alur_not( alu, DST ) : 0;
		}
		
		/* Check if exponent is negative - meaning the number is between
		 * 0 & 1, if it is then just set the integer to 0 */
		bias >>= 1;
		
		if ( exp < bias )
			return alur_clr( alu, DST );
		
		exp -= bias;
		
		/* Check if number is to big to fit in the integer */
		if ( (exp + sman_dig + 1) > dlength )
		{
			(void)alur_set_max( alu, DST );
			return neg ? alur_not( alu, DST ) : 0;
		}
		
		/* alur_get_raw() will have retrieved and released a register,
		 * since that succeeded will end up with that register here as the
		 * register was not released from memory, only usage */
		
		/* Mantissas have an assumed bit,
		 * in this case it's value can only be 1 */
		dlength = 1;
		(void)alur_set_raw( alu, DST, &dlength, 1, 0 );
		
		tmp = alu_get_reg_node( alu, 0 );
		alur_init_unsigned( alu, TMP, tmp );
		
		/* These cannot possibly fail at this point */
		(void)alur__shl( alu, DST, TMP, sman_dig );
		(void)alur__or( alu, DST, SMAN );
		(void)alur__shl( alu, DST, TMP, exp );
		
		alu_rem_reg_node( alu, &tmp );
		
		return 0;
	}
	
	return alu_err_null_ptr("alu");
}

int alur_mov_flt2flt( alu_t *alu, alur_t DST, alur_t SRC )
{
	if ( alu )
	{
		int ret;
		alur_t DEXP, DMAN, SEXP, SMAN;
		alub_t d;
		void *D;
		size_t dlen, slen, exp, dbias, sbias, inf;
		bool neg;
		
		ret = alur_get_exponent( alu, SRC, &exp );
		
		if ( ret != 0 )
		{
			alu_error( ret );
			return ret;
		}
		
		alur_init_exponent( DST, DEXP );
		alur_init_exponent( SRC, SEXP );
		alur_init_mantissa( DST, DMAN );
		alur_init_mantissa( SRC, SMAN );
		
		dlen = DMAN.upto - DMAN.from;
		slen = SMAN.upto - SMAN.from;
		D = (void*)alur_data( alu, DST );
		
		/* Set +/- */
		neg = alur_below0( alu, SRC );
		d = alub( D, DST.upto -1 );
		*(d.ptr) &= ~(d.mask);
		*(d.ptr) |= IFTRUE( neg, d.mask );
		
		/* Make sure we refer to upper bits of both mantissa's if they're of
		 * different lengths */
		if ( dlen > slen )
		{
			DMAN.from = DMAN.upto - slen;
			
			/* Clear out useless bits, we'll worry about recuring digits once
			 * we have working arithmetic */
			d = alub( D, DMAN.from );
			while ( d.bit > DST.from )
			{
				alub_dec(&d);
				*(d.ptr) &= ~(d.mask);
			}
		}
		else if ( slen > dlen )
		{
			SMAN.from = SMAN.upto - dlen;
		}
		
		dbias = alur_get_exponent_bias( DST );
		sbias = alur_get_exponent_bias( SRC );
		
		inf = (sbias << 1) | 1;
		
		if ( exp >= inf )
		{
			inf = (dbias << 1) | 1;
			inf = IFTRUE( exp >= inf, inf );
			(void)alur_set_exponent( alu, DST, inf );
			
			/* If SRC was +/-NaN then this will set that */
			return alur_mov_int2int( alu, DMAN, SMAN );
		}
		
		exp -= sbias;
		exp += dbias;
		inf = (dbias << 1) | 1;
		
		if ( exp >= inf )
		{
			/* Beyond what DST can handle, default to +/-Infinity */
			(void)alur_set_exponent( alu, DST, inf );
			return alur_clr( alu, DMAN );
		}
		
		(void)alur_set_exponent( alu, DST, exp );
		return alur_mov_int2int( alu, DMAN, SMAN );
	}
	
	return alu_err_null_ptr("alu");
}

int alur_mov
(
	alu_t *alu,
	alur_t DST,
	alur_t SRC
)
{
	if ( alur_floating( SRC ) )
	{
		if ( alur_floating( DST ) )
			return alur_mov_flt2flt( alu, DST, SRC );
		return alur_mov_flt2int( alu, DST, SRC );
	}
	
	if ( alur_floating( DST ) )
		return alur_mov_int2flt( alu, DST, SRC );
	
	return alur_mov_int2int( alu, DST, SRC );
}

int_t alu_mov( alu_t *alu, uint_t num, uint_t val )
{
	if ( alu )
	{
		alur_t NUM, VAL;
		
		alur_init_unsigned( alu, NUM, num );
		alur_init_unsigned( alu, VAL, val );
		
		return alur_mov( alu, NUM, VAL );
	}
	
	return alu_err_null_ptr("alu");
}

alub_t alur_end_bit( alu_t *alu, alur_t NUM )
{
	alub_t n;
	size_t b;
	void *N;
	
	NUM.node %= alu_used( alu );
	
	N = alur_data( alu, NUM );
	n = alub( N, NUM.upto );
	b = n.bit;
	
	while ( b > NUM.from )
	{
		alub_dec(&n);
		b = IFTRUE( !( *(n.ptr) & n.mask ), n.bit );
	}
	
	return n;
}

alub_t alu_end_bit( alu_t *alu, uint_t num )
{
	alub_t n = {0};
	
	if ( alu )
	{
		alur_t NUM;
		alur_init_unsigned( alu, NUM, num );
		return alur_end_bit( alu, NUM );
	}
	
	(void)alu_err_null_ptr("alu");
	return n;
}

int_t alup_cmp_int2int( alup_t _NUM, alup_t _VAL )
{
	int a = !(_NUM.data), b = !(_VAL.data), r = a - b;
	
	if ( r )
		return r;
	else
	{
		alub_t
			n = alub( _NUM.data, _NUM.upto )
			, v = alub( _VAL.data, _VAL.upto );
		size_t ndiff = n.bit - _NUM.from, vdiff = v.bit - _VAL.from;
		
		while ( ndiff > vdiff )
		{
			--ndiff;
			alub_dec(&n);
			
			r = !!(*(n.ptr) & n.mask);
			
			if ( r )
				return r;
		}
		
		while ( vdiff > ndiff )
		{
			--vdiff;
			alub_dec(&v);
			
			r = !!(*(v.ptr) & v.mask);
			
			if ( r )
				return -r;
		}
		
		while ( ndiff )
		{
			--ndiff;
			alub_dec(&n);
			alub_dec(&v);
			
			a = !!(*(n.ptr) & n.mask);
			b = !!(*(v.ptr) & v.mask);
			r = a - b;
			
			if ( r )
				return r;
		}
	}
	return 0;
}

int_t alur_cmp(
	alu_t *alu
	, alur_t NUM
	, alur_t VAL
)
{	
	if ( alu )
	{
		int ret, a, b;
		
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
		
		a = alur_below0( alu, NUM );
		b = alur_below0( alu, VAL );
		
		ret = -(a - b);
		
		if ( ret )
			return ret;
			
		if ( alur_floating( NUM ) )
		{
			if ( alur_floating( VAL ) )
			{
				alur_t NEXP, VEXP, NMAN, VMAN;
				
				alur_init_exponent( NUM, NEXP );
				alur_init_exponent( VAL, VEXP );
				
				ret = alur_cmp( alu, NEXP, VEXP );
				
				if ( ret )
					return ret;
				
				alur_init_mantissa( NUM, NMAN );
				alur_init_mantissa( VAL, VMAN );
				
				return alur_cmp( alu, NMAN, VMAN );
			}
			
			alu->block.fault = EINVAL;
			alu_error( EINVAL );
			return 0;
		}
		else
		{
			alup_t _NUM, _VAL;
			
			alup_init_register( alu, _NUM, NUM );
			alup_init_register( alu, _VAL, VAL );
			
			if ( a )
			{
				alur_neg( alu, NUM );
				alur_neg( alu, VAL );
			}
			
			ret = alup_cmp_int2int( _NUM, _VAL );
			
			if ( a )
			{
				alur_neg( alu, NUM );
				alur_neg( alu, VAL );
				/* Example: -3 vs -2 became 3 vs 2,
				 * 3 vs 2 returns 1 but -3 vs -2 should
				 * return -1 so we flip the result to
				 * match */
				return -ret;
			}
			
			return ret;
		}
	}
	
	return alu_err_null_ptr("alu");
}

int_t alur_not( alu_t *alu, alur_t NUM )
{
	if ( alu )
	{
		alub_t n, e;
		void *part;
		size_t i, stop, mask, mask_init, mask_last;
		
		NUM.node %= alu_used( alu );
		
		if ( !NUM.node )
		{
			int_t ret = EINVAL;
			alu_error( ret );
			return ret;
		}
		
		part = alur_data( alu, NUM );
		
		n = alub( part, NUM.from );
		e = alub( part, NUM.upto - 1 );
		
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

int_t alur_inc( alu_t *alu, alur_t NUM )
{
	alub_t n;
	void *part;
	
	NUM.node %= alu_used( alu );
	part = alur_data( alu, NUM );
	
	n = alub( part, NUM.from );
	
	for ( ; n.bit < NUM.upto; alub_inc(&n) )
	{
		bool is1 = !!(*(n.ptr) & n.mask);
		*(n.ptr) &= ~(n.mask);
		*(n.ptr) |= IFTRUE( !is1, n.mask );
		NUM.upto = IFTRUE( is1, NUM.upto );
	}
	
	return EITHER( NUM.upto, EOVERFLOW, 0 );
}

int_t alur_dec( alu_t *alu, alur_t NUM )
{
	alub_t n;
	void *part;
	
	NUM.node %= alu_used( alu );
	part = alur_data( alu, NUM );
	
	n = alub( part, NUM.from );
	
	for ( ; n.bit < NUM.upto; alub_inc(&n) )
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
		alur_t NUM, VAL, MAN, TMP;
		size_t nexp;
			
		alur_init_floating( alu, NUM, num );
		alur_init_floating( alu, VAL, val );
		alur_init_unsigned( alu, TMP, tmp );
		
		NUM.upto = alu_Nbits(alu);
		VAL.upto = alu_Nbits(alu);
		
		ret = alur_get_exponent( alu, NUM, &nexp );
		
		if ( ret == 0 )
		{
			size_t vexp, diff;
			bool truncated = false;
			
			(void)alur_get_exponent( alu, VAL, &vexp );
			
			if ( nexp > vexp )
			{
				if ( vexp )
				{
					diff = nexp - vexp;
					alur_init_mantissa( VAL, MAN );
					
					/* Match exponent and align mantissa */
					(void)alur_set_exponent( alu, VAL, nexp );
					alu->block.fault = 0;
					alur__shr( alu, MAN, TMP, diff );
					truncated = (alu_errno(alu) == ERANGE);
					
					/* Insert assumed bit back into position */
					if ( diff < MAN.upto )
					{
						void *V = alur_data( alu, VAL );
						alub_t v = alub( V, MAN.upto - diff );
						*(v.ptr) |= v.mask;
					}
				}
			}
			else if ( vexp > nexp )
			{
				if ( nexp )
				{
					diff = vexp - nexp;
					
					alur_init_mantissa( NUM, MAN );
					
					(void)alur_set_exponent( alu, NUM, vexp );
					alu->block.fault = 0;
					alur__shr( alu, MAN, TMP, diff );
					truncated = (alu_errno(alu) == ERANGE);
					
					/* Insert assumed bit back into position */
					if ( diff < MAN.upto )
					{
						void *N = alur_data( alu, NUM );
						alub_t n = alub( N, MAN.upto - diff );
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

int_t alup_add_int2int( alup_t _NUM, alup_t _VAL )
{
	bool_t changed = false, carry = false, active, append;
	alub_t
		n = alub( _NUM.data, _NUM.from )
		, v = alub( _VAL.data, _VAL.from );
	size_t stop =
		_NUM.from + LOWEST( _NUM.upto - _NUM.from, _VAL.upto - _VAL.from );
	
	for ( ; n.bit < stop; alub_inc(&n), alub_inc(&v) )
	{
		/* Add carry */
		active = !!(*(n.ptr) & n.mask);
		*(n.ptr) &= ~(n.mask);
		*(n.ptr) |= EITHER
		(
			carry
			, EITHER( active, 0, n.mask )
			, EITHER( active, n.mask, 0 )
		);
		carry = EITHER( carry, active, 0 );
		
		/* Add VAL bit */
		active = !!(*(n.ptr) & n.mask);
		append = !!(*(v.ptr) & v.mask);
		*(n.ptr) &= ~(n.mask);
		*(n.ptr) |= EITHER
		(
			append
			, EITHER( active, 0, n.mask )
			, EITHER( active, n.mask, 0 )
		);
		carry = EITHER( carry, true, (active && append) );
		
		/* Detect ENODATA */
		changed = EITHER( changed, true, append );
	}
	
	for ( ; !changed && v.bit < _VAL.upto; alub_inc(&v) )
	{
		changed = EITHER( changed, true, !!(*(v.ptr) & v.mask) );
	}
	
	for ( ; carry && n.bit < _NUM.upto; alub_inc(&n) )
	{
		carry = !!(*(n.ptr) & n.mask);
		*(n.ptr) &= ~(n.mask);
		*(n.ptr) |= EITHER( carry, 0, n.mask );
	}
	
	return EITHER
	(
		carry
		, EOVERFLOW
		, EITHER
		(
			changed
			, 0
			, ENODATA
		)
	);
}

int_t alur_addition(
	alu_t *alu
	, alur_t NUM
	, alur_t VAL
	, uint_t cpy
	, uint_t tmp
)
{	
	if ( alu )
	{
		int ret;
		alub_t n, v;
		void *part;
		alur_t CPY, CEXP, CMAN, TEXP, TMAN, TMP;
		
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
		
		if ( alur_floating( VAL ) )
		{
			alur_init_floating( alu, TMP, tmp );
			
			TMP.upto = alu_Nbits(alu);
			
			/* VAL is supposed to be unchanged so use VCPY instead */
			ret = alur_mov( alu, TMP, VAL );
			
			if ( ret == 0 )
			{	
				alur_init_floating( alu, CPY, cpy );
				
				CPY.upto = alu_Nbits(alu);
				
				/* Need NUM to be unchanged so can restore details later,
				* having both floats the same size also makes math easier,
				* also no chance of failure here as the previous move
				* succeeded in getting the same temporary registers this
				* one will be looking for */
				(void)alur_mov( alu, CPY, NUM );
				
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
							do
							{
								alur_t _CMAN, _TMAN;
								size_t bias, _exp;
								ssize_t exp;
								
								alur_init_unsigned( alu, _CMAN, nodes[0] );
								alur_init_unsigned( alu, _TMAN, nodes[1] );
								
								alur_init_exponent( CPY, CEXP );
								alur_init_exponent( TMP, TEXP );
								
								alur_init_mantissa( CPY, CMAN );
								alur_init_mantissa( TMP, TMAN );
								
								_CMAN.upto = CMAN.upto + 2;
								_TMAN.upto = TMAN.upto + 2;
								
								alur_mov_int2int( alu, _CMAN, CMAN );
								alur_mov_int2int( alu, _TMAN, TMAN );
								
								(void)alur_get_exponent( alu, CPY, &_exp );
								bias = alur_get_exponent_bias( CPY );
								
								if ( _exp && _exp != bias )
								{
									_exp -= bias;
									
									n = alub
									(
										(void*)alur_data( alu, _CMAN )
										, CMAN.upto
									);
									
									*(n.ptr) |= n.mask;
									
									v = alub
									(
										(void*)alur_data( alu, _TMAN )
										, TMAN.upto
									);
									
									*(v.ptr) |= v.mask;
								}
									
								exp = _exp;
								
								ret = alur_add( alu, _CMAN, _TMAN );
								
								if ( ret == ENODATA )
								{
									++exp;
									ret = 0;
								}
								
								if ( ret != 0 && ret != ENODATA )
									break;
								
								n = alur_end_bit( alu, _CMAN );
								part = alur_data( alu, _CMAN );
								for
								(
									v = alub( part, 0 )
									; v.bit < n.bit && !(*(v.ptr) & v.mask)
									; alub_inc(&v)
								);
								
								ret = alur_set_exponent( alu, CPY, bias + exp );
								
								if ( ret )
									break;
								
								(void)alur_mov_int2int( alu, CMAN, _CMAN );
								
								ret = alur_mov( alu, NUM, CPY );
								if ( ret )
									break;
								
								alu_rem_reg_nodes( alu, nodes, 2 );
								return EITHER( truncated, ERANGE, ret );
							}
							while (0);
							
							alu_rem_reg_nodes( alu, nodes, 2 );
							alu_error(ret);
							return ret;
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
		else
		{
			alup_t _NUM, _VAL;
			
			alup_init_register( alu, _NUM, NUM );
			alup_init_register( alu, _VAL, VAL );
			
			return alup_add_int2int( _NUM, _VAL );
		}
	}
	
	return alu_err_null_ptr("alu");
}

int_t alur_add(
	alu_t *alu
	, alur_t NUM
	, alur_t VAL
)
{
	uint_t nodes[2];
	int_t ret = alu_get_reg_nodes( alu, nodes, 2, 0 );
	
	if ( ret == 0 )
	{
		ret = alur_addition( alu, NUM, VAL, nodes[0], nodes[1] );
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

int_t alur_add_raw( alu_t *alu, alur_t NUM, void *raw, size_t size, uint_t info )
{
	int ret;
	uint_t tmp = alu_get_reg_node( alu, size );
	alur_t TMP;
	
	if ( tmp )
	{
		if ( info & ALU_INFO_FLOAT )
		{
			alur_init_floating( alu, TMP, tmp );
		}
		else
		{
			alur_init_unsigned( alu, TMP, tmp );
			TMP.info = info;
		}
		
		ret = alur_set_raw( alu, TMP, raw, size, info );
		
		if ( ret == 0 )
			return alur_add( alu, NUM, TMP );
			
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

int_t alup_sub_int2int( alup_t _NUM, alup_t _VAL )
{
	bool_t changed = false, carry = false, active, append;
	alub_t
		n = alub( _NUM.data, _NUM.from )
		, v = alub( _VAL.data, _VAL.from );
	size_t stop =
		_NUM.from + LOWEST( _NUM.upto - _NUM.from, _VAL.upto - _VAL.from );
	
	for ( ; n.bit < stop; alub_inc(&n), alub_inc(&v) )
	{
		/* Add carry */
		active = !!(*(n.ptr) & n.mask);
		*(n.ptr) &= ~(n.mask);
		*(n.ptr) |= EITHER
		(
			carry
			, EITHER( active, 0, n.mask )
			, EITHER( active, n.mask, 0 )
		);
		carry = EITHER( carry, !active, 0 );
		
		/* Add VAL bit */
		active = !!(*(n.ptr) & n.mask);
		append = !!(*(v.ptr) & v.mask);
		*(n.ptr) &= ~(n.mask);
		*(n.ptr) |= EITHER
		(
			append
			, EITHER( active, 0, n.mask )
			, EITHER( active, n.mask, 0 )
		);
		carry = EITHER( carry, true, (!active && append) );
		
		/* Detect ENODATA */
		changed = EITHER( changed, true, append );
	}
	
	for ( ; !changed && v.bit < _VAL.upto; alub_inc(&v) )
	{
		changed = EITHER( changed, true, !!(*(v.ptr) & v.mask) );
	}
	
	for ( ; carry && n.bit < _NUM.upto; alub_inc(&n) )
	{
		active = !!(*(n.ptr) & n.mask);
		*(n.ptr) &= ~(n.mask);
		*(n.ptr) |= EITHER( active, 0, n.mask );
		carry = !active;
	}
	
	return EITHER
	(
		carry
		, EOVERFLOW
		, EITHER
		(
			changed
			, 0
			, ENODATA
		)
	);
}

int_t alur_sub( alu_t *alu, alur_t NUM, alur_t VAL )
{	
	if ( alu )
	{
		alup_t _NUM, _VAL;
		
		alup_init_register( alu, _NUM, NUM );
		alup_init_register( alu, _VAL, VAL );
		
		return alup_sub_int2int( _NUM, _VAL );
	}
	
	return alu_err_null_ptr("alu");
}

int_t alup__shl( alup_t _NUM, void* _tmp, size_t by )
{	
	int_t ret = 0;
	
	if ( by )
	{
		size_t diff;
		alup_t _TMP = {0};
			
		/* Mantissa will use same bits but exponent will be greater */
		if ( alup_floating( _NUM ) )
		{
			alup_t _EXP, _VAL;
			
			alup_init_exponent( _NUM, _EXP );
			alup_init_unsigned( _VAL, &by, sizeof(size_t) );
			return alup_add_int2int( _EXP, _VAL );
		}
		
		diff = _NUM.upto - _NUM.from;
		
		if ( by >= diff )
			return alup_mov_int2int( _NUM, _TMP );
		else
		{
			alub_t n, v;
			size_t size = (diff / UNIC_CHAR_BIT) + !!(diff % UNIC_CHAR_BIT);
			
			alup_init_unsigned( _TMP, _tmp, size );
			
			/* We have the ptr so might as well */
			ret = alup_mov_int2int( _TMP, _NUM );
			
			n = alub( _NUM.data, _NUM.from );
			v = alub( _tmp, 0 );
			
			while ( by )
			{
				--by;
				
				ret = EITHER( *(n.ptr) & n.mask, ERANGE, ret );
				
				*(n.ptr) &= ~(n.mask);
				alub_inc(&n);
			}
			
			while ( n.bit < _NUM.upto )
			{
				*(n.ptr) &= ~(n.mask);
				*(n.ptr) |= IFTRUE( *(v.ptr) & v.mask, n.mask );
				
				alub_inc(&n);
				alub_inc(&v);
			}
		}
	}
	
	return ret;
}

int_t alur__shl( alu_t *alu, alur_t NUM, alur_t TMP, size_t by )
{	
	if ( by )
	{
		size_t diff = NUM.upto - NUM.from;
			
		/* Mantissa will use same bits but exponent will be greater */
		if ( alur_floating( NUM ) )
		{
			NUM.upto--;
			NUM.from += alu_man_dig( diff );
			NUM.info = 0;
			
			alu_set_raw( alu, TMP.node, by, 0 );
			return alur_add( alu, NUM, TMP );
		}
		
		if ( by >= diff )
			return alur_clr( alu, NUM );
		
		if ( alu )
		{
			int ret;
			alub_t n, v;
			
			NUM.node %= alu_used( alu );
			TMP.node %= alu_used( alu );
			
			ret = IFTRUE( !NUM.node || !TMP.node, EINVAL );
			
			if ( ret )
			{
				alu_error( ret );
				return ret;
			}
			
			/* We have the register so might as well */
			ret = alur_mov( alu, TMP, NUM );
			
			n = alub( (void*)alur_data( alu, NUM ), NUM.from );
			v = alub( (void*)alur_data( alu, TMP ), TMP.from );
			
			while ( by )
			{
				--by;
				
				alu->block.fault = EITHER( *(n.ptr) & n.mask, ERANGE, alu->block.fault );
				
				*(n.ptr) &= ~(n.mask);
				alub_inc(&n);
			}
			
			while ( n.bit < NUM.upto )
			{
				*(n.ptr) &= ~(n.mask);
				*(n.ptr) |= IFTRUE( *(v.ptr) & v.mask, n.mask );
				
				alub_inc(&n);
				alub_inc(&v);
			}
			
			return 0;
		}
		
		return alu_err_null_ptr("alu");
	}
	
	return 0;
}

int_t alur__shr( alu_t *alu, alur_t NUM, alur_t TMP, size_t by )
{	
	if ( by )
	{
		size_t diff = NUM.upto - NUM.from;
		
		/* Mantissa will use same bits but exponent will be lesser */
		if ( alur_floating( NUM ) )
		{
			NUM.upto--;
			NUM.from += alu_man_dig( diff );
			NUM.info = 0;
			
			alu_set_raw( alu, TMP.node, by, 0 );
			return alur_sub( alu, NUM, TMP );
		}
		
		if ( alu )
		{
			int ret;
			alub_t n, v;
			void *part;
			bool neg = alur_below0( alu, NUM );
		
			if ( by >= diff )
				return alur_set( alu, NUM, neg );
		
			NUM.node %= alu_used( alu );
			TMP.node %= alu_used( alu );
			
			ret = IFTRUE( !NUM.node || !TMP.node, EINVAL );
			
			if ( ret )
			{
				alu_error( ret );
				return ret;
			}
			
			/* We have the register so might as well */
			ret = alur_mov( alu, TMP, NUM );
			
			if ( ret )
			{
				alu_error( ret );
				return ret;
			}
			
			part = alur_data( alu, NUM );
			n = alub( part, NUM.upto );
			
			part = alur_data( alu, TMP );
			v = alub( part, TMP.upto );
			
			while ( by )
			{
				--by;
				alub_dec(&n);
				
				alu->block.fault = EITHER( *(n.ptr) & n.mask, ERANGE, alu->block.fault );
				
				*(n.ptr) &= ~(n.mask);
				*(n.ptr) |= IFTRUE( neg, n.mask );
			}
			
			while ( n.bit > NUM.from )
			{
				alub_dec(&n);
				alub_dec(&v);
				
				*(n.ptr) &= ~(n.mask);
				*(n.ptr) |= IFTRUE( *(v.ptr) & v.mask, n.mask );
			}
			
			return 0;
		}
		
		return alu_err_null_ptr("alu");
	}
	
	return 0;
}

int_t alur__shift
(
	alu_t *alu
	, alur_t NUM
	, alur_t VAL
	, alur_t TMP
	, func_alur__shift_t _shift
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
		
		alur_set_raw( alu, TMP, &by, sizeof(size_t), 0 );
		
		cmp = alur_cmp( alu, VAL, TMP );
		
		if ( cmp < 0 )
		{
			alur_get_raw( alu, VAL, &by, sizeof(size_t) );
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


int_t alur_multiply
(
	alu_t *alu
	, alur_t NUM
	, alur_t VAL
	, uint_t cpy
	, uint_t tmp
)
{
	if ( alu )
	{
		int ret;
		alur_t CPY;
		
		if ( alur_floating(VAL) )
		{
			/* Not yet supported */
			return ENOSYS;
		}
		
		if ( alur_floating(NUM) )
		{
			/* Not yet supported */
			return ENOSYS;
		}
		
		cpy %= alu_used( alu );
		alur_init_unsigned( alu, CPY, cpy );
		CPY.info = NUM.info;
		
		ret = alur_mov( alu, CPY, NUM );
		
		if ( ret == 0 )
		{
			bool carry = false, caught;
			alur_t TMP;
			alub_t v;
			size_t p;
			void *V;
			
			NUM.node %= alu_used( alu );
			VAL.node %= alu_used( alu );
			
			tmp %= alu_used( alu );
			alur_init_unsigned( alu, TMP, tmp );
			
			ret = IFTRUE( !NUM.node || !VAL.node || !CPY.node || !TMP.node, EINVAL );
			
			if ( ret )
			{
				alu_error( ret );
				return ret;
			}
			
			alur_clr( alu, NUM );
			
			V = alur_data( alu, VAL );
			v = alub( V, VAL.from );
			
			for ( p = v.bit; v.bit < VAL.upto; alub_inc(&v) )
			{
				if ( *(v.ptr) & v.mask )
				{	
					(void)alur__shl( alu, CPY, TMP, v.bit - p );
					ret = alur_add( alu, NUM, CPY );
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

int_t alur_mul
(
	alu_t *alu
	, alur_t NUM
	, alur_t VAL
)
{
	int ret;
	uint_t nodes[2];
	
	ret = alu_get_reg_nodes( alu, nodes, 2, 0 );
	
	if ( ret == 0 )
	{
		ret = alur_multiply( alu, NUM, VAL, nodes[0], nodes[1] );
		
		alu_rem_reg_nodes( alu, nodes, 2 );
		
		return ret;
	}
	
	alu_error( ret );
	return ret;
}

int_t alur_neg( alu_t *alu, alur_t NUM )
{
	int ret = alur_not( alu, NUM );
	
	if ( ret == 0 )
		return alur_inc( alu, NUM );
		
	alu_error( ret );
	return ret;
}

int_t alur_divide
(
	alu_t *alu
	, alur_t NUM
	, alur_t VAL
	, uint_t rem
	, uint_t tmp
)
{	
	if ( alu )
	{	
		int ret;
		alur_t SEG, REM, TMP;
		alub_t n;
		size_t bits = 0;
		bool nNeg, vNeg;
		void *N;
		
		NUM.node %= alu_used( alu );
		VAL.node %= alu_used( alu );
		
		rem %= alu_used( alu );
		alur_init_unsigned( alu, REM, rem );
		REM.info = NUM.info;
		
		tmp %= alu_used( alu );
		alur_init_unsigned( alu, TMP, tmp );
		
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
		
		nNeg = alur_below0( alu, NUM );
		vNeg = alur_below0( alu, VAL );
		
		if ( nNeg )
			alur_neg( alu, NUM );
		
		if ( vNeg )
			alur_neg( alu, VAL );
		
		(void)alur_mov_int2int( alu, REM, NUM );
		(void)alur_clr( alu, NUM );
		
		N = alur_data( alu, NUM );
		
		SEG = REM;
		
		n = alur_end_bit( alu, REM );
		SEG.upto = SEG.from = n.bit + 1;
		n = alub( N, NUM.from );
		
		while ( SEG.from > REM.from )
		{
			++bits;
			SEG.from--;
			if ( alur_cmp( alu, SEG, VAL ) >= 0 )
			{
				ret = alur_sub( alu, SEG, VAL );
				
				if ( ret == ENODATA )
					break;
				
				alur__shl( alu, NUM, TMP, bits );
				*(n.ptr) |= n.mask;
				bits = 0;
			}
		}
		
		if ( bits )
			alur__shl( alu, NUM, TMP, bits );
		
		if ( nNeg != vNeg )
			alur_neg( alu, NUM );
		
		if ( nNeg )
			alur_neg( alu, REM );
		
		if ( vNeg )
			alur_neg( alu, VAL );
		
		return ret;
	}
	
	return alu_err_null_ptr("alu");
}

int_t alur_div
(
	alu_t *alu
	, alur_t NUM
	, alur_t VAL
)
{
	int ret = 0;
	uint_t nodes[2] = {0};
	
	ret = alu_get_reg_nodes( alu, nodes, 2, 0 );
	
	if ( ret == 0 )
	{	
		ret = alur_divide( alu, NUM, VAL, nodes[0], nodes[1] );
		alu_rem_reg_nodes( alu, nodes, 2 );
		return ret;
	}
	
	alu_error( ret );
	return ret;
}

int_t alur_rem
(
	alu_t *alu
	, alur_t NUM
	, alur_t VAL
)
{
	int ret, tmpret;
	uint_t nodes[2];
	
	ret = alu_get_reg_nodes( alu, nodes, 2, 0 );
	
	if ( ret == 0 )
	{
		alur_t REM;
		
		if ( NUM.info & ALU_INFO_FLOAT )
		{
			alur_init_floating( alu, REM, nodes[0] );
		}
		else
		{
			alur_init_unsigned( alu, REM, nodes[0] );
			REM.info = NUM.info;
		}
		
		ret = alur_divide( alu, NUM, VAL, nodes[0], nodes[1] );
		
		switch ( ret )
		{
		case 0: case ENODATA: case EOVERFLOW:
			tmpret = ret;
			ret = alur_mov( alu, NUM, REM );
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

int_t alur__rol( alu_t *alu, alur_t NUM, alur_t TMP, size_t by )
{	
	if ( alu )
	{
		int ret;
		alub_t n, v;
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
		
		num_part = alur_data( alu, NUM );
		tmp_part = alur_data( alu, TMP );
		
		alur_mov( alu, TMP, NUM );
		
		n = alub( num_part, NUM.upto );
		v = alub( tmp_part, TMP.upto - by );
		
		while ( v.bit > TMP.from )
		{
			alub_dec(&v);
			alub_dec(&n);
			
			*(n.ptr) &= ~(n.mask);
			*(n.ptr) |= (n.mask * !!(*(v.ptr) & v.mask));
		}
		
		v = alub( tmp_part, TMP.upto );
		while ( n.bit > NUM.from )
		{
			alub_dec(&v);
			alub_dec(&n);
			
			*(n.ptr) &= ~(n.mask);
			*(n.ptr) |= (n.mask * !!(*(v.ptr) & v.mask));
		}
		
		return 0;
	}
	
	return alu_err_null_ptr("alu");
}

int_t alur__ror( alu_t *alu, alur_t NUM, alur_t TMP, size_t by )
{	
	if ( by )
	{
		int ret;
		alub_t n, v;
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
		
		num_part = alur_data( alu, NUM );
		tmp_part = alur_data( alu, TMP );
		
		alur_mov( alu, TMP, NUM );
		
		n = alub( num_part, NUM.from );
		v = alub( tmp_part, TMP.from + by );
		
		while ( v.bit < TMP.upto )
		{	
			*(n.ptr) &= ~(n.mask);
			*(n.ptr) |= IFTRUE( *(v.ptr) & v.mask, n.mask );

			alub_inc(&v);
			alub_inc(&n);
		}
		
		v = alub( tmp_part, TMP.from );
		while ( n.bit < NUM.upto )
		{
			*(n.ptr) &= ~(n.mask);
			*(n.ptr) |= IFTRUE( *(v.ptr) & v.mask, n.mask );
			
			alub_inc(&v);
			alub_inc(&n);
		}
	}
	
	return 0;
}

int_t alur__rotate
(
	alu_t *alu
	, alur_t NUM
	, alur_t VAL
	, alur_t TMP
	, func_alur__shift_t _shift
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
		
		alur_set_raw( alu, TMP, &by, sizeof(size_t), 0 );
		
		cmp = alur_cmp( alu, VAL, TMP );
		
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

size_t alu_lowest_upto( alur_t NUM, alur_t VAL )
{
	size_t ndiff, vdiff;
	
	ndiff = (NUM.upto - NUM.from);
	vdiff = (VAL.upto - VAL.from);
		
	return NUM.from + LOWEST(ndiff,vdiff);
}

int_t alur_and( alu_t *alu, alur_t NUM, alur_t VAL )
{	
	if ( alu )
	{
		int ret;
		alub_t n, v;
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
		
		part = alur_data( alu, NUM );
		n = alub( part, NUM.from );
		
		part = alur_data( alu, VAL );
		v = alub( part, VAL.from );
		
		for
		(
			; n.bit < pos
			; alub_inc(&n), alub_inc(&v)
		)
		{
			/* TODO: Do branchless version of this */
			if ( !(*(v.ptr) & v.mask) )
				*(n.ptr) &= ~(n.mask);
		}
		
		while ( n.bit < NUM.upto )
		{
			*(n.ptr) &= ~(n.mask);
			alub_inc(&n);
		}
		
		return 0;
	}
	
	return alu_err_null_ptr("alu");
}

int_t alur__or( alu_t *alu, alur_t NUM, alur_t VAL )
{	
	if ( alu )
	{
		int ret;
		alub_t n, v;
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
		
		part = alur_data( alu, NUM );
		n = alub( part, NUM.from );
		
		part = alur_data( alu, VAL );
		v = alub( part, VAL.from );
		
		for
		(
			; n.bit < pos
			; alub_inc(&n), alub_inc(&v)
		)
		{
			*(n.ptr) |= (*(v.ptr) & v.mask) ? n.mask : UNIC_SIZE_C(0);
		}
		
		return 0;
	}
	
	return alu_err_null_ptr("alu");
}

int_t alur_xor( alu_t *alu, alur_t NUM, alur_t VAL )
{	
	if ( alu )
	{
		int ret;
		alub_t n, v;
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
		
		part = alur_data( alu, NUM );
		n = alub( part, NUM.from );
		part = alur_data( alu, VAL );
		v = alub( part, VAL.from );
		
		for
		(
			; n.bit < pos
			; alub_inc(&n), alub_inc(&v)
		)
		{
			*(n.ptr) ^= (*(v.ptr) & v.mask) ? n.mask : UNIC_SIZE_C(0);
		}
		
		return 0;
	}
	
	return alu_err_null_ptr("alu");
}
