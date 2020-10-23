#include <alu/alup.h>

bool_t alup_below0( alup_t _PTR )
{
	if ( _PTR.data && alup___signed(_PTR) )
	{
		alub_t sign = alub( _PTR.data, _PTR.upto - 1 );
		return !!(*(sign.ptr) & sign.mask);
	}
	
	return false;
}

alub_t alup_end_bit( alup_t _SRC )
{
	alub_t s = {0};
	
	if ( _SRC.data )
	{
		size_t b;
		
		s = alub( _SRC.data, _SRC.upto );
		
		do
		{
			alub_dec(&s);
			b = IFTRUE( !( *(s.ptr) & s.mask ), s.bit );
		}
		while ( b > _SRC.from );
		
	}
	
	return s;
}

size_t alup_get_exponent( alup_t _SRC )
{
	size_t dst = 0;
	alup_t _EXP, _DST;
		
	alup_init_unsigned( _DST, &dst, sizeof(size_t) );
	alup_init_exponent( _SRC, _EXP );
	
	(void)alup_mov_int2int( _DST, _EXP );
	
	return dst;
}

int_t alup_set_exponent( alup_t _DST, size_t src )
{
	alup_t _EXP, _SRC;
	
	alup_init_unsigned( _SRC, &src, sizeof(size_t) );
	alup_init_exponent( _DST, _EXP );
	
	return alup_mov_int2int( _EXP, _SRC );
}

size_t alup_get_exponent_bias( alup_t _SRC )
{
	alup_t _EXP;
	size_t bias = UNIC_SIZE_C(~0);
	alup_init_exponent( _SRC, _EXP );
	bias <<= (_EXP.upto - _EXP.from) - 1;
	return ~bias;
}

int_t	alup_set( alup_t _DST, bool fillwith )
{
	if ( _DST.data )
	{
		alub_t n;
		
		/* Force to either 1 or 0 */
		fillwith = !!fillwith;
		
		for ( n = alub( _DST.data, _DST.from ); n.bit < _DST.upto; alub_inc(&n) )
		{
			*(n.ptr) &= ~(n.mask);
			*(n.ptr) |= IFTRUE( fillwith, n.mask );
		}
		
		return 0;
	}
	
	return alu_err_null_ptr("_DST.data");
}

int_t	alup_set_max( alup_t _DST )
{
	int_t ret = alup_set( _DST, 1 );
	
	if ( ret == 0 )
	{
		if ( alup_floating( _DST ) )
		{
			alup_t _MAN;
			
			_DST.info |= ALU_INFO__SIGN;
			
			alup_init_mantissa( _DST, _MAN );
			alup_set( _MAN, 0 );
		}
		
		if ( alup___signed( _DST ) )
		{
			alub_t n = alub( _DST.data, _DST.upto - 1 );
			*(n.ptr) &= ~(n.mask);
		}
		
		return 0;
	}
	
	alu_error(ret);
	return ret;
}

int_t	alup_set_min( alup_t _DST )
{
	int_t ret = alup_set( _DST, 0 );
	
	if ( ret == 0 )
	{
		if ( alup_floating( _DST ) )
		{
			alup_t _EXP;
			
			_DST.info |= ALU_INFO__SIGN;
			
			alup_init_exponent( _DST, _EXP );
			alup_set( _EXP, 1 );
		}
			
		if ( alup___signed( _DST ) )
		{
			alub_t n = alub( _DST.data, _DST.upto - 1 );
			*(n.ptr) |= n.mask;	
		}
		
		return 0;
	}
	
	alu_error(ret);
	return ret;
}

int_t	alup_mov_int2int( alup_t _DST, alup_t _SRC )
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

