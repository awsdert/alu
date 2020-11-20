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
	alup_t _EXP;
	
	alup_init_exponent( _PTR, _EXP );

	if ( print_info )
	{	
		alu__printf
		(
			"%s: part = %p, from = %zu, leng = %zu, upto = %zu"
			", signed = %c, floating = %c"
			", exp_dig = %zu, man_dig = %zu"
			, file
			, line
			, func
			, pfx
			, _PTR->data
			, _PTR->from
			, _PTR->leng
			, alup_until_pos( _PTR )
			, '0' + alup___signed( _PTR )
			, '0' + alup_floating( _PTR )
			, _EXP.leng
			, _PTR->mant
		);
	}
	
	if ( print_value )
	{
		alub_t p = alup_final_bit( _PTR );
		size_t bits = 0, sep_at = 4;
		char sec_sep = ' ', dig_sep = '\'';
		
		fprintf( aluout, "%s:%u: %s() %s = ", file, line, func, pfx );
		
		if ( alup___signed( _PTR ) )
		{
			alub_dec(&p);
			bits = 1;
			(void)fputc( '0' + !!(*(p.ptr) & p.mask), aluout );
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
				if ( bits >= sep_at )
				{
					//fputc( dig_sep, aluout );
					bits = 0;
				}
				alub_dec(&p);
				(void)fputc( '0' + !!(*(p.ptr) & p.mask), aluout );
				++bits;
			}
			
			(void)fputc( sec_sep, aluout );
		}
		
		while ( p.bit > _PTR->from )
		{
			if ( bits >= sep_at )
			{
				fputc( dig_sep, aluout );
				bits = 0;
			}
			alub_dec(&p);
			(void)fputc( '0' + !!(*(p.ptr) & p.mask), aluout );
			++bits;
		}
		
		fputc( '\n', aluout );
	}
}

bool_t alup_below0( alup_t const * const _PTR )
{
	if ( _PTR->data && alup___signed(_PTR) )
	{
		alub_t sign = alub( _PTR->data, _PTR->from + (_PTR->leng - 1) );
		return !!(*(sign.ptr) & sign.mask);
	}
	
	return false;
}

alub_t alup_first_one( alup_t const * const _SRC )
{
	alub_t s = {0};
	
	if ( _SRC->data )
	{
		size_t b;
		
		s = alub( _SRC->data, _SRC->from );
		
		do
		{
			b = EITHER( *(s.ptr) & s.mask, _SRC->leng, b + 1 );
			alub_inc(&s);
		}
		while ( b < _SRC->leng );
		
	}
	
	return s;
}

alub_t alup_final_one( alup_t const * const _SRC )
{
	alub_t s = {0};
	
	if ( _SRC->data )
	{
		size_t b;
		
		s = alup_final_bit( _SRC );
		
		do
		{
			alub_dec(&s);
			b = IFTRUE( !( *(s.ptr) & s.mask ), s.bit );
		}
		while ( b > _SRC->from );
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
	bias <<= _EXP.leng - 1;
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
			size_t upto = _DST->from + LOWEST( _DST->leng, _SRC->leng );
			bool neg = alup_below0( _SRC );
			
			for ( ; d.bit < upto; alub_inc(&d), alub_inc(&s) )
			{
				*(d.ptr) &= ~(d.mask);
				*(d.ptr) |= IFTRUE( *(s.ptr) & s.mask, d.mask );
			}

			upto = _DST->from + _DST->leng;
			
			for ( ; d.bit < upto; alub_inc(&d) )
			{
				*(d.ptr) &= ~(d.mask);
				*(d.ptr) |= IFTRUE( neg, d.mask );
			}
			
			return IFTRUE( d.bit < (_DST->from + _SRC->leng), ERANGE );
		}
		
		return alup_set( _DST, 0 );
	}
	
	return alu_err_null_ptr("_DST->data");
}

