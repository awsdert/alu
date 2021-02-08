#include <alu/alup.h>
size_t alu_man_dig( size_t bits )
{
	size_t max_exp = bits - LDBL_MANT_DIG;
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
					, bits - max_exp
				)
			)
		)
	);
}

void alup__print
(
	char *file
	, uint_t line
	, const char *func
	, char *pfx
	, alup_t const * const _PTR
	, bool print_info
	, bool print_value
)
{
	alup_t _EXP, _MAN;
	
	alup_init_exponent( _PTR, _EXP );
	alup_init_mantissa( _PTR, _MAN );

	if ( print_info )
	{	
		alu__printf
		(
			"%s: part = %p, bits = %zu, upto = %zu, from = %zu"
			", signed = %c, mdig = %zu"
			", _EXP.bits = %zu, _MAN.bits = %zu"
			, file
			, line
			, func
			, pfx
			, _PTR->data
			, _PTR->bits
			, _PTR->upto
			, _PTR->from
			, '0' + alup___signed( _PTR )
			, _PTR->mdig
			, _EXP.bits
			, _MAN.bits
		);
	}
	
	if ( print_value )
	{
		alub_t p = alup_until_bit( _PTR );
		size_t bit = 0, bits = _PTR->bits, sep_at = 4;
		char sec_sep = ' ', dig_sep = '\'';
		
		fprintf( aluout, "%s:%u: %s() %s = ", file, line, func, pfx );
		
#define alup_print_bit() \
		do \
		{ \
			alub_dec( &p ); \
			fputc( '0' + !!(*(p.ptr) & p.mask), aluout ); \
			++bit; \
			--bits; \
		} \
		while (0)
		
		if ( alup___signed( _PTR ) )
		{
			alup_print_bit();
			
			(void)fputc( sec_sep, aluout );
		}
		
		if ( alup_floating( _PTR ) )
		{
			ssize_t exp = alup_get_exponent( _PTR );
			char str[bitsof(size_t)] = {0};
			
			if ( exp )
				exp -= alup_get_exponent_bias( _PTR );
			
			sprintf( str, "%+05zd%c", (ssize_t)exp, sec_sep );
			
			fputs( str, aluout );
			
			while ( p.bit > _EXP.from )
			{
				if ( bit >= sep_at )
				{
					//fputc( dig_sep, aluout );
					bit = 0;
				}
				
				alup_print_bit();
			}
			
			(void)fputc( sec_sep, aluout );
		}
		
		while ( p.bit > _PTR->from )
		{
			if ( bit >= sep_at )
			{
				fputc( dig_sep, aluout );
				bit = 0;
			}
			
			alup_print_bit();
		}
		
		fputc( '\n', aluout );
		
#undef alup_print_bit
	}
}

bool_t alup_below0( alup_t const * const _SRC )
{
	if ( _SRC && _SRC->data && alup___signed(_SRC) )
	{
		alub_t s = alup_final_bit( _SRC );
		return !!(*(s.ptr) & s.mask);
	}
	
	return false;
}

alub_t alup_first_bit_with_val( alup_t const * const _SRC, bool val )
{
	alub_t s = {0};
	
	if ( _SRC && _SRC->data )
	{
		/* Ensure is only 1 or 0 contained if maps to integer */
		val = !!val;
		
		s = alup_first_bit( _SRC );
			
		while ( s.bit < _SRC->last )
		{
			if ( !!( *(s.ptr) & s.mask ) == val )
				break;
			
			alub_inc(&s);
		}
	}
	
	return s;
}

alub_t alup_final_bit_with_val( alup_t const * const _SRC, bool val )
{
	alub_t s = {0};
	
	if ( _SRC && _SRC->data )
	{
		/* Ensure is only 1 or 0 contained if maps to integer */
		val = !!val;
		
		s = alup_final_bit( _SRC );
			
		while ( s.bit > _SRC->from )
		{
			if ( !!( *(s.ptr) & s.mask ) == val )
				break;
			
			alub_dec(&s);
		}
	}
	
	return s;
}


size_t alup_get_exponent( alup_t const * const _SRC )
{
	int_t ret;
	size_t dst = 0;
	alup_t _EXP, _DST;
	
	alup_init_unsigned( _DST, &dst, bitsof(size_t) );
	alup_init_exponent( _SRC, _EXP );
	
	ret = alup_mov_int2int( &_DST, &_EXP );
	if ( ret )
		alu_error( ret );
	
	return dst;
}

int_t alup_set_exponent( alup_t const * const _DST, size_t src )
{
	alup_t _EXP, _SRC;
	
	alup_init_unsigned( _SRC, &src, bitsof(size_t) );
	alup_init_exponent( _DST, _EXP );
	
	return alup_mov_int2int( &_EXP, &_SRC );
}

size_t alup_get_exponent_bias( alup_t const * const _SRC )
{
	alup_t _EXP;
	size_t bias = UNIC_SIZE_C(~0);
	alup_init_exponent( _SRC, _EXP );
	bias <<= _EXP.bits - 1;
	return ~bias;
}

int_t	alup_set( alup_t const * const _DST, bool fillwith )
{
	if ( _DST->data )
	{
		alub_t n;
		size_t upto = alup_until_pos( _DST );
		
		/* Force to either 1 or 0 */
		fillwith = !!fillwith;
		
		for ( n = alub( _DST->data, _DST->from ); n.bit < upto; alub_inc(&n) )
		{
			*(n.ptr) &= ~(n.mask);
			*(n.ptr) |= IFTRUE( fillwith, n.mask );
		}
		
		return 0;
	}
	
	return alu_err_null_ptr("_DST->data");
}

int_t	alup_set_intmax( alup_t const * const _DST )
{	
	int_t ret = alup_set( _DST, 1 );
	
	if ( ret == 0 )
	{
		if ( alup___signed( _DST ) )
		{
			alup_set_sign( _DST, 0 );
		}
		
		return 0;
	}
	
	return ret;
}

int_t	alup_set_intmin( alup_t const * const _DST )
{	
	int_t ret = alup_set( _DST, 0 );
	
	if ( ret == 0 )
	{
		if ( alup___signed( _DST ) )
		{
			alup_set_sign( _DST, 1 );
		}
		
		return 0;
	}
	
	return ret;
}

int_t	alup_set_fltinf( alup_t const * const _DST, bool_t neg )
{
	alup_t _MAN;
	size_t bias = alup_get_exponent_bias( _DST );
	
	alup_set_sign( _DST, neg );
	alup_set_exponent( _DST, (bias << 1) | 1 );
	alup_init_mantissa( _DST, _MAN );
	
	return alup_set( &_MAN, 0 );
}

int_t	alup_set_max( alup_t const * const _DST )
{
	if ( alup_floating( _DST ) )
	{
		return alup_set_fltinf( _DST, 0 );
	}
	
	return alup_set_intmax( _DST );
}

int_t	alup_set_min( alup_t const * const _DST )
{
	if ( alup_floating( _DST ) )
	{
		return alup_set_fltinf( _DST, 1 );
	}
	
	return alup_set_intmin( _DST );
}

int_t	alup_set_inf( alup_t const * const _DST, bool_t neg )
{
	if ( alup_floating( _DST ) )
	{
		return alup_set_fltinf( _DST, neg );
	}
	
	/* Integers do not support infinity, min/max is closest we can get */
	
	if ( neg )
	{
		return alup_set_intmin( _DST );
	}
	
	return alup_set_intmax( _DST );
}