int_t	alup_mov_int2flt( alup_t _DST, alup_t _SRC )
{	
	alup_t _MAN, _NIL = {0};
	alub_t d, s;
	size_t exp, bias, b;
	bool neg = alup_below0( _SRC );
	
	/* Set +/- */
	d = alub( _DST.data, _DST.upto - 1 );
	*(d.ptr) &= ~(d.mask);
	*(d.ptr) |= IFTRUE( neg, d.mask );
	
	if ( neg )
	{
		alup_neg( _SRC );
	}
	
	s = alup_end_bit( _SRC );
	b = s.bit - _SRC.from;
	
	/* +0 should look like an integer 0 */
	if ( !b && !(*(s.ptr) & s.mask) )
	{
		/* Put back the way we received it */
		if ( neg )
		{
			alup_neg( _SRC );
		}
		
		return alup_mov_int2int( _DST, _NIL );
	}
	
	alup_init_mantissa( _DST, _MAN );
	
	bias = alup_get_exponent_bias( _DST );
	
	if ( b >= bias )
	{
		/* Put back the way we received it */
		if ( neg )
		{
			alup_neg( _SRC );
		}
		
		/* Set Infinity */
		exp = (bias << 1) | 1;
		(void)alup_set_exponent( _DST, exp );
		/* Clear mantissa so not treated as NaN */
		(void)alup_mov_int2int( _MAN, _NIL );
		return EOVERFLOW;
	}
	
	exp = (bias + b);
	(void)alup_set_exponent( _DST, exp );
	
	_SRC.upto = s.bit;
	_SRC.from = _SRC.upto - LOWEST( b, _MAN.upto - _MAN.from );
	_MAN.from = _MAN.upto - LOWEST( b, _MAN.upto - _MAN.from );
	
	(void)alup_mov_int2int( _MAN, _SRC );
	
	/* Round to nearest */
	if ( _SRC.from > s.bit - b )
	{
		s = alub( _SRC.data, (_SRC.upto - b) - 1 );
		d = alub( _MAN.data, _MAN.from );
		
		*(d.ptr) |= IFTRUE( *(s.ptr) & s.mask, d.mask );
	}
	
	/* Put back the way we received it */
	if ( neg )
	{
		alup_neg( _SRC );
	}
	
	return 0;
}

int_t	alup_mov_flt2int( alup_t _DST, alup_t _SRC )
{
	alup_t _MAN;
	alub_t d, s;
	size_t dlength, slength, sman_dig
		, exp = alup_get_exponent( _SRC )
		, bias = alup_get_exponent_bias(_SRC);
	bool Inf, neg;
	
	dlength = _DST.upto - _DST.from;
	slength = _SRC.upto - _SRC.from;
	
	sman_dig = alu_man_dig(slength);
	
	alup_init_mantissa( _SRC, _MAN );
	
	neg = alup_below0( _SRC );
	Inf = (exp == bias);
		
	/* Set +/- */
	d = alub( _DST.data, _DST.upto - 1 );
	*(d.ptr) &= ~(d.mask);
	*(d.ptr) |= (neg * d.mask);
	
	if ( exp <= bias )
	{
		alup_set( _DST, 0 );
		return 0;
	}
	
	exp -= bias;
	
	if ( Inf || exp >= dlength )
	{
		s = alup_end_bit( _MAN );
		
		/* NaN cannot be recorded by an integer, use 0 instead */
		if ( Inf && *(s.ptr) & s.mask )
		{
			alup_set( _DST, 0 );
			return 0;
		}
		
		/* Infinity cannot be recorded by an integer,
		 * use min/max instead */
		if ( neg )
			alup_set_min( _DST );
		else
			alup_set_max( _DST );
			
		return 0;
	}
	
	(void)alup_set( _DST, 0 );
	
	/* alur_get_raw() will have retrieved and released a register,
	 * since that succeeded will end up with that register here as the
	 * register was not released from memory, only usage */
	
	/* Put in the assumed bit and move it to correct location */
	d = alub( _DST.data, _DST.from );
	*(d.ptr) |= d.mask;
	(void)alup__shl( _DST, sman_dig );
	
	/* Put in rest of mantissa */
	(void)alup__or( _DST, _MAN );
	
	/* Align to correct position now that full integer is there */
	if ( exp <= sman_dig )
		return alup__shr( _DST, sman_dig - exp );
	
	(void)alup__shl( _DST, exp );
	return 0;
}