int_t	alup_mov_int2flt( alup_t const * const _DST, alup_t const * const _SRC )
{
	alub_t s;
	ssize_t exp, bias;
	bool neg = alup_below0( _SRC ), is0;
	
	if ( neg )
	{
		alup_neg( _SRC );
	}
	
	s = alup_final_one( _SRC );
	exp = s.bit - _SRC->from;
	is0 = !(exp || *(s.ptr) & s.mask);
	
	if ( is0 )
	{
		/* Put back the way we received it */
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
		/* Put back the way we received it */
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
		ssize_t upto = LOWEST( exp + 1, (ssize_t)(_DST->mant) );
		
		alup_init_mantissa( _DST, _MAN );
		alup_set( &_MAN, 0 );
		
		//alu_printf( "exp = %zd, upto = %zd", exp, upto );
		
		_REF = *_SRC;
		_REF.leng = upto - 1;
		_REF.from = IFTRUE( s.bit, s.bit - _REF.leng );
		_MAN.from += ( _DST->mant - upto );
		
		//alup_print( &_REF, 1, 1 );
		
		(void)alup_mov_int2int( &_MAN, &_REF );
		
		/* Round to nearest */
		if ( _SRC->from < _REF.from )
		{
			s = alub( _REF.data, _REF.from - 1 );
			
			if ( *(s.ptr) & s.mask )
				alup_inc( &_MAN );
		}
		
		(void)alup_set_exponent( _DST, exp + bias );
		alup_set_sign( _DST, neg );
		
		/* Put back the way we received it */
		if ( neg )
		{
			alup_neg( _SRC );
		}
		
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
	
	if ( Inf || exp >= (ssize_t)(_DST->leng) )
	{
		s = alup_final_one( &_MAN );
		
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
	
	_MAN.from += _MAN.leng - LOWEST( exp, (ssize_t)(_MAN.leng) );
	(void)alup_mov_int2int( _DST, &_MAN );
	ret = alup__shl( _DST, exp );
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
	if ( _DST->mant > _SRC->mant )
	{	
		alup_set( &_DMAN, 0 );
		_DMAN.leng = _SRC->mant;
		_DMAN.from += _DST->mant - _SRC->mant;
	}
	else if ( _SRC->mant > _DST->mant )
	{
		_SMAN.leng = _DST->mant;
		_SMAN.from += _SRC->mant - _DST->mant;
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
	return alup_mov_int2int(  &_DMAN, &_SMAN );
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
		mask_last <<= e.pos;
		mask_last = ~mask_last;
		
		mask = EITHER( e.seg > d.seg, mask_last, mask_last & mask_init );
		
		*(e.ptr) ^= mask;
		*(d.ptr) ^= IFTRUE( e.seg > d.seg, mask_init );
		
		for ( ++(d.seg); d.seg < e.seg; ++(d.seg) )
		{
			++(d.ptr);
			d.ptr[d.seg] = ~(d.ptr[d.seg]);
		}
		
		return 0;
	}
	
	return alu_err_null_ptr("_DST->data");
}

int_t alup_cmp_int2int( alup_t const * const _NUM, alup_t const * const _VAL )
{
	int a = !(_NUM->data), b = !(_VAL->data), r = a - b;
	
	if ( r )
		return r;
		
	a = alup_below0( _NUM );
	b = alup_below0( _VAL );
	r = a - b;
	
	if ( r )
		return r;
	else
	{
		alub_t n = alup_final_bit( _NUM ), v = alup_final_bit( _VAL );
		size_t nlen = n.bit - _NUM->from, vlen = v.bit - _VAL->from;
		
		while ( nlen > vlen )
		{
			--nlen;
			alub_dec(&n);
			
			r = !!(*(n.ptr) & n.mask);
			
			if ( r )
				return r;
		}
		
		while ( vlen > nlen )
		{
			--vlen;
			alub_dec(&v);
			
			r = !!(*(v.ptr) & v.mask);
			
			if ( r )
				return -r;
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
				
				_NMAN.leng++;
				_VMAN.leng++;
				
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
		
		n = alup_final_one( _NUM );
		
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
			
			_VMAN.leng++;
			_VMAN.from += IFTRUE
			(
				vexp >= 0 && vexp <= (ssize_t)(_VMAN.leng)
				,  _VMAN.leng - vexp
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
		ssize_t exp = alup_get_exponent( _SRC )
			, bias = alup_get_exponent_bias( _SRC );
		
		if ( exp )
			exp -= bias;
		
		if ( exp >= 0 && exp < bias )
		{
			alup_t _MAN;
			size_t upto;
			
			alup_init_mantissa( _SRC, _MAN );
			
			upto = alup_until_pos( &_MAN ) - 1;
			_MAN.from = upto - exp;
			
			return alup__inc_int( &_MAN );
		}
		
		return ERANGE;
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
		ssize_t exp = alup_get_exponent( _SRC )
			, bias = alup_get_exponent_bias( _SRC );
		
		if ( exp )
			exp -= bias;
		
		if ( exp >= 0 && exp < bias )
		{
			int_t ret;
			alup_t _MAN;
			alub_t until_bit;
			
			alup_init_mantissa( _SRC, _MAN );

			until_bit = alup_until_bit( &_MAN );
			*(until_bit.ptr) |= until_bit.mask;
			
			_MAN.from = (until_bit.bit - 1) - exp;
			_MAN.leng = exp + 1;
			
			ret = alup__dec_int( &_MAN );
			
			if ( !(*(until_bit.ptr) & until_bit.mask ) )
			{
				alub_t final_one = alup_final_one( &_MAN );
				size_t mov = until_bit.bit - final_one.bit;
				exp -= mov;
				alup__shl( &_MAN, mov );
			}
			
			alup_set_exponent( _SRC, exp + bias );
			return ret;
		}
		
		return ERANGE;
	}
	
	return alup__dec_int( _SRC );
}

int_t alup_match_exponents( void *_num, void *_val, size_t bits )
{
	int_t ret;
	alup_t _NUM, _VAL, _DST, _MAN = {0};
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
		alup_init_mantissa( &_DST, _MAN );
			
		/* Match exponent and align mantissa */
		(void)alup_set_exponent( &_DST, exp );
		
		ret = alup__shr( &_MAN, diff );
		
		truncated = (ret == ERANGE);
	
		/* Insert assumed bit back into position */
		if ( diff < _MAN.leng )
		{
			alub_set( _MAN.data, _MAN.leng - diff, 1 );
		}
	}
	
	return IFTRUE( truncated, ERANGE );
}

int_t alup__add_int2int( alup_t const * const _NUM, alup_t const * const _VAL )
{
	bool_t changed = false, carry = false, active, append;
	alub_t
		n = alub( _NUM->data, _NUM->from )
		, v = alub( _VAL->data, _VAL->from );
	size_t stop = _NUM->from + LOWEST( _NUM->leng, _VAL->leng );
	
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
	if ( alup_floating( _NUM ) || alup_floating( _VAL ) )
	{
		int_t ret;
		alup_t _CPY, _TMP, _CMAN, _TMAN;
		size_t exp, cexp, texp;
		size_t bits = HIGHEST( _NUM->leng, _VAL->leng );
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
				alub_t c = alub( _CMAN.data, _CMAN.leng - 1 );
				
				++exp;
				alup__shr( &_CMAN, 1 );
				
				*(c.ptr) &= ~(c.mask);
				*(c.ptr) |= IFTRUE( both == 2, c.mask );
			}
		}
		
		(void)alup_set_exponent( &_CPY, exp );
		
		ret = alup_mov( _NUM, &_CPY );
		
		return IFTRUE( truncated || ret == ERANGE, ERANGE );
	}
	
	return alup__add_int2int( _NUM, _VAL );
}

int_t alup__sub_int2int( alup_t const * const _NUM, alup_t const * const _VAL )
{
	bool_t changed = false, carry = false, active, append;
	alub_t
		n = alub( _NUM->data, _NUM->from )
		, v = alub( _VAL->data, _VAL->from );
	size_t stop =
		_NUM->from + LOWEST( _NUM->leng, _VAL->leng );
	
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
	if ( alup_floating( _NUM ) || alup_floating( _VAL ) )
	{
		int_t ret;
		alup_t _CPY, _TMP, _CMAN, _TMAN;
		size_t exp, cexp, texp;
		size_t bits = HIGHEST( _NUM->leng, _VAL->leng );
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
		
		ret = alup__sub_int2int( &_CMAN, &_TMAN );
		
		if ( cexp || texp )
		{
			size_t bias = alup_get_exponent_bias( &_CPY );
			
			if ( ret == EOVERFLOW || cexp == texp )
			{	
				//size_t both = 1 + (ret == EOVERFLOW && cexp == texp);
				//alub_t c = alub( _CMAN.data, _CMAN.upto - 1 );
				
				--exp;
				alup__shl( &_CMAN, 1 );
				
				//*(c.ptr) &= ~(c.mask);
				//*(c.ptr) |= IFTRUE( both == 2, c.mask );
			}
			
			if ( cexp == texp )
			{
				alup_set( &_TMP, 0 );
				
				_TMP.sign = 0;
				_TMP.mant = 0;
				
				if ( alup_cmp_int2int( &_CMAN, &_TMP ) == 0 )
				{
					if ( ret != EOVERFLOW )
						exp  = 0;
				}
			}
			
			alub_set
			(
				_CPY.data
				, _CPY.leng - 1
				, (cexp >= bias && cexp < texp) || (!cexp && texp >= bias)
			);
		}
		
		(void)alup_set_exponent( &_CPY, exp );
		
		ret = alup_mov( _NUM, &_CPY );
		
		return IFTRUE( truncated || ret == ERANGE, ERANGE );
	}
	
	return alup__sub_int2int( _NUM, _VAL );
}

int_t alup__shl( alup_t const * const _NUM, size_t by )
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
		
		diff = _NUM->leng;
		
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

int_t alup__shr( alup_t const * const _NUM, size_t by )
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
		alup_t _MAN;
		alub_t sign = alup_final_bit( _SRC );
		bool_t neg = !(*(sign.ptr) & sign.mask);
		
		*(sign.ptr) &= ~(sign.mask);
		*(sign.ptr) |= IFTRUE( neg, sign.mask );
		
		alup_init_mantissa( _SRC, _MAN );
		
		return alup__neg_int( &_MAN );
	}
	
	return alup__neg_int( _SRC );
}