int_t	alup_mov_int2int( alup_t const * const _DST, alup_t const * const _SRC )
{
	if ( _DST->data )
	{	
		if ( _SRC->data )
		{
			alub_t
				d = alub( _DST->data, _DST->from )
				, s = alub( _SRC->data, _SRC->from );
			size_t upto = _DST->from + LOWEST( _DST->bits, _SRC->bits );
			bool neg = alup_below0( _SRC );
			
			for ( ; d.bit < upto; alub_inc(&d), alub_inc(&s) )
			{
				*(d.ptr) &= ~(d.mask);
				*(d.ptr) |= IFTRUE( *(s.ptr) & s.mask, d.mask );
			}

			upto = _DST->from + _DST->bits;
			
			for ( ; d.bit < upto; alub_inc(&d) )
			{
				*(d.ptr) &= ~(d.mask);
				*(d.ptr) |= IFTRUE( neg, d.mask );
			}
			
			return IFTRUE( d.bit < (_DST->from + _SRC->bits), ERANGE );
		}
		
		return alup_set( _DST, 0 );
	}
	
	return alu_err_null_ptr("_DST->data");
}

int_t	alup_mov_int2flt( alup_t const * const _DST, alup_t const * const _SRC )
{
	alub_t s;
	ssize_t exp, bias;
	bool neg = alup_below0( _SRC ), is0, val;
	
	if ( neg )
	{
		alup_neg( _SRC );
	}
	
	s = alup_final_bit_with_val( _SRC, 1 );
	val = !!(*(s.ptr) & s.mask);
	exp = s.bit - _SRC->from;
	is0 = !(neg || exp || val != neg);
	
	if ( is0 )
	{	
		if ( neg )
		{
			alup_neg( _SRC );
		}
		
		(void)alup_set( _DST, 0 );
		return ENODATA;
	}
	
	bias = alup_get_exponent_bias( _DST );
	
	if ( exp >= bias )
	{
		if ( neg )
		{
			alup_neg( _SRC );
		}
		
		(void)alup_set_inf( _DST, neg );
		return ERANGE;
	}
	else
	{
		alup_t _MAN, _REF;
		ssize_t upto = LOWEST( exp + 1, (ssize_t)(_DST->mdig) );
		
		alup_init_mantissa( _DST, _MAN );
		alup_set( &_MAN, 0 );
		
		_REF = *_SRC;
		_REF.bits = upto - 1;
		_REF.from = IFTRUE( s.bit, s.bit - _REF.bits );
		_MAN.from += ( _DST->mdig - upto );
		
		(void)alup_mov_int2int( &_MAN, &_REF );
		
		if ( neg )
		{
			alup_neg( _SRC );
		}
		
		/* Round to nearest */
		if ( _SRC->from < _REF.from )
		{
			s = alub( _REF.data, _REF.from - 1 );
			
			if ( *(s.ptr) & s.mask )
				alup_inc( &_MAN );
		}

		(void)alup_set_exponent( _DST, exp + bias );
		alup_set_sign( _DST, neg );
		
		return 0;
	}
}

int_t	alup_mov_flt2int( alup_t const * const _DST, alup_t const * const _SRC )
{
	int_t ret;
	alup_t _MAN;
	alub_t s;
	ssize_t exp = alup_get_exponent( _SRC )
		, bias = alup_get_exponent_bias(_SRC)
		, inf = (bias << 1) | 1;
	bool Inf, neg;
	
	alup_init_mantissa( _SRC, _MAN );
	
	neg = alup_below0( _SRC );
	Inf = (exp == inf);
	
	if ( !exp )
	{
		return alup_set( _DST, 0 );
	}
	
	exp -= bias;
	
	if ( Inf || exp >= (ssize_t)(_DST->bits) )
	{
		s = alup_final_bit_with_val( &_MAN, true );
		
		/* NaN cannot be recorded by an integer, use 0 instead */
		if ( Inf && *(s.ptr) & s.mask )
		{
			alup_set( _DST, 0 );
			return 0;
		}
		
		return alup_set_inf( _DST, neg );
	}
	
	if ( exp < 0 )
	{
		return alup_set( _DST, 0 );
	}
	
	_MAN.from += _MAN.bits - LOWEST( exp, (ssize_t)(_MAN.bits) );
	(void)alup_mov_int2int( _DST, &_MAN );
	ret = alup__shl_int2int( _DST, exp );
	/* Put in the assumed bit */
	alub_set( _DST->data, _DST->from + exp, 1 );
	
	if ( neg )
	{
		alup_neg( _DST );
	}

	return ret;
}

int_t	alup_mov_flt2flt( alup_t const * const _DST, alup_t const * const _SRC )
{
	alup_t _DMAN, _SMAN;
	ssize_t exp, dbias, sbias;
	bool neg = alup_below0( _SRC );
	int_t ret = 0;
	
	exp = alup_get_exponent( _SRC );
	
	if ( !exp )
	{
		return alup_set( _DST, 0 );
	}
	
	dbias = alup_get_exponent_bias( _DST );
	sbias = alup_get_exponent_bias( _SRC );
	
	alup_init_mantissa( _DST, _DMAN );
	alup_init_mantissa( _SRC, _SMAN );
	
	/* Make sure we refer to upper bits of both mantissa's if they're of
	 * different lengths */
	if ( _DST->mdig > _SRC->mdig )
	{	
		alup_set( &_DMAN, 0 );
		_DMAN.bits = _SRC->mdig;
		_DMAN.from += _DST->mdig - _SRC->mdig;
	}
	else if ( _SRC->mdig > _DST->mdig )
	{
		_SMAN.bits = _DST->mdig;
		_SMAN.from += _SRC->mdig - _DST->mdig;
		ret = ERANGE;
	}
	
	if ( exp >= ((sbias << 1) | 1) )
	{
		alup_set_sign( _DST, neg );
		(void)alup_set_exponent( _DST, (dbias << 1) | 1 );	
		/* If SRC was +/-NaN then this will set that */
		return alup_mov_int2int( &_DMAN, &_SMAN );
	}
	
	exp -= sbias;
	
	if ( exp >= dbias || exp <= -dbias )
	{	
		return alup_set_inf( _DST, neg );
	}
	
	alup_set_sign( _DST, neg );
	(void)alup_set_exponent( _DST, exp + dbias );
	(void)alup_mov_int2int(  &_DMAN, &_SMAN );
	return ret;
}

int_t	alup_mov( alup_t const * const _DST, alup_t const * const _SRC )
{
	if ( alup_floating( _DST ) )
	{
		if ( alup_floating( _SRC ) )
			return alup_mov_flt2flt( _DST, _SRC );
		return alup_mov_int2flt( _DST, _SRC );
	}
	else
	{
		if ( alup_floating( _SRC ) )
			return alup_mov_flt2int( _DST, _SRC );
		return alup_mov_int2int( _DST, _SRC );
	}
}

int_t alup_not( alup_t const * const _DST )
{
	if ( _DST->data )
	{
		alub_t d, e;
		size_t mask, mask_init, mask_last;
		
		d = alup_first_bit( _DST );
		e = alup_final_bit( _DST );
		
		mask = 0;
		mask_init = mask_last = ~mask;
		
		mask_init <<= d.pos;
		mask_last <<= e.pos + 1;
		mask_last = ~mask_last;
		
		mask = EITHER( e.seg > d.seg, mask_last, mask_last & mask_init );
		mask = EITHER( mask, mask, mask_init );
		
		*(d.ptr) ^= IFTRUE( e.seg > d.seg, mask_init );
		
		for ( ++(d.seg); d.seg < e.seg; ++(d.seg) )
		{
			++(d.ptr);
			d.ptr[d.seg] = ~(d.ptr[d.seg]);
		}
		
		*(e.ptr) ^= mask;
		
		return 0;
	}
	
	return alu_err_null_ptr("_DST->data");
}