int_t	alup_mov_flt2flt( alup_t _DST, alup_t _SRC )
{
	alup_t _DMAN, _SMAN;
	alub_t d;
	size_t dlen, slen, exp, dbias, sbias, inf;
	bool neg;
	
	exp = alup_get_exponent( _SRC );
	
	alup_init_mantissa( _DST, _DMAN );
	alup_init_mantissa( _SRC, _SMAN );
	
	dlen = _DMAN.upto - _DMAN.from;
	slen = _SMAN.upto - _SMAN.from;
	
	/* Set +/- */
	neg = alup_below0( _SRC );
	d = alub( _DST.data, _DST.upto -1 );
	*(d.ptr) &= ~(d.mask);
	*(d.ptr) |= IFTRUE( neg, d.mask );
	
	/* Make sure we refer to upper bits of both mantissa's if they're of
	 * different lengths */
	if ( dlen > slen )
	{
		_DMAN.from = _DMAN.upto - slen;
		
		/* Clear out useless bits, we'll worry about recuring digits once
		 * we have working arithmetic */
		d = alub( _DST.data, _DMAN.from );
		while ( d.bit > _DST.from )
		{
			alub_dec(&d);
			*(d.ptr) &= ~(d.mask);
		}
	}
	else if ( slen > dlen )
	{
		_SMAN.from = _SMAN.upto - dlen;
	}
	
	dbias = alup_get_exponent_bias( _DST );
	sbias = alup_get_exponent_bias( _SRC );
	
	inf = (sbias << 1) | 1;
	
	if ( exp >= inf )
	{
		inf = (dbias << 1) | 1;
		inf = IFTRUE( exp >= inf, inf );
		(void)alup_set_exponent( _DST, inf );
		
		/* If SRC was +/-NaN then this will set that */
		return alup_mov_int2int( _DMAN, _SMAN );
	}
	
	exp -= sbias;
	exp += dbias;
	inf = (dbias << 1) | 1;
	
	if ( exp >= inf )
	{
		alup_t _NIL = {0};
		/* Beyond what DST can handle, default to +/-Infinity */
		(void)alup_set_exponent( _DST, inf );
		return alup_mov_int2int( _DMAN, _NIL );
	}
	
	(void)alup_set_exponent( _DST, exp );
	return alup_mov_int2int(  _DMAN, _SMAN );
}

int_t	alup_mov( alup_t _DST, alup_t _SRC )
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