int_t alup__rol_int2int( alup_t const * const _NUM, size_t by )
{
	size_t diff = _NUM->leng;
	
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
			man = bias + _MAN.leng;
			exp -= bias;
			
			alup_init_unsigned( _TMP, _tmp, man );
			tupto = alup_until_pos( &_TMP );
			
			alub_set( _MAN.data, alup_until_pos( &_MAN ), 1 );
			_MAN.leng++;
			
			_TMP.from = tupto - _MAN.leng;
			(void)alup_mov_int2int( &_TMP, &_MAN );
			
			_TMP.from = 0;
			(void)alup__rol_int2int( &_TMP, by );
			
			t = alup_final_one( &_TMP );
			
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
			
			_MAN.leng--;
			_TMP.leng = t.bit - _TMP.from;
			_TMP.from = IFTRUE( _TMP.leng >= _MAN.leng, t.bit - _MAN.leng );
			
			return alup_mov_int2int( &_MAN, &_TMP );
		}
	}
	
	return alup__rol_int2int( _NUM, by );
}

int_t alup__ror_int2int( alup_t const * const _NUM, size_t by )
{
	size_t diff = _NUM->leng;
	
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
			man = bias + _MAN.leng;
			exp -= bias;
			
			alup_init_unsigned( _TMP, _tmp, man );
			tupto = alup_until_pos( &_TMP );
			
			alub_set( _MAN.data, alup_until_pos( &_MAN ), 1 );
			_MAN.leng++;
			
			_TMP.from = tupto - _MAN.leng;
			(void)alup_mov_int2int( &_TMP, &_MAN );
			
			_TMP.from = 0;
			(void)alup__ror_int2int( &_TMP, by );
			
			t = alup_final_one( &_TMP );
			
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
			
			_MAN.leng--;
			_TMP.leng = t.bit - _TMP.from;
			_TMP.from = IFTRUE( _TMP.leng >= _MAN.leng, t.bit - _MAN.leng );
			
			return alup_mov_int2int( &_MAN, &_TMP );
		}
	}
	
	return alup__ror_int2int( _NUM, by );
}