int_t alup_cmp_int2int( alup_t const * const _NUM, alup_t const * const _VAL )
{
	int a = !!(_NUM && _NUM->data && _NUM->upto > _NUM->from)
		, b = !!(_VAL && _VAL->data && _VAL->upto > _VAL->from)
		, r = a - b;
	
	if ( r )
		return r;
		
	a = alup_below0( _NUM );
	b = alup_below0( _VAL );
	r = a - b;
	
	if ( r )
		return -r;
	else
	{
		alub_t n = alup_until_bit( _NUM ), v = alup_until_bit( _VAL );
		size_t nlen = n.bit - _NUM->from, vlen = v.bit - _VAL->from;
		
		while ( nlen > vlen )
		{
			--nlen;
			alub_dec(&n);
			
			a = !!(*(n.ptr) & n.mask);
			r = a - b;
			
			if ( r )
				return -r;
		}
		
		while ( vlen > nlen )
		{
			--vlen;
			alub_dec(&v);
			
			b = !!(*(v.ptr) & v.mask);
			r = a - b;
			
			if ( r )
				return r;
		}
		
		while ( nlen )
		{
			--nlen;
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

int_t alup_cmp( alup_t const * const _NUM, alup_t const * const _VAL )
{
	alup_t _VMAN;
	ssize_t nexp, vexp, vbias;
	bool_t nneg = alup_below0( _NUM ), vneg = alup_below0( _VAL ), vhad = 0;
	int_t ret = (nneg < vneg) - (nneg > vneg);
	
	if ( ret )
		return ret;
	
	if ( alup_floating( _NUM ) )
	{
		alup_t _NMAN;
		bool_t nhad;
		ssize_t nbias = alup_get_exponent_bias( _NUM );
		
		alup_init_mantissa( _NUM, _NMAN );
		
		nexp = alup_get_exponent( _NUM );
		nhad = !!nexp;
		nexp -= IFTRUE( nhad, nbias );
		
		if ( alup_floating( _VAL ) )
		{
			vbias = alup_get_exponent_bias( _VAL );
			
			vexp = alup_get_exponent( _VAL );
			vhad = !!vexp;
			vexp -= IFTRUE( vhad, vbias );			
			
			ret = (nexp > vexp) - (nexp < vexp);
			
			if ( ret == 0 )
			{					
				alup_init_mantissa( _VAL, _VMAN );
				
				alub_set( _NUM->data, alup_until_pos(&_NMAN), 1 );
				alub_set( _VAL->data, alup_until_pos(&_VMAN), 1 );
				
				_NMAN.bits++;
				_VMAN.bits++;
				
				// FIXME: Doesn't account for different size mantissas
				
				ret = alup_cmp_int2int( &_NMAN, &_VMAN );
				
				alup_set_exponent( _NUM, nexp + IFTRUE( nhad, nbias ) );
				alup_set_exponent( _VAL, vexp + IFTRUE( vhad, vbias ) );
			}
		}
		
		return ret;
	}
	else if ( alup_floating( _VAL ) )
	{
		alub_t n;
		
		if ( nneg )
			alup_neg( _NUM );
		
		n = alup_final_bit_with_val( _NUM, true );
		
		vbias = alup_get_exponent_bias( _VAL );
		
		nexp = n.bit - _NUM->from;
		vexp = alup_get_exponent( _VAL );
		vhad = !!vexp;
		vexp -= IFTRUE( vhad, vbias );
		
		ret = (nexp > vexp) - (nexp < vexp);
		
		if ( ret == 0 )
		{
			
			alup_init_mantissa( _VAL, _VMAN );
			
			alub_set( _VAL->data, alup_until_pos( &_VMAN ), 1 );
			
			_VMAN.bits++;
			_VMAN.from += IFTRUE
			(
				vexp >= 0 && vexp <= (ssize_t)(_VMAN.bits)
				,  _VMAN.bits - vexp
			);
			
			ret = alup_cmp_int2int( _NUM, &_VMAN );
			
			alup_set_exponent( _VAL, vexp + IFTRUE( vhad, vbias ) );
		}
		
		if ( nneg )
			alup_neg( _NUM );
			
		return ret;
	}
	
	return alup_cmp_int2int( _NUM, _VAL );
}

int_t alup__inc_int( alup_t const * const _SRC )
{
	alub_t n;
	bool_t is1 = true;
	size_t upto = alup_until_pos( _SRC );
	
	for
	(
		n = alub( _SRC->data, _SRC->from )
		; n.bit < upto
		; alub_inc(&n)
	)
	{
		is1 = !!(*(n.ptr) & n.mask);
		*(n.ptr) &= ~(n.mask);
		*(n.ptr) |= IFTRUE( !is1, n.mask );
		if ( !is1 ) return 0;
	}
	
	return EOVERFLOW;
}

int_t alup_inc( alup_t const * const _SRC )
{	
	if ( alup_floating( _SRC ) )
	{	
		bool_t neg = alup_get_sign( _SRC );
		ssize_t exp = alup_get_exponent( _SRC )
			, bias = alup_get_exponent_bias( _SRC ), mdig = _SRC->mdig;
		
		if ( exp )
		{
			exp -= bias;
		
			if ( exp >= 0 && exp <= mdig )
			{
				alup_t _EXP, _MAN;
				alub_t e;
				
				alup_init_exponent( _SRC, _EXP );
				
				_EXP.upto = _SRC->upto;
				alup_set( &_EXP, 0 );
				
				alup_init_mantissa( _SRC, _MAN );
				
				e = alub( _MAN.data, _MAN.upto );
				*(e.ptr) |= e.mask;
				
				_MAN.from = _MAN.upto - exp;
				_MAN.upto++;
				
				alup__inc_int( &_MAN );
				
				/* Normalise */
				if ( !(*(e.ptr) & e.mask) )
				{
					if ( neg )
					{
						--exp;
						alup__shl_int2int( &_MAN, 1 );
						neg = (exp == 0);
					}
					else
					{
						++exp;
						alup__shr_int2int( &_MAN, 1 );
					}
				}
				
				alup_set_sign( _SRC, neg );
				return alup_set_exponent( _SRC, exp + bias );
			}
			
			return ERANGE;
		}
		
		alup_set_sign( _SRC, false );
		return alup_set_exponent( _SRC, bias );
	}
	
	return alup__inc_int( _SRC );
}

int_t alup__dec_int( alup_t const * const _SRC )
{
	alub_t n;
	bool_t is0 = true;
	size_t upto = alup_until_pos( _SRC );
	
	for
	(
		n = alub( _SRC->data, _SRC->from )
		; n.bit < upto
		; alub_inc(&n)
	)
	{
		is0 = !(*(n.ptr) & n.mask);
		*(n.ptr) &= ~(n.mask);
		if ( !is0 ) return 0;
		*(n.ptr) |= n.mask;
	}
	
	return EOVERFLOW;
}

int_t alup_dec( alup_t const * const _SRC )
{	
	if ( alup_floating( _SRC ) )
	{
		bool_t neg = alup_get_sign( _SRC );
		ssize_t exp = alup_get_exponent( _SRC )
			, bias = alup_get_exponent_bias( _SRC ), mdig = _SRC->mdig;
		
		if ( exp )
		{	
			exp -= bias;
			
			if ( exp >= 0 && exp <= mdig )
			{
				int ret;
				alup_t _MAN;
				alub_t e;
				
				alup_init_mantissa( _SRC, _MAN );
				
				e = alub( _MAN.data, _MAN.upto );
				*(e.ptr) |= e.mask;
				
				_MAN.from = _MAN.upto - exp;
				_MAN.upto++;
				
				ret = alup__dec_int( &_MAN );
				
				if ( ret == EOVERFLOW )
				{
					++exp;
					alup_set_sign( _SRC, true );
					alup__shr_int2int( &_MAN, 1 );
				}
				else if ( !( *(e.ptr) & e.mask ) )
				{
					if ( neg )
					{
						++exp;
						alup__shr_int2int( &_MAN, 1 );
					}
					else if ( exp == 0 )
					{
						_MAN.upto--;
						_MAN.from = _SRC->from;
						e = alup_final_bit_with_val( &_MAN, true );
						exp = !!(*(e.ptr) & e.mask) * (_MAN.upto - e.bit);
						exp = -exp;
					}
					else
					{
						--exp;
						alup__shl_int2int( &_MAN, 1 );
					}
				}
				
				neg = !!((*(e.ptr) & e.mask) || exp);
				return alup_set_exponent( _SRC, neg * (bias + exp) );
			}
			
			return ERANGE;
		}
		
		alup_set_sign( _SRC, true );
		return alup_set_exponent( _SRC, bias );
	}
	
	return alup__dec_int( _SRC );
}

int_t alup_match_exponents( void *_num, void *_val, size_t bits )
{
	int_t ret;
	alup_t _NUM, _VAL, _DST;
	size_t exp = 0, nexp, vexp, diff = 0;
	bool truncated = false;
	
	alup_init_floating( _NUM, _num, bits );
	alup_init_floating( _VAL, _val, bits );
	
	nexp = alup_get_exponent( &_NUM );
	vexp = alup_get_exponent( &_VAL );
		
	if ( nexp > vexp )
	{
		exp = nexp;
		diff = nexp - vexp;
		_DST = _VAL;
	}
	else if ( vexp > nexp )
	{
		exp = vexp;
		diff = vexp - nexp;
		_DST = _NUM;
	}
	
	if ( diff )
	{
		alup_t _MAN;
		
		alup_init_mantissa( &_DST, _MAN );
			
		/* Match exponent and align mantissa */
		(void)alup_set_exponent( &_DST, exp );
		
		ret = alup__shr_int2int( &_MAN, diff );
		
		truncated = (ret == ERANGE);
	
		/* Insert assumed bit back into position */
		if ( diff < _MAN.bits )
		{
			alub_set( _MAN.data, _MAN.bits - diff, 1 );
		}
	}
	
	return IFTRUE( truncated, ERANGE );
}

typedef int_t (*alup__addsub_int2int)( alup_t const * const _NUM, alup_t const * const _VAL );

int_t alup_addsub
(
	alup_t const * const _NUM
	, alup_t const * const _VAL
	, void *_cpy
	, void *_tmp
	, bool adding
)
{
	alup__addsub_int2int addsub =
		adding ? alup__add_int2int : alup__sub_int2int;
	
	if ( alup_floating( _NUM ) || alup_floating( _VAL ) )
	{
		int_t ret;
		alup_t _CPY, _TMP, _CEXP, _TEXP, _DST;
		ssize_t cexp, texp;
		size_t bits = HIGHEST( _NUM->bits, _VAL->bits );
		bool_t truncated = false;
		
		//alu_printf( "adding = %s", adding ? "true" : "false" );
		
		alup_init_floating( _CPY, _cpy, bits );
		alup_init_floating( _TMP, _tmp, bits );
		
		(void)alup_mov( &_CPY, _NUM );
		(void)alup_mov( &_TMP, _VAL );
		
		/* Get exponents */
		alup_init_exponent( &_CPY, _CEXP );
		alup_init_exponent( &_TMP, _TEXP );
		
		alup_init___signed( _DST, NULL, bitsof(ssize_t) );
		
		_DST.data = &cexp;
		alup_mov_int2int( &_DST, &_CEXP );
		
		_DST.data = &texp;
		alup_mov_int2int( &_DST, &_TEXP );
		
		/* Use exponents */
		if ( cexp || texp )
		{
			alub_t final
				, csign = alup_final_bit( &_CPY )
				, tsign = alup_final_bit( &_TMP )
				/* Assumed mantissa bits */
				, cmbit
				, tmbit;
			bool_t neg
				, cneg = *(csign.ptr) & csign.mask
				, tneg = *(tsign.ptr) & tsign.mask;
			alup_t _CMAN, _TMAN, __CPY, __TMP;
			size_t //bias = alup_get_exponent_bias( &_CPY ),
				exp = 0, diff = 0, shift = _CEXP.bits - adding;
			
			alup_init_mantissa( &_CPY, _CMAN );
			alup_init_mantissa( &_TMP, _TMAN );
			
			alup_init_unsigned( __CPY, _cpy, bits );
			alup_init_unsigned( __TMP, _tmp, bits );
			
			if ( cexp > texp )
			{
				exp = cexp;
				diff = cexp - texp;
				_DST = __TMP;
			}
			else if ( texp > cexp )
			{
				exp = texp;
				diff = texp - cexp;
				_DST = __CPY;
			}
			else exp = cexp;
			
			/* Prevent curruption of values by removing unneeded signs */
			*(csign.ptr) &= ~(csign.mask);
			*(tsign.ptr) &= ~(tsign.mask);
			
			/* Insert assumed bits */
			cmbit = alup_until_bit( &_CMAN );
			tmbit = alup_until_bit( &_TMAN );
			
			alub_set_val( cmbit, !!cexp );
			alub_set_val( tmbit, !!texp );
			
			/* Convert mantissa/s to integer representation
			 * CHECKME: I may have misunderstood a set of results here */
			if ( cneg ) alup_neg( &__CPY );
			if ( tneg ) alup_neg( &__TMP );
			
			/* Minimise potential loss of bits, shift is done on added
			 * mantissas as well to simplify the exponent correction later*/
			alup__shl_int2int( &__CPY, shift );
			alup__shl_int2int( &__TMP, shift );
			
			/* Align significant digits to match positions */
			if ( diff )
			{
				_DST.sign = false;
				ret = alup__shr_int2int( &_DST, diff );
				truncated = (ret == ERANGE);
			}
			
			ret = addsub( &__CPY, &__TMP );
			
			neg = (cneg != tneg || (!cneg && ret == EOVERFLOW));
			
			bits = _CEXP.last;
			
			final = alup_final_bit_with_val( &__CPY, !neg );
			
			tneg = final.bit < bits;
			texp = EITHER( tneg, -1, 1 )
				* (
					(ssize_t)EITHER
					(
						tneg
						, bits - final.bit
						, final.bit - bits
					)
				);
			exp += texp;
			
			bits = _CMAN.last + !tneg;
			if ( bits >= final.bit )
			{
				alup__shl_int2int( &__CPY, bits - final.bit );
			}
			else
			{
				ret = alup__shr_int2int( &__CPY, (final.bit - bits) );
				truncated = (truncated || ret == ERANGE);
			}
			
			alup_set_sign( &_CPY, neg );
			
			(void)alup_set_exponent( &_CPY, exp );
		}
		
		ret = alup_mov( _NUM, &_CPY );
		
		return IFTRUE( truncated || ret == ERANGE, ERANGE );
	}
	
	return addsub( _NUM, _VAL );
}

int_t alup__add_int2int( alup_t const * const _NUM, alup_t const * const _VAL )
{
	bool_t changed = false, carry = false, active, append;
	alub_t
		n = alub( _NUM->data, _NUM->from )
		, v = alub( _VAL->data, _VAL->from );
	size_t stop = _NUM->from + LOWEST( _NUM->bits, _VAL->bits );
	
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
	
	stop = alup_until_pos( _VAL );
	for ( ; !changed && v.bit < stop; alub_inc(&v) )
	{
		changed = EITHER( changed, true, !!(*(v.ptr) & v.mask) );
	}
	
	stop = alup_until_pos( _NUM );
	for ( ; carry && n.bit < stop; alub_inc(&n) )
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

int_t alup__add( alup_t const * const _NUM, alup_t const * const _VAL, void *_cpy, void *_tmp )
{
#if 1
	if ( alup_floating( _NUM ) || alup_floating( _VAL ) )
	{
		int_t ret;
		alup_t _CPY, _TMP, _CMAN, _TMAN;
		size_t exp, cexp, texp;
		size_t bits = HIGHEST( _NUM->bits, _VAL->bits );
		bool_t truncated = false;
		
		alup_init_floating( _CPY, _cpy, bits );
		alup_init_floating( _TMP, _tmp, bits );
		
		alup_init_mantissa( &_CPY, _CMAN );
		alup_init_mantissa( &_TMP, _TMAN );
		
		alup_mov( &_CPY, _NUM );
		alup_mov( &_TMP, _VAL );
		
		cexp = alup_get_exponent( &_CPY );
		texp = alup_get_exponent( &_TMP );
		
		ret = alup_match_exponents( _CPY.data, _TMP.data, bits );
		
		truncated = (ret == ERANGE);
		
		exp = alup_get_exponent( &_CPY );
		
		ret = alup__add_int2int( &_CMAN, &_TMAN );
		
		if ( cexp || texp )
		{
			if ( ret == EOVERFLOW || cexp == texp )
			{
				size_t both = 1 + (ret == EOVERFLOW && cexp == texp);
				alub_t c = alub( _CMAN.data, _CMAN.bits - 1 );
				
				++exp;
				alup__shr_int2int( &_CMAN, 1 );
				
				*(c.ptr) &= ~(c.mask);
				*(c.ptr) |= IFTRUE( both == 2, c.mask );
			}
		}
		
		(void)alup_set_exponent( &_CPY, exp );
		
		ret = alup_mov( _NUM, &_CPY );
		
		return IFTRUE( truncated || ret == ERANGE, ERANGE );
	}
	
	return alup__add_int2int( _NUM, _VAL );
#else
	return alup_addsub( _NUM, _VAL, _cpy, _tmp, alup__add_int2int );
#endif
}

int_t alup__sub_int2int( alup_t const * const _NUM, alup_t const * const _VAL )
{
	bool_t changed = false, carry = false, active, append;
	alub_t n = alup_first_bit( _NUM ), v = alup_first_bit( _VAL );
	size_t stop = _NUM->from + LOWEST( _NUM->bits, _VAL->bits );
	
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
	
	stop = alup_until_pos( _VAL );
	for ( ; !changed && v.bit < stop; alub_inc(&v) )
	{
		changed = EITHER( changed, true, !!(*(v.ptr) & v.mask) );
	}
	
	stop = alup_until_pos( _NUM );
	for ( ; carry && n.bit < stop; alub_inc(&n) )
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

int_t alup__sub( alup_t const * const _NUM, alup_t const * const _VAL, void *_cpy, void *_tmp )
{
#if 0
	if ( alup_floating( _NUM ) || alup_floating( _VAL ) )
	{
		int_t ret;
		alup_t _CPY, _TMP;
		ssize_t cexp, texp;
		size_t bits = HIGHEST( _NUM->bits, _VAL->bits );
		bool_t truncated = false;
		
		alup_init_floating( _CPY, _cpy, bits );
		alup_init_floating( _TMP, _tmp, bits );
		
		alup_mov( &_CPY, _NUM );
		alup_mov( &_TMP, _VAL );
		
		cexp = alup_get_exponent( &_CPY );
		texp = alup_get_exponent( &_TMP );
		
		if ( cexp || texp )
		{
			bool_t neg = alup_below0( &_CPY );
			alub_t final;
			alup_t _CMAN, _TMAN;
			ssize_t bias = alup_get_exponent_bias( &_CPY ), exp;
			
			ret = alup_match_exponents( _CPY.data, _TMP.data, bits );
			
			truncated = (ret == ERANGE);
			
			exp = alup_get_exponent( &_CPY );
			
			alup_init_mantissa( &_CPY, _CMAN );
			alup_init_mantissa( &_TMP, _TMAN );
			
			ret = alup__sub_int2int( &_CMAN, &_TMAN );
			
			if ( ret == EOVERFLOW )
			{
				neg = !neg;
				alup_neg( &_CMAN );
			}
			
			final = alup_final_bit( &_CMAN );
			exp = _CMAN.upto - final.bit;
			
			alup_set_sign( &_CPY, neg );
			
			(void)alup_set_exponent( &_CPY, IFTRUE( exp, exp + bias ) );
		}
		
		ret = alup_mov( _NUM, &_CPY );
		
		return IFTRUE( truncated || ret == ERANGE, ERANGE );
	}
	
	return alup__sub_int2int( _NUM, _VAL );
#else
	return alup_addsub( _NUM, _VAL, _cpy, _tmp, false );
#endif
}

int_t alup__shl_int2int( alup_t const * const _NUM, size_t by )
{	
	int_t ret = 0;
	
	if ( by )
	{
		size_t diff;
		alup_t _TMP = {0};
			
		/* Mantissa will use same bits but exponent will be greater */
		if ( alup_floating( _NUM ) )
		{
			alup_t _EXP;
			
			alup_init_exponent( _NUM, _EXP );
			alup_init_unsigned( _TMP, &by, bitsof(size_t) );
			
			return alup__add_int2int( &_EXP, &_TMP );
		}
		
		diff = _NUM->bits;
		
		if ( by >= diff )
			return alup_set( _NUM, 0 );
		else
		{
			alub_t d = alup_until_bit( _NUM ), s = d;
			
			while ( by )
			{
				--by;
				alub_dec(&s);
				
				ret = IFTRUE( ret || *(s.ptr) & s.mask, ERANGE );
			}
			
			while ( s.bit > _NUM->from )
			{
				alub_dec(&s);
				alub_dec(&d);
				
				*(d.ptr) &= ~(d.mask);
				*(d.ptr) |= IFTRUE( *(s.ptr) & s.mask, d.mask );
			}
			
			while ( d.bit > _NUM->from )
			{
				alub_dec(&d);
				
				*(d.ptr) &= ~(d.mask);
			}
		}
	}
	
	return ret;
}

int_t alup__shr_int2int( alup_t const * const _NUM, size_t by )
{	
	int_t ret = 0;
	
	if ( by )
	{
		alup_t _TMP;
		
		/* Mantissa will use same bits but exponent will be lesser */
		if ( alup_floating( _NUM ) )
		{
			alup_t _EXP;
			
			alup_init_exponent( _NUM, _EXP );
			alup_init_unsigned( _TMP, &by, bitsof(size_t) );
			
			return alup__sub_int2int( &_EXP, &_TMP );
		}
		else if ( by >= _NUM->bits )
		{
			return alup_set( _NUM, alup_get_sign( _NUM ) );
		}
		else
		{
			bool_t neg = alup_below0( _NUM );
			alub_t d = alub( _NUM->data, _NUM->from ), s = d;
			size_t upto = alup_until_pos( _NUM );
			
			while ( by )
			{
				--by;
				
				ret = IFTRUE( ret || *(s.ptr) & s.mask, ERANGE );
				
				alub_inc(&s);
			}
			
			while ( s.bit < upto )
			{
				*(d.ptr) &= ~(d.mask);
				*(d.ptr) |= IFTRUE( *(s.ptr) & s.mask, d.mask );
				
				alub_inc(&s);
				alub_inc(&d);
			}
			
			while ( d.bit < upto )
			{	
				*(d.ptr) &= ~(d.mask);
				*(d.ptr) |= IFTRUE( neg, d.mask );
				
				alub_inc(&d);
			}
		}
	}
	
	return ret;
}

int_t alup_shl( alup_t const * const _NUM, alup_t const * const _VAL )
{
	int_t ret;
	size_t mov;
	alup_t _MOV;

	alup_init_unsigned( _MOV, &mov, bitsof(size_t) );
	
	ret = alup_mov( &_MOV, _VAL );
	mov = EITHER( ret, UNIC_SIZE_C(~0), mov );
	
	return alup__shl_int2int( _NUM, mov );
}

int_t alup_shr( alup_t const * const _NUM, alup_t const * const _VAL )
{
	int_t ret;
	size_t mov;
	alup_t _MOV;

	alup_init_unsigned( _MOV, &mov, bitsof(size_t) );
	
	ret = alup_mov( &_MOV, _VAL );
	mov = EITHER( ret, UNIC_SIZE_C(~0), mov );
	
	return alup__shr_int2int( _NUM, mov );
}

int_t alup__neg_int( alup_t const * const _SRC )
{
	int ret = alup_not( _SRC );
	
	if ( ret == 0 )
		return alup__inc_int( _SRC );
		
	alu_error( ret );
	return ret;
}

int_t alup_neg( alup_t const * const _SRC )
{
	if ( alup_floating( _SRC ) )
	{
		alub_t sign = alup_final_bit( _SRC );
		bool_t neg = !(*(sign.ptr) & sign.mask);
		
		*(sign.ptr) &= ~(sign.mask);
		*(sign.ptr) |= IFTRUE( neg, sign.mask );
	
		return 0;
	}
	
	return alup__neg_int( _SRC );
}

int_t alup__rol_int2int( alup_t const * const _NUM, size_t by )
{
	size_t diff = _NUM->bits;
	
	if ( by && diff )
	{	
		alub_t d, w;
		uintmax_t was, tmp;
		alup_t _SEG, _TMP;
		size_t cap, upto = alup_until_pos( _NUM );
		
		_SEG = *_NUM;
		alup_init_unsigned( _TMP, &tmp, bitsof(uintmax_t) );
		
		by %= diff;
		
		d = alub( _NUM->data, _NUM->from );
		
		_SEG.from = upto - by;
		alup_mov_int2int( &_TMP, &_SEG );
		cap = by;
		
		while ( d.bit < upto )
		{
			was = tmp;
			
			_SEG.from = d.bit;
			alup_mov_int2int( &_TMP, &_SEG );
			
			for
			(
				w = alub( &was, 0 )
				; w.bit < cap && d.bit < upto
				; alub_inc(&w), alub_inc(&d)
			)
			{	
				*(d.ptr) &= ~(d.mask);
				*(d.ptr) |= IFTRUE( *(w.ptr) & w.mask, d.mask );
			}
			
			cap = bitsof(uintmax_t);
		}
	}
	
	return 0;
}

int_t alup__rol( alup_t const * const _NUM, void *_tmp, size_t by )
{
	if ( alup_floating( _NUM ) )
	{
		ssize_t exp = alup_get_exponent( _NUM );
		
		if ( exp )
		{
			size_t bias = alup_get_exponent_bias( _NUM ), man;
			alup_t _MAN, _TMP;
			alub_t t;
			bool_t neg = alup_below0( _NUM );
			size_t upto = alup_until_pos( _NUM ), tupto;
			
			alup_init_mantissa( _NUM, _MAN );
			man = bias + _MAN.bits;
			exp -= bias;
			
			alup_init_unsigned( _TMP, _tmp, man );
			tupto = alup_until_pos( &_TMP );
			
			alub_set( _MAN.data, alup_until_pos( &_MAN ), 1 );
			_MAN.bits++;
			
			_TMP.from = tupto - _MAN.bits;
			(void)alup_mov_int2int( &_TMP, &_MAN );
			
			_TMP.from = 0;
			(void)alup__rol_int2int( &_TMP, by );
			
			t = alup_final_bit_with_val( &_TMP, true );
			
			/* Set Sign */
			
			alub_set
			(
				_NUM->data
				, upto - 1
				, neg && (t.bit || *(t.ptr) & t.mask)
			);
			
			/* Set Exponent */
			
			alup_set_exponent( _NUM, t.bit + bias );
			
			/* Set Mantissa */
			
			_MAN.bits--;
			_TMP.bits = t.bit - _TMP.from;
			_TMP.from = IFTRUE( _TMP.bits >= _MAN.bits, t.bit - _MAN.bits );
			
			return alup_mov_int2int( &_MAN, &_TMP );
		}
	}
	
	return alup__rol_int2int( _NUM, by );
}

int_t alup__ror_int2int( alup_t const * const _NUM, size_t by )
{
	size_t diff = _NUM->bits;
	
	if ( by && diff )
	{	
		alub_t d, w;
		uintmax_t was, tmp;
		alup_t _SEG, _TMP;
		size_t cap, upto = alup_until_pos( _NUM );
		
		_SEG = *_NUM;
		alup_init_unsigned( _TMP, &tmp, bitsof(uintmax_t) );
		
		by %= diff;
		/* We're faking a right rotate by going opposite direction so convert
		 * by to the equivalent value */
		by = diff - by;
		
		d = alub( _NUM->data, _NUM->from );
		
		_SEG.from = upto - by;
		alup_mov_int2int( &_TMP, &_SEG );
		cap = by;
		
		while ( d.bit < upto )
		{
			was = tmp;
			
			_SEG.from = d.bit;
			alup_mov_int2int( &_TMP, &_SEG );
			
			for
			(
				w = alub( &was, 0 )
				; w.bit < cap && d.bit < upto
				; alub_inc(&w), alub_inc(&d)
			)
			{	
				*(d.ptr) &= ~(d.mask);
				*(d.ptr) |= IFTRUE( *(w.ptr) & w.mask, d.mask );
			}
			
			cap = bitsof(uintmax_t);
		}
	}
	
	return 0;
}

int_t alup__ror( alup_t const * const _NUM, void *_tmp, size_t by )
{
	if ( alup_floating( _NUM ) )
	{
		ssize_t exp = alup_get_exponent( _NUM );
		
		if ( exp )
		{
			size_t bias = alup_get_exponent_bias( _NUM ), man;
			alup_t _MAN, _TMP;
			alub_t m, t;
			bool_t neg = alup_below0( _NUM );
			size_t upto = alup_until_pos( _NUM ), tupto;
			
			alup_init_mantissa( _NUM, _MAN );
			man = bias + _MAN.bits;
			exp -= bias;
			
			alup_init_unsigned( _TMP, _tmp, man );
			tupto = alup_until_pos( &_TMP );
			
			alub_set( _MAN.data, alup_until_pos( &_MAN ), 1 );
			_MAN.bits++;
			
			_TMP.from = tupto - _MAN.bits;
			(void)alup_mov_int2int( &_TMP, &_MAN );
			
			_TMP.from = 0;
			(void)alup__ror_int2int( &_TMP, by );
			
			t = alup_final_bit_with_val( &_TMP, true );
			
			/* Set Sign */
			
			m = alub( _NUM->data, upto - 1 );
			*(m.ptr) &= ~(m.mask);
			*(m.ptr) |= IFTRUE( neg && (t.bit || *(t.ptr) & t.mask), m.mask );
			
			/* Set Exponent */
			
			alup_set_exponent
			(
				_NUM
				, t.bit + IFTRUE( t.bit || *(t.ptr) & t.mask, bias )
			);
			
			/* Set Mantissa */
			
			_MAN.bits--;
			_TMP.bits = t.bit - _TMP.from;
			_TMP.from = IFTRUE( _TMP.bits >= _MAN.bits, t.bit - _MAN.bits );
			
			return alup_mov_int2int( &_MAN, &_TMP );
		}
	}
	
	return alup__ror_int2int( _NUM, by );
}

int_t alup_rol( alup_t const * const _NUM, alup_t const * const _VAL )
{
	int_t ret;
	size_t rot;
	alup_t _ROT;

	alup_init_unsigned( _ROT, &rot, bitsof(size_t) );
	
	ret = alup_mov( &_ROT, _VAL );
	if ( ret )
		alu_error(ret);
	
	return alup__rol_int2int( _NUM, rot );
}

int_t alup_ror( alup_t const * const _NUM, alup_t const * const _VAL )
{
	int_t ret;
	size_t rot;
	alup_t _ROT;

	alup_init_unsigned( _ROT, &rot, bitsof(size_t) );
	
	ret = alup_mov( &_ROT, _VAL );
	if ( ret )
		alu_error(ret);
	
	return alup__ror_int2int( _NUM, rot );
}

int_t alup_and( alup_t const * const _NUM, alup_t const * const _VAL )
{	
	if ( !_NUM->data || !_VAL->data )
	{
		int_t ret = EINVAL;
		
		alu_error( ret );
		
		if ( !_NUM->data ) alu_puts( "_NUM->data was NULL!" );
		
		if ( !_VAL->data ) alu_puts( "_VAL->data was NULL!" );
		
		return ret;
	}
	else
	{
		alub_t
			n = alub( _NUM->data, _NUM->from )
			, v = alub( _VAL->data, _VAL->from );
		bool_t neg = alup_below0( _VAL );
		size_t
			mask = UNIC_SIZE_C(~0)
			, stop = _NUM->from + LOWEST( _NUM->bits, _VAL->bits );
		
		for ( ; n.bit < stop; alub_inc(&n), alub_inc(&v) )
		{	
			*(n.ptr) &= EITHER( *(v.ptr) & v.mask, mask, ~(n.mask) );
		}
		
		stop = alup_until_pos( _NUM );
		for ( ; n.bit < stop; alub_inc(&n) )
		{
			*(n.ptr) &= EITHER( neg, mask, ~(n.mask) );
		}
		
		return 0;
	}
}

int_t alup__or( alup_t const * const _NUM, alup_t const * const _VAL )
{
	
	if ( !_NUM->data || !_VAL->data )
	{
		int_t ret = EINVAL;
		
		alu_error( ret );
		
		if ( !_NUM->data ) alu_puts( "_NUM->data was NULL!" );
		
		if ( !_VAL->data ) alu_puts( "_VAL->data was NULL!" );
		
		return ret;
	}
	else
	{
		alub_t
			n = alub( _NUM->data, _NUM->from )
			, v = alub( _VAL->data, _VAL->from );
		bool_t neg = alup_below0( _VAL );
		size_t stop = _NUM->from + LOWEST( _NUM->bits, _VAL->bits );
	
		for ( ; n.bit < stop; alub_inc(&n), alub_inc(&v) )
		{
			*(n.ptr) |= IFTRUE( *(v.ptr) & v.mask, n.mask );
		}
		
		stop = alup_until_pos( _NUM );
		for ( ; n.bit < stop; alub_inc(&n) )
		{
			*(n.ptr) |= IFTRUE( neg, n.mask );
		}
		
		return 0;
	}
}

int_t alup_xor( alup_t const * const _NUM, alup_t const * const _VAL )
{	
	if ( !_NUM->data || !_VAL->data )
	{
		int_t ret = EINVAL;
		
		alu_error(ret);
		
		if ( !_NUM->data ) alu_puts("_NUM->data was NULL!");
		
		if ( !_VAL->data ) alu_puts("_VAL->data was NULL!");
		
		return ret;
	}
	else
	{
		bool_t neg = alup_below0( _VAL );
		alub_t
			n = alub( _NUM->data, _NUM->from ), v = alub( _VAL->data, _VAL->from );
		size_t stop =
			_NUM->from + LOWEST( _NUM->bits, _VAL->bits );
		
		for ( ; n.bit < stop; alub_inc(&n), alub_inc(&v) )
		{
			*(n.ptr) ^= IFTRUE( *(v.ptr) & v.mask, n.mask );
		}
		
		stop = alup_until_pos( _NUM );
		for ( ; n.bit < stop; alub_inc(&n) )
		{
			*(n.ptr) ^= IFTRUE( neg, n.mask );
		}
		
		return 0;
	}
	
	return alu_err_null_ptr("alu");
}

int_t alup__mul_int2int
(
	alup_t const * const _NUM
	, alup_t const * const _VAL
	, void *_cpy
)
{
	int ret;
	alup_t _CPY;
	
	alup_init_unsigned( _CPY, _cpy, _NUM->bits );
	_CPY.sign = _NUM->sign;
	
	ret = alup_mov_int2int( &_CPY, _NUM );
	
	if ( ret == 0 )
	{
		bool carry = false, caught;
		alub_t v;
		size_t p;
		
		alup_set( _NUM, 0 );
		
		v = alub( _VAL->data, _VAL->from );
		
		for ( p = v.bit; v.bit < _VAL->upto; alub_inc(&v) )
		{
			if ( *(v.ptr) & v.mask )
			{	
				(void)alup__shl_int2int( &_CPY, v.bit - p );
				ret = alup__add_int2int( _NUM, &_CPY );
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

int_t alup__mul( alup_t const * const _NUM, alup_t const * const _VAL, void *_cpy, void *_tmp )
{
	if ( alup_floating( _NUM ) || alup_floating( _VAL ) )
	{
		alup_t _DST, _SRC;
		ssize_t exp, dexp, sexp, bias, bits = _NUM->bits;
		bool_t dneg, sneg;
		alub_t final, prior;
		
		/* Ensure dealing with just floating numbers */
		
		alup_init_floating( _DST, _tmp, bits );
		alup_mov( &_DST, _NUM );
		
		alup_init_floating( _SRC, _NUM->data, bits );
		_SRC.upto = _NUM->upto;
		_SRC.from = _NUM->from;
		alup_mov( &_SRC, _VAL );
		
		dneg = alup_below0( &_DST );
		sneg = alup_below0( &_SRC );
		
		dexp = alup_get_exponent( &_DST );
		sexp = alup_get_exponent( &_SRC );
		
		if ( !dexp || !sexp )
		{
			alup_set( _NUM, 0 );
			alup_set_sign( &_DST, dneg != sneg );
			return 0;
		}
		
		bias = alup_get_exponent_bias( &_DST );
		
		dexp -= bias;
		sexp -= bias;
		
		exp = dexp + sexp;
		
		if ( exp >= bias )
		{	
			alu_puts("MUL 1");
			
			(void)alup_set_fltinf( _NUM, dneg != sneg );
			return ERANGE;
		}
		else
		{
			size_t add, mov;
			ssize_t dmov, smov, mdig = _DST.mdig;
			alup_t _DEXP, _SEXP, _DMAN, _SMAN, __DMAN, __SMAN;
			
			alup_init_exponent( &_DST, _DEXP );
			alup_init_exponent( &_SRC, _SEXP );
			
			_DEXP.upto++;
			_SEXP.upto++;
			alup_set( &_DEXP, 0 );
			alup_set( &_SEXP, 0 );
			
			alup_init_mantissa( &_DST, _DMAN );
			alup_init_mantissa( &_SRC, _SMAN );
			
			alup_init_unsigned( __DMAN, _DST.data, bits );
			alup_init_unsigned( __SMAN, _SRC.data, bits );
			__SMAN.upto = _SRC.upto;
			__SMAN.from = _SRC.from;
			
			alub_set( _DST.data, _DEXP.from, 1 );
			alub_set( _SRC.data, _SEXP.from, 1 );
			
			dmov = mdig - (dexp+1);
			dmov = IFTRUE( dmov > 0, dmov );
			dmov = EITHER( dmov < mdig, dmov, mdig );
			smov = mdig - sexp;
			smov = IFTRUE( smov > 0, smov );
			smov = EITHER( smov < mdig, smov, mdig );
			
#if 0
			alu_puts("Before");
			alup_print( &__DMAN, 1, 1 );
			alup_print( &__SMAN, 1, 1 );
#endif
	
			(void)alup__shr_int2int( &__DMAN, dmov );
			(void)alup__shr_int2int( &__SMAN, dmov );
			_SMAN.from = _DEXP.from - smov;
			_SMAN.bits = smov;
			
#if 0
			alu_printf("After moving %zd & %zd bits", dmov, smov );
			alup_print( &__DMAN, 1, 1 );
			alup_print( &__SMAN, 1, 1 );
#endif
			
			prior = alup_final_bit_with_val( &__DMAN, true );
			(void)alup__mul_int2int( &__DMAN, &__SMAN, _cpy );
			
			final = alup_final_bit_with_val( &__DMAN, true );
			add = final.bit - prior.bit;
			
			if ( final.bit >= _DEXP.from )
			{
				//alu_puts("MUL final.bit >= dmuntil_pos");
				
				/* Normalise */
				mov = final.bit - _DEXP.from;
				
				exp = dexp + add;
				alup__shr_int2int( &__DMAN, mov );
			}
			else
			{
				//alu_puts("MUL final.bit < dmuntil_pos");
				
				/* Normalise */
				mov = _DEXP.from - final.bit;
				
				exp = dexp + add;
				alup__shl_int2int( &__DMAN, mov );
			}
			
			alup_set_sign( &_DST, dneg != sneg );
			alup_set_exponent( &_DST, exp + bias );
				
			return alup_mov( _NUM, &_DST );
		}
	}
	
	return alup__mul_int2int( _NUM, _VAL, _cpy );
}

int_t alup__div_int2int( alup_t const * const _NUM, alup_t const * const _VAL, void *_rem )
{	

	int ret = 0;
	alup_t _SEG, _REM;
	alub_t n;
	bool_t nNeg, vNeg;
	size_t bits = 0;
	
	if ( !_NUM->data || !_VAL->data || !_rem )
	{
		ret = EINVAL;
		
		alu_error( ret );
		
		if ( !_NUM->data ) alu_puts("NUM.data was NULL!");
			
		if ( !_VAL->data ) alu_puts("VAL.data was NULL!");
			
		if ( !_rem ) alu_puts("_rem was NULL!");
		
		return ret;
	}
	
	nNeg = alup_below0( _NUM );
	vNeg = alup_below0( _VAL );
	
	if ( nNeg )
		alup_neg( _NUM );
	
	if ( vNeg )
		alup_neg( _VAL );
	
	alup_init_unsigned( _REM, _rem, _NUM->bits );
	
	(void)alup_mov_int2int( &_REM, _NUM );
	(void)alup_set( _NUM, 0 );
	
	_SEG = _REM;
	
	n = alup_final_bit_with_val( &_REM, true );
	_SEG.upto = _SEG.from = n.bit + 1;
	_SEG.last = n.bit;
	_SEG.bits = 0;
	n = alup_first_bit( _NUM );

/* Currently a subtle bug is causing a segfault with the other method, this
 * seems like it would be quicker anyways */
#define ALUP__DIV_USE_REM_FROM
#ifdef ALUP__DIV_USE_REM_FROM
	while ( _SEG.from > _REM.from )
#else
	while ( alup_cmp_int2int( &_REM, _VAL ) >= 0 )
#endif
	{
		++bits;
		_SEG.bits++;
		_SEG.from--;
		if ( alup_cmp_int2int( &_SEG, _VAL ) >= 0 )
		{
			ret = alup__sub_int2int( &_SEG, _VAL );
			
			if ( ret == ENODATA )
				break;
			
			alup__shl_int2int( _NUM, bits );
			*(n.ptr) |= n.mask;
			bits = 0;
		}
	}

#ifdef ALUP__DIV_USE_REM_FROM
	bits += _SEG.from - _REM.from;
	
	if ( bits )
		alup__shl_int2int( _NUM, bits );
#endif
		
	if ( nNeg != vNeg )
		alup_neg( _NUM );
	
	_REM.sign = _NUM->sign;
	if ( nNeg )
		alup_neg( &_REM );
	
	if ( vNeg )
		alup_neg( _VAL );
		
	n = alup_final_bit_with_val( &_REM, true );
	
	return EITHER( ret, ret, IFTRUE( *(n.ptr) & n.mask, ERANGE ) );
}

int_t alup__div( alup_t const * const _NUM, alup_t const * const _VAL, void *_rem, void *_tmp )
{
	if ( alup_floating( _NUM ) || alup_floating( _VAL ) )
	{
		alup_t _DST, _SRC;
		ssize_t exp, dexp, sexp, bias, bits = _NUM->bits;
		bool_t dneg, sneg;
		alub_t final, prior;
		
		/* Ensure dealing with just floating numbers */
		
		alup_init_floating( _DST, _tmp, bits );
		alup_mov( &_DST, _NUM );
		
		alup_init_floating( _SRC, _NUM->data, bits );
		_SRC.upto = _NUM->upto;
		_SRC.from = _NUM->from;
		alup_mov( &_SRC, _VAL );
		
		dneg = alup_below0( &_DST );
		sneg = alup_below0( &_SRC );
		
		dexp = alup_get_exponent( &_DST );
		sexp = alup_get_exponent( &_SRC );
		
		if ( !dexp || !sexp )
		{
			alup_set( _NUM, 0 );
			alup_set_sign( &_DST, dneg != sneg );
			return 0;
		}
		
		bias = alup_get_exponent_bias( &_DST );
		
		dexp -= bias;
		sexp -= bias;
		
		exp = dexp - sexp;
		
		if ( exp >= bias )
		{	
			alu_puts("MUL 1");
			
			(void)alup_set_fltinf( _NUM, dneg != sneg );
			return ERANGE;
		}
		else
		{
			size_t add, mov;
			ssize_t dmov, smov, mdig = _DST.mdig;
			alup_t _DEXP, _SEXP, _DMAN, _SMAN, __DMAN, __SMAN;
			
			alup_init_exponent( &_DST, _DEXP );
			alup_init_exponent( &_SRC, _SEXP );
			
			_DEXP.upto++;
			_SEXP.upto++;
			alup_set( &_DEXP, 0 );
			alup_set( &_SEXP, 0 );
			
			alup_init_mantissa( &_DST, _DMAN );
			alup_init_mantissa( &_SRC, _SMAN );
			
			alup_init_unsigned( __DMAN, _DST.data, bits );
			alup_init_unsigned( __SMAN, _SRC.data, bits );
			__SMAN.upto = _SRC.upto;
			__SMAN.from = _SRC.from;
			
			alub_set( _DST.data, _DEXP.from, 1 );
			alub_set( _SRC.data, _SEXP.from, 1 );
			
			dmov = mdig - (dexp+1);
			dmov = IFTRUE( dmov > 0, dmov );
			dmov = EITHER( dmov < mdig, dmov, mdig );
			smov = mdig - sexp;
			smov = IFTRUE( smov > 0, smov );
			smov = EITHER( smov < mdig, smov, mdig );
			
#if 0
			alu_puts("Before");
			alup_print( &__DMAN, 1, 1 );
			alup_print( &__SMAN, 1, 1 );
#endif
	
			(void)alup__shr_int2int( &__DMAN, dmov );
			(void)alup__shr_int2int( &__SMAN, dmov );
			_SMAN.from = _DEXP.from - smov;
			_SMAN.bits = smov;
			
#if 0
			alu_printf("After moving %zd & %zd bits", dmov, smov );
			alup_print( &__DMAN, 1, 1 );
			alup_print( &__SMAN, 1, 1 );
#endif
			prior = alup_final_bit_with_val( &__DMAN, true );
			(void)alup__div_int2int( &__DMAN, &__SMAN, _rem );
			
			final = alup_final_bit_with_val( &__DMAN, true );
			add = final.bit - prior.bit;
			
			if ( final.bit >= _DEXP.from )
			{
				//alu_puts("MUL final.bit >= dmuntil_pos");
				
				/* Normalise */
				mov = final.bit - _DEXP.from;
				
				exp = dexp + add;
				alup__shr_int2int( &__DMAN, mov );
			}
			else
			{
				//alu_puts("MUL final.bit < dmuntil_pos");
				
				/* Normalise */
				mov = _DEXP.from - final.bit;
				
				exp = dexp + add;
				alup__shl_int2int( &__DMAN, mov );
			}
			
			alup_set_sign( &_DST, dneg != sneg );
			alup_set_exponent( &_DST, exp + bias );
				
			return alup_mov( _NUM, &_DST );
		}
	}
	
	return alup__div_int2int( _NUM, _VAL, _rem );
}