int_t alup_not( alup_t _NUM )
{
	if ( _NUM.data )
	{
		alub_t n, e;
		size_t i, stop, mask, mask_init, mask_last;
		
		n = alub( _NUM.data, _NUM.from );
		e = alub( _NUM.data, _NUM.upto - 1 );
		
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
	
	return alu_err_null_ptr("_NUM.data");
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


int_t alup_inc( alup_t _NUM )
{
	alub_t n;
	bool_t is1 = true;
	
	for
	(
		n = alub( _NUM.data, _NUM.from )
		; is1 && n.bit < _NUM.upto
		; alub_inc(&n)
	)
	{
		is1 = !!(*(n.ptr) & n.mask);
		*(n.ptr) &= ~(n.mask);
		*(n.ptr) |= IFTRUE( !is1, n.mask );
	}
	
	return EITHER( is1, EOVERFLOW, 0 );
}

int_t alup_match_exponents( void *_num, void *_val, size_t size )
{
	int_t ret;
	alup_t _NUM, _VAL, _DST, _MAN = {0};
	size_t exp = 0, nexp, vexp, diff = 0;
	bool truncated = false;
	
	alup_init_floating( _NUM, _num, size );
	alup_init_floating( _VAL, _val, size );
	
	nexp = alup_get_exponent( _NUM );
	vexp = alup_get_exponent( _VAL );
		
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
		alup_init_mantissa( _DST, _MAN );
			
		/* Match exponent and align mantissa */
		(void)alup_set_exponent( _DST, exp );
		
		ret = alup__shr( _MAN, diff );
		
		truncated = (ret == ERANGE);
	
		/* Insert assumed bit back into position */
		if ( diff < _MAN.upto )
		{
			alub_t m = alub( _MAN.data, _MAN.upto - diff );
			*(m.ptr) |= m.mask;
		}
	}
	
	return IFTRUE( truncated, ERANGE );
}

int_t alup__add_int2int( alup_t _NUM, alup_t _VAL )
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

int_t alup__add( alup_t _NUM, alup_t _VAL, void *_cpy, void *_tmp )
{
	int ret;
	
	if ( alup_floating( _NUM ) || alup_floating( _VAL ) )
	{
		alup_t _CPY, _TMP, _CMAN, _TMAN;
		size_t exp, cexp, texp;
		size_t
			bits = HIGHEST( _NUM.upto - _NUM.from, _VAL.upto - _VAL.from )
			, size = (bits / UNIC_CHAR_BIT) + !!(bits % UNIC_CHAR_BIT);
		bool_t truncated = false;
		
		alup_init_floating( _CPY, _cpy, size );
		alup_init_floating( _TMP, _tmp, size );
		
		alup_init_mantissa( _CPY, _CMAN );
		alup_init_mantissa( _TMP, _TMAN );
		
		alup_mov( _CPY, _NUM );
		alup_mov( _TMP, _VAL );
		
		cexp = alup_get_exponent( _CPY );
		texp = alup_get_exponent( _TMP );
		
		ret = alup_match_exponents( _CPY.data, _TMP.data, size );
		
		truncated = (ret == ERANGE);
		
		exp = alup_get_exponent( _CPY );
		
		ret = alup__add_int2int( _CMAN, _TMAN );
		
		if ( cexp || texp )
		{
			if ( cexp == texp || ret == EOVERFLOW ) ++exp;
		}
		
		(void)alup_set_exponent( _CPY, exp );
		
		ret = alup_mov( _NUM, _CPY );
		
		return IFTRUE( truncated || ret == ERANGE, ERANGE );
	}
	
	return alup__add_int2int( _NUM, _VAL );
}

int_t alup__sub_int2int( alup_t _NUM, alup_t _VAL )
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

int_t alup__sub( alup_t _NUM, alup_t _VAL, void *_cpy, void *_tmp )
{
	int ret;
	
	if ( alup_floating( _NUM ) || alup_floating( _VAL ) )
	{
		alup_t _CPY, _TMP, _CMAN, _TMAN;
		size_t exp, cexp, texp;
		size_t
			bits = HIGHEST( _NUM.upto - _NUM.from, _VAL.upto - _VAL.from )
			, size = (bits / UNIC_CHAR_BIT) + !!(bits % UNIC_CHAR_BIT);
		bool_t truncated = false;
		
		alup_init_floating( _CPY, _cpy, size );
		alup_init_floating( _TMP, _tmp, size );
		
		alup_init_mantissa( _CPY, _CMAN );
		alup_init_mantissa( _TMP, _TMAN );
		
		alup_mov( _CPY, _NUM );
		alup_mov( _TMP, _VAL );
		
		cexp = alup_get_exponent( _CPY );
		texp = alup_get_exponent( _TMP );
		
		ret = alup_match_exponents( _CPY.data, _TMP.data, size );
		
		truncated = (ret == ERANGE);
		
		exp = alup_get_exponent( _CPY );
		
		ret = alup__sub_int2int( _CMAN, _TMAN );
		
		if ( cexp || texp )
		{
			if ( ret == EOVERFLOW && cexp > texp ) --exp;
		}
		
		(void)alup_set_exponent( _CPY, exp );
		
		ret = alup_mov( _NUM, _CPY );
		
		return IFTRUE( truncated || ret == ERANGE, ERANGE );
	}
	
	return alup__sub_int2int( _NUM, _VAL );
}

int_t alup__shl( alup_t _NUM, size_t by )
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
			alup_init_unsigned( _TMP, &by, sizeof(size_t) );
			
			return alup__add_int2int( _EXP, _TMP );
		}
		
		diff = _NUM.upto - _NUM.from;
		
		if ( by >= diff )
			return alup_set( _NUM, 0 );
		else
		{
			alub_t d = alub( _NUM.data, _NUM.upto ), s = d;
			
			while ( by )
			{
				--by;
				alub_dec(&s);
				
				ret = IFTRUE( ret || *(s.ptr) & s.mask, ERANGE );
			}
			
			while ( s.bit > _NUM.from )
			{
				alub_dec(&s);
				alub_dec(&d);
				
				*(d.ptr) &= ~(d.mask);
				*(d.ptr) |= IFTRUE( *(s.ptr) & s.mask, d.mask );
			}
			
			while ( d.bit > _NUM.from )
			{
				alub_dec(&d);
				
				*(d.ptr) &= ~(d.mask);
			}
		}
	}
	
	return ret;
}