int_t alup_rol( alup_t const * const _NUM, alup_t const * const _VAL, void *_tmp )
{
	size_t rot, len = _NUM->leng;
	alup_t _LEN, _TMP, _ROT;
	
	alup_init_unsigned( _LEN, &len, bitsof(size_t) );
	alup_init_unsigned( _ROT, &rot, bitsof(size_t) );
	alup_init_unsigned( _TMP, _tmp, _NUM->leng );
	
	/* Retrieve only an amount that fits into size_t,
	 * also ensure within bounds of nbits */
	alup__div_int2int( _VAL, &_LEN, _tmp );
	alup_mov_int2int( &_ROT, &_TMP );
	
	/* Reconstruct _VAL to what it was,
	 * supposed to be constant - we fake it here */
	alup__mul_int2int( _VAL, &_LEN, _tmp );
	alup__add_int2int( _VAL, &_ROT );
	
	return alup__rol_int2int( _NUM, rot );
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
			, stop = _NUM->from + LOWEST( _NUM->leng, _VAL->leng );
		
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
		size_t stop = _NUM->from + LOWEST( _NUM->leng, _VAL->leng );
	
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
			_NUM->from + LOWEST( _NUM->leng, _VAL->leng );
		
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
	
	alup_init_unsigned( _CPY, _cpy, _NUM->leng );
	_CPY.sign = _NUM->sign;
	
	ret = alup_mov_int2int( &_CPY, _NUM );
	
	if ( ret == 0 )
	{
		bool carry = false, caught;
		alub_t v;
		size_t p, upto = alup_until_pos( _VAL );
		
		alup_set( _NUM, 0 );
		
		v = alub( _VAL->data, _VAL->from );
		
		for ( p = v.bit; v.bit < upto; alub_inc(&v) )
		{
			if ( *(v.ptr) & v.mask )
			{	
				(void)alup__shl( &_CPY, v.bit - p );
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
		int_t ret;
		alup_t _DST, _SRC, _DMAN, _SMAN;
		ssize_t exp, dexp, sexp, dbias, sbias, bits;
		bool_t dneg, sneg;
		ssize_t ddiff = 0, sdiff = 0;
		size_t dmupto, smupto;
		alub_t final, prior;
		
		/* Ensure dealing with just floats, impossible for both to take _tmp
		 * given the above if statment */
		
		if ( alup_floating( _NUM ) )
		{
			_DST = *_NUM;
		}
		else
		{
			bits = _NUM->leng;
			alup_init_floating( _DST, _tmp, bits );
			alup_mov( &_DST, _NUM );
		}
		
		if ( alup_floating( _VAL ) )
		{
			_SRC = *_VAL;
		}
		else
		{
			bits = _VAL->leng;
			alup_init_floating( _SRC, _tmp, bits );
			alup_mov( &_SRC, _VAL );
		}
		
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
		
		dbias = alup_get_exponent_bias( &_DST );
		sbias = alup_get_exponent_bias( &_DST );
		
		dexp -= dbias;
		sexp -= sbias;
		
		exp = dexp + sexp;
				
		alup_init_mantissa( &_DST, _DMAN );
		alup_init_mantissa( &_SRC, _SMAN );
		
		ddiff = EITHER
		(
			dexp >= 0 && dexp < (ssize_t)(_DMAN.leng)
			, dexp
			, _DMAN.leng
		);
		sdiff = EITHER
		(
			sexp >= 0 && sexp < (ssize_t)(_SMAN.leng)
			, sexp
			, _SMAN.leng
		);
		
		alup_set_exponent( &_DST, 0 );
		alup_set_sign( &_DST, 0 );
		
		dmupto = alup_until_pos( &_DMAN );
		smupto = alup_until_pos( &_SMAN );
		
		alub_set( _DST.data, dmupto, 1 );
		alub_set( _SRC.data, smupto, 1 );
		
		_SMAN.from = smupto - sdiff;
		_DMAN.leng = _DST.leng;
		_SMAN.leng++;
		
		alup__shr( &_DMAN, _DST.mant - ddiff );
		prior = alup_final_one( &_DMAN );
		
		ret = alup__mul_int2int( &_DMAN, &_SMAN, _cpy );
		
		/* We mangled this so restore it now */
		alup_set_exponent( &_SRC, sexp + sbias );
		
		if ( exp >= dbias )
		{	
			alu_puts("MUL 1");
			
			(void)alup_set_fltinf( _NUM, dneg != sneg );
			return ERANGE;
		}
		else
		{
			size_t add, mov;
			
			final = alup_final_one( &_DMAN );
			add = final.bit - prior.bit;
			
			if ( final.bit < dmupto - 1 )
			{
				/* Normalise */
				mov = dmupto - final.bit;
				
				exp = dexp + add;
				alup__shl( &_DMAN, mov );
			}
			else if ( final.bit >= dmupto )
			{
				/* Normalise */
				mov = final.bit - dmupto;
				
				exp = dexp + add;
				alup__shr( &_DMAN, mov );
			}
			else if ( ret == EOVERFLOW )
			{	
				exp++;
				alup__shr( &_DMAN, 1 );
				alub_set( _DMAN.data, dmupto - 1, 1 );
			}
			
			alup_set_exponent( &_DST, exp + dbias );
		}
		
		alup_set_sign( &_DST, dneg != sneg );
		
		if ( alup_floating( _NUM ) )
			return 0;
			
		return alup_mov( _NUM, &_DST );
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
	
	alup_init_unsigned( _REM, _rem, _NUM->leng );
	
	(void)alup_mov_int2int( &_REM, _NUM );
	(void)alup_set( _NUM, 0 );
	
	_SEG = _REM;
	
	n = alup_final_one( &_REM );
	_SEG.from = n.bit + 1;
	_SEG.leng = 0;
	n = alub( _NUM->data, _NUM->from );
	
#if 0
	while ( _SEG.from > _REM.from )
#else
	while ( alup_cmp_int2int( &_REM, _VAL ) >= 0 )
#endif
	{
		++bits;
		_SEG.leng++;
		_SEG.from--;
		if ( alup_cmp_int2int( &_SEG, _VAL ) >= 0 )
		{
			ret = alup__sub_int2int( &_SEG, _VAL );
			
			if ( ret == ENODATA )
				break;
			
			alup__shl( _NUM, bits );
			*(n.ptr) |= n.mask;
			bits = 0;
		}
	}
	
	bits += _SEG.from - _REM.from;
	
	if ( bits )
		alup__shl( _NUM, bits );
		
	if ( nNeg != vNeg )
		alup_neg( _NUM );
	
	_REM.sign = _NUM->sign;
	if ( nNeg )
		alup_neg( &_REM );
	
	if ( vNeg )
		alup_neg( _VAL );
		
	n = alup_final_one( &_REM );
	
	return EITHER( ret, ret, IFTRUE( *(n.ptr) & n.mask, ERANGE ) );
}

int_t alup__div( alup_t const * const _NUM, alup_t const * const _VAL, void *_rem, void *_tmp )
{
	if ( alup_floating( _NUM ) || alup_floating( _VAL ) )
	{
		alup_t _DST, _SRC, _DMAN, _SMAN;
		size_t exp_len, dmupto, smupto;
		ssize_t exp, dexp, sexp, dbias, sbias, dbits, sbits, bits, size;
		bool_t dneg, sneg;
		alub_t prior;
		
		/* Ensure dealing with just floats, impossible for both to take _tmp
		 * given the above if statment */
		 
		bits = _NUM->leng * 2;
		size = BITS2SIZE( bits );
		alup_init_floating( _DST, _tmp, size );
		alup_mov( &_DST, _NUM );

		size /= 2;
		alup_init_floating( _SRC, _NUM->data, _NUM->leng );
		alup_mov( &_SRC, _VAL );
		
		dneg = alup_below0( &_DST );
		sneg = alup_below0( &_SRC );
		
		dexp = alup_get_exponent( &_DST );
		sexp = alup_get_exponent( &_SRC );
		
		if ( !dexp || !sexp )
		{
			alup_set( _NUM, 0 );
			alup_set_sign( _NUM, dneg != sneg );
			return 0;
		}
		
		dbias = alup_get_exponent_bias( &_DST );
		sbias = alup_get_exponent_bias( &_SRC );
		
		dexp -= dbias;
		sexp -= sbias;
				
		alup_init_mantissa( &_DST, _DMAN );
		alup_init_mantissa( &_SRC, _SMAN );
		
		dmupto = alup_until_pos( &_DMAN );
		smupto = alup_until_pos( &_SMAN );
		
		dbits = _DMAN.leng;
		sbits = _SMAN.leng;
		bits = LOWEST( dbits, sbits );
		
		exp_len = (_DST.leng - _DST.mant) - 1;
		_SMAN.from = smupto - EITHER( sexp >= 0 && sexp < sbits, sexp, sbits );
		
		alub_set( _DST.data, dmupto, 1 );
		alub_set( _SRC.data, smupto, 1 );
		
		_DMAN.leng = _DST.leng;
		_SMAN.leng++;
		
		alup__shl( &_DMAN, exp_len );
		prior = alup_final_one( &_DMAN );
		(void)alup__div_int2int( &_DMAN, &_SMAN, _rem );
		
		exp = dexp + sexp;
		
		if ( exp > dbias )
		{	
			alu_puts("Route 1");
			/* Set infinity */
			return alup_set_inf( _NUM, dneg != sneg );
		}
		else if ( exp <= (ssize_t)(_DMAN.leng) )
		{
			/* Normalise */
			alub_t first = alup_first_one( &_DMAN ), final = alup_final_one( &_DMAN );
			size_t rel2prv = prior.bit - final.bit, mov;
			bool_t round = (first.bit - _DST.from) < exp_len;
			
			exp = dexp - rel2prv;
			
			if ( final.bit > dmupto )
			{
				bool_t up;
				mov = final.bit - dmupto;
				
				//alu_printf( "Route 2.1, mov = %zu", mov );
				
				//alup_print( _DST, 0, 1 );
				alup__shr( &_DMAN, mov - 1 );
				up = alub_get( _DST.data, _DST.from );
				alup__shr( &_DMAN, 1 );
				if ( round || up )
					alup_inc( &_DMAN );
				//alup_print( _DST, 0, 1 );
			}
			else if ( final.bit < dmupto )
			{
				mov = dmupto - final.bit;
				
				alu_puts("Route 2.2");
				
				alup__shl( &_DMAN, mov + 1 );
				//exp = dexp - rel2prv;
				
				if ( round )
				{
					alup_inc( &_DMAN );
				}
				
				alup__shr( &_DMAN, 1 );
			}
			
			alup_set_exponent( &_DST, exp + dbias );
		}	
		else
		{
			//alu_puts("Route 3");
			alup__shr( &_DMAN, (exp_len - 1) );
			alup_set_exponent( &_DST, exp + dbias );
		}
		
		alub_set( _DST.data, (_DST.from + _DST.leng) - 1, dneg != sneg );
			
		return alup_mov( _NUM, &_DST );
	}
	
	return alup__div_int2int( _NUM, _VAL, _rem );
}