int_t alup__shr( alup_t _NUM, size_t by )
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
			alup_init_unsigned( _TMP, &by, sizeof(size_t) );
			
			return alup__sub_int2int( _EXP, _TMP );
		}
		else
		{
			bool_t neg = alup_below0( _NUM );
			alub_t d = alub( _NUM.data, _NUM.from ), s = d;
			
			while ( by )
			{
				--by;
				
				ret = IFTRUE( ret || *(s.ptr) & s.mask, ERANGE );
				
				alub_inc(&s);
			}		
			while ( s.bit < _NUM.upto )
			{
				*(d.ptr) &= ~(d.mask);
				*(d.ptr) |= IFTRUE( *(s.ptr) & s.mask, d.mask );
				
				alub_inc(&s);
				alub_inc(&d);
			}
			
			while ( d.bit < _NUM.upto )
			{	
				*(d.ptr) &= ~(d.mask);
				*(d.ptr) |= IFTRUE( neg, d.mask );
				
				alub_inc(&d);
			}
		}
	}
	
	return ret;
}

int_t alup_neg( alup_t _NUM )
{
	int ret = alup_not( _NUM );
	
	if ( ret == 0 )
		return alup_inc( _NUM );
		
	alu_error( ret );
	return ret;
}

int_t alup__rol( alup_t _NUM, void *_tmp, size_t by )
{	
	alup_t _TMP;
	size_t diff = _NUM.upto - _NUM.from;
	
	by %= diff;
	
	if ( alup_floating( _NUM ) )
	{
		alup_t _EXP, _MAN;
		
		alup_init_exponent( _NUM, _EXP );
		alup_init_mantissa( _NUM, _MAN );
		alup_init_unsigned( _TMP, &by, sizeof(size_t) );
		/* FIXME: Haven't accounted for the assumed bit */
		(void)alup__rol( _MAN, _tmp, by );
		return alup__sub_int2int( _EXP, _TMP );
	}
		
	if ( by )
	{	
		alub_t n, v;
		size_t size = (diff / UNIC_CHAR_BIT) + !!(diff % UNIC_CHAR_BIT);
		
		alup_init_unsigned( _TMP, _tmp, size );
		
		alup_mov_int2int( _TMP, _NUM );
		
		n = alub( _NUM.data, _NUM.upto );
		v = alub( _tmp, diff - by );
		
		while ( v.bit )
		{
			alub_dec(&v);
			alub_dec(&n);
			
			*(n.ptr) &= ~(n.mask);
			*(n.ptr) |= (n.mask * !!(*(v.ptr) & v.mask));
		}
		
		v = alub( _tmp, diff );
		while ( n.bit > _NUM.from )
		{
			alub_dec(&v);
			alub_dec(&n);
			
			*(n.ptr) &= ~(n.mask);
			*(n.ptr) |= (n.mask * !!(*(v.ptr) & v.mask));
		}
	}
	
	return 0;
}

int_t alup__ror( alup_t _NUM, void *_tmp, size_t by )
{
	if ( _NUM.data && _tmp )
	{
		size_t diff = _NUM.upto - _NUM.from;
		
		by %= diff;
		
		if ( by )
		{
			alup_t _TMP;
			alub_t n, v;
			size_t size = (diff / UNIC_CHAR_BIT) + !!(diff % UNIC_CHAR_BIT);
			
			alup_init_unsigned( _TMP, _tmp, size );
			
			(void)alup_mov_int2int( _TMP, _NUM );
			
			n = alub( _NUM.data, _NUM.from );
			v = alub( _tmp, by );
			
			while ( v.bit < _TMP.upto )
			{	
				*(n.ptr) &= ~(n.mask);
				*(n.ptr) |= IFTRUE( *(v.ptr) & v.mask, n.mask );

				alub_inc(&v);
				alub_inc(&n);
			}
			
			v = alub( _tmp, 0 );
			while ( n.bit < _NUM.upto )
			{
				*(n.ptr) &= ~(n.mask);
				*(n.ptr) |= IFTRUE( *(v.ptr) & v.mask, n.mask );
				
				alub_inc(&v);
				alub_inc(&n);
			}
		}
		
		return 0;
	}
	
	if ( !_NUM.data )
		return alu_err_null_ptr("_NUM.data");
	
	return alu_err_null_ptr("_tmp");
}

int_t alup_and( alup_t _NUM, alup_t _VAL )
{	
	if ( !_NUM.data || !_VAL.data )
	{
		int_t ret = EINVAL;
		
		alu_error( ret );
		
		if ( !_NUM.data ) alu_puts( "_NUM.data was NULL!" );
		
		if ( !_VAL.data ) alu_puts( "_VAL.data was NULL!" );
		
		return ret;
	}
	else
	{
		alub_t
			n = alub( _NUM.data, _NUM.from )
			, v = alub( _VAL.data, _VAL.from );
		bool_t neg = alup_below0( _VAL );
		size_t
			mask = UNIC_SIZE_C(~0)
			, stop = _NUM.from + LOWEST
			(
				_NUM.upto - _NUM.from
				, _VAL.upto - _VAL.from
			);
		
		for ( ; n.bit < stop; alub_inc(&n), alub_inc(&v) )
		{	
			*(n.ptr) &= EITHER( *(v.ptr) & v.mask, mask, ~(n.mask) );
		}
		
		for ( ; n.bit < _NUM.upto; alub_inc(&n) )
		{
			*(n.ptr) &= EITHER( neg, mask, ~(n.mask) );
		}
		
		return 0;
	}
}

int_t alup__or( alup_t _NUM, alup_t _VAL )
{
	
	if ( !_NUM.data || !_VAL.data )
	{
		int_t ret = EINVAL;
		
		alu_error( ret );
		
		if ( !_NUM.data ) alu_puts( "_NUM.data was NULL!" );
		
		if ( !_VAL.data ) alu_puts( "_VAL.data was NULL!" );
		
		return ret;
	}
	else
	{
		alub_t
			n = alub( _NUM.data, _NUM.from )
			, v = alub( _VAL.data, _VAL.from );
		bool_t neg = alup_below0( _VAL );
		size_t stop = _NUM.from + LOWEST
			(
				_NUM.upto - _NUM.from
				, _VAL.upto - _VAL.from
			);
	
		for ( ; n.bit < stop; alub_inc(&n), alub_inc(&v) )
		{
			*(n.ptr) |= IFTRUE( *(v.ptr) & v.mask, n.mask );
		}
		
		for ( ; n.bit < _NUM.upto; alub_inc(&n) )
		{
			*(n.ptr) |= IFTRUE( neg, n.mask );
		}
		
		return 0;
	}
}

int_t alup_xor( alup_t _NUM, alup_t _VAL )
{	
	if ( !_NUM.data || !_VAL.data )
	{
		int_t ret = EINVAL;
		
		alu_error(ret);
		
		if ( !_NUM.data ) alu_puts("_NUM.data was NULL!");
		
		if ( !_VAL.data ) alu_puts("_VAL.data was NULL!");
		
		return ret;
	}
	else
	{
		bool_t neg = alup_below0( _VAL );
		alub_t
			n = alub( _NUM.data, _NUM.from ), v = alub( _VAL.data, _VAL.from );
		size_t stop =
			_NUM.from + LOWEST( _NUM.upto - _NUM.from, _VAL.upto - _VAL.from );
		
		for ( ; n.bit < stop; alub_inc(&n), alub_inc(&v) )
		{
			*(n.ptr) ^= IFTRUE( *(v.ptr) & v.mask, n.mask );
		}
		
		for ( ; n.bit < _NUM.upto; alub_inc(&n) )
		{
			*(n.ptr) ^= IFTRUE( neg, n.mask );
		}
		
		return 0;
	}
	
	return alu_err_null_ptr("alu");
}

int_t alup__mul_int2int
(
	alup_t _NUM
	, alup_t _VAL
	, void *_cpy
)
{
	int ret;
	alup_t _CPY;
	size_t
		diff = _NUM.upto - _NUM.from
		, size = (diff / UNIC_CHAR_BIT) + !!(diff % UNIC_CHAR_BIT)
	;
	
	alup_init_unsigned( _CPY, _cpy, size );
	_CPY.info = _NUM.info;
	
	ret = alup_mov_int2int( _CPY, _NUM );
	
	if ( ret == 0 )
	{
		bool carry = false, caught;
		alub_t v;
		size_t p;
		
		alup_set( _NUM, 0 );
		
		v = alub( _VAL.data, _VAL.from );
		
		for ( p = v.bit; v.bit < _VAL.upto; alub_inc(&v) )
		{
			if ( *(v.ptr) & v.mask )
			{	
				(void)alup__shl( _CPY, v.bit - p );
				ret = alup__add_int2int( _NUM, _CPY );
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

int_t alup__div_int2int( alup_t _NUM, alup_t _VAL, void *_rem, void *_tmp )
{	

	int ret;
	alup_t _SEG, _REM;
	alub_t n;
	bool_t nNeg, vNeg;
	size_t bits, diff, size;
	
	if ( !_NUM.data || !_VAL.data || !_rem || !_tmp )
	{
		ret = EINVAL;
		
		alu_error( ret );
		
		if ( !_NUM.data ) alu_puts("NUM.data was NULL!");
			
		if ( !_VAL.data ) alu_puts("VAL.data was NULL!");
			
		if ( !_rem ) alu_puts("_rem was NULL!");
			
		if ( !_tmp ) alu_puts("_tmp was NULL!");
		
		return ret;
	}
	
	ret = 0;
	nNeg = alup_below0( _NUM );
	vNeg = alup_below0( _VAL );
	
	if ( nNeg )
		alup_neg( _NUM );
	
	if ( vNeg )
		alup_neg( _VAL );
	
	bits = 0;
	diff = _NUM.upto - _NUM.from;
	size = (diff / UNIC_CHAR_BIT) + !!(diff % UNIC_CHAR_BIT);
	
	alup_init_unsigned( _REM, _rem, size );
	_REM.info = _NUM.info;
	
	(void)alup_mov_int2int( _REM, _NUM );
	(void)alup_set( _NUM, 0 );
	
	_SEG = _REM;
	_SEG.info = 0;
	
	n = alup_end_bit( _REM );
	_SEG.upto = _SEG.from = n.bit + 1;
	n = alub( _NUM.data, _NUM.from );
	
	while ( _SEG.from > _REM.from )
	{
		++bits;
		_SEG.from--;
		if ( alup_cmp_int2int( _SEG, _VAL ) >= 0 )
		{
			ret = alup__sub_int2int( _SEG, _VAL );
			
			if ( ret == ENODATA )
				break;
			
			alup__shl( _NUM, bits );
			*(n.ptr) |= n.mask;
			bits = 0;
		}
	}
	
	if ( bits )
		alup__shl( _NUM, bits );
		
	if ( nNeg != vNeg )
		alup_neg( _NUM );
	
	if ( nNeg )
		alup_neg( _REM );
	
	if ( vNeg )
		alup_neg( _VAL );
	
	return ret;
}

int_t alup__div( alup_t _NUM, alup_t _VAL, void *_rem, void *_tmp )
{
	if ( alup_floating( _NUM ) || alup_floating( _VAL ) )
	{
		int_t ret = ENOSYS;
		alu_error(ret);
		return ret;
	}
	
	return alup__div_int2int( _NUM, _VAL, _rem, _tmp );
}
