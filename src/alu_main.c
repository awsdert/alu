#include "alu.h"
#include <string.h>
#include <stdio.h>

void alu_print_info( char *pfx, alu_t *alu, alu_reg_t reg, uint_t info )
{
	reg.node %= alu_used( alu );
	alu_printf( "Checking %s.info for flags:", pfx );

	alu_printf
	(
		"alu_reg_get_active( alu, %s ), has this flag?... %s"
		, pfx, alu_reg_get_active( alu, reg ) ? "Yes" : "No"
	);
	
	if ( info & ALU_INFO__SIGN )
	{
		alu_printf
		(
			"alu_reg_signed( %s ), has this flag?... %s"
			, pfx, alu_reg_signed( reg ) ? "Yes" : "No"
		);
	}
	
	if ( info & ALU_INFO_FLOAT )
	{
		alu_printf
		(
			"alu_reg_floating( %s ), has this flag?... %s"
			, pfx, alu_reg_floating( reg ) ? "Yes" : "No"
		);
	}
}

void alu_print_reg( char *pfx, alu_t *alu, alu_reg_t reg, bool print_info, bool print_value )
{
	void *part;
	alu_bit_t n;
	
	reg.node %= alu_used( alu );
	part = alu_reg_data( alu, reg );
	n = alu_bit_set_bit( part, reg.upto );

	reg.upto = alu_bits_perN(alu);

	if ( print_info )
	{
		alu_printf
		(
			"%s: node = %u, part = %p, from = %zu, upto = %zu"
			, pfx
			, reg.node
			, part
			, reg.from
			, reg.upto
		);
		
		alu_print_info( pfx, alu, reg, -1 );
	}
	
	if ( print_value )
	{
		fprintf( stderr, "%s = ", pfx );
		if ( n.bit == reg.from )
		{
			fputc( '0', stderr );
			alu_error( ERANGE );
		}
		while ( n.bit > reg.from )
		{
			n = alu_bit_dec(n);
			(void)fputc( '0' + !!(*(n.ptr) & n.mask), stderr );
		}
		fputc( '\n', stderr );
	}
}

int_t alu__err_null_ptr
(
	char const * const file
	, uint_t line
	, char const * const func
	, char const * const name
)
{
	alu__printf( "'%s' was null", file, line, func, name );
	return EDESTADDRREQ;
}

int_t alu__err_range
(
	char const * const file
	, uint_t line
	, char const * const func
	, char const * const name
)
{
	alu__printf( "'%s' was out of range", file, line, func, name );
	return ERANGE;
}

size_t alu_set_bounds( alu_t *alu, alu_reg_t *REG, size_t from, size_t upto )
{
	size_t full;
	
	full = alu_size_perN( alu ) * CHAR_BIT;
	upto = HIGHEST( upto, 1 );
	upto = LOWEST( upto, full );
	
	REG->upto = upto;
	REG->from = from * (from < upto);
	
	return upto;
}

int_t alu_setup_reg( alu_t *alu, uint_t want, uint_t used, size_t perN )
{
	int ret;
	uint_t i;
	size_t need;
	
	if ( alu )
	{
		want = HIGHEST( want, ALU_REG_ID_NEED );
		want = LOWEST( want, ALU_REG_ID_LIMIT );
		used = LOWEST( want, HIGHEST( used, ALU_REG_ID_NEED ) );
		
		need = (want / CHAR_BIT) + !!(want % CHAR_BIT);
		perN = HIGHEST( perN, need );
		perN += LOWEST
		(
			sizeof(uintmax_t)
			, sizeof(uintmax_t) - (perN % sizeof(uintmax_t))
		);
		
		ret = alu_vec_expand( alu, want, perN );
		
		if ( ret == 0 )
		{
			alu_set_used( alu, used );
			
			for ( i = 0; i < ALU_REG_ID_NEED; ++i )
			{
				alu_set_active( alu, i );
			}
			
			alu_set_constants( alu );
			return 0;
		}
		
		alu_error( ret );
		return ret;
	}
	
	return alu_err_null_ptr( "alu" );
}

int_t alu_get_reg_node( alu_t *alu, uint_t *dst, size_t need )
{
	int ret;
	uint_t count = 0, r = count;
	void *part;
	
	if ( alu )
	{
		if ( dst )
		{
			*dst = 0;
			
			need = HIGHEST( need, alu_size_perN( alu ) );
			count = alu_used( alu );
			
			for ( r = ALU_REG_ID_NEED; r < count; ++r )
			{
				count = SET1IF( alu_get_active( alu, r ), count );
			}
			
			r = SET2IF( count, count, r - 1 );
			count = alu_used( alu );
			
			if ( r >= alu_upto( alu ) || need > alu_size_perN( alu ) )
			{
				count = HIGHEST( r + 1, alu_upto( alu ) );
				ret = alu_setup_reg( alu, count, count, need );
					
				if ( ret != 0 )
				{
					alu_error( ret );
					return ret;
				}
			}
			
			alu_used( alu ) = HIGHEST( r + 1, count );
			
			part = alu_data( alu, r );
			(void)memset( part, 0, need );
			
			alu_set_active( alu, r );
			
			*dst = r;
			
			return 0;
		}
		
		return alu_err_null_ptr( "dst" );		
	}
	
	return alu_err_null_ptr( "alu" );
}

int_t alu_rem_reg_nodes( alu_t *alu, uint_t *nodes, int count )
{	
	if ( alu )
	{
		if ( nodes )
		{		
			for ( --count; count >= 0; --count )
			{
				alu_rem_reg_node( alu, nodes + count );
			}
			
			return 0;
		}
		
		return alu_err_null_ptr( "nodes" );
	}
	
	return alu_err_null_ptr( "alu" );
}

int_t alu_get_reg_nodes( alu_t *alu, uint_t *nodes, uint_t count, size_t need )
{
	int ret = 0;
	uint_t r;
	uint_t *dst;
	
	if ( alu )
	{
		for ( r = 0; r < count; ++r )
		{
			dst = nodes + r;
				
			ret = alu_get_reg_node( alu, dst, need );
			count *= (ret == 0);
		}
		
		if ( ret != 0 )
		{
			alu_error( ret );
			alu_rem_reg_nodes( alu, nodes, r );
		}
		
		return ret;
	}
	
	return alu_err_null_ptr( "alu" );
}

void alu_set_constants( alu_t *alu )
{
	int r, used = alu_used( alu );
	size_t perN = alu_size_perN( alu ), last = perN - 1;
	uchar_t *part;
	
	used = LOWEST( used, ALU_REG_ID_NEED );
	
	for ( r = 0; r < used; ++r )
	{
		part = alu_data( alu, r );
		
		memset
		(
			part
			, -1 * ((r == ALU_REG_ID_UMAX) | (r == ALU_REG_ID_IMAX))
			, perN
		);
		
		part[last] =
			(SCHAR_MAX * (r == ALU_REG_ID_IMAX))
			| (part[last] * (r != ALU_REG_ID_IMAX))
		;
		
		part[last] =
			(SCHAR_MIN * (r == ALU_REG_ID_IMIN))
			| (part[last] * (r != ALU_REG_ID_IMIN))
		;
	}
}

int_t alu_str2reg
(
	alu_t *alu
	, alu_src_t src
	, alu_reg_t dst
	, alu_base_t base
)
{
	size_t b, perN;
	uint_t nodes[ALU_BASE_COUNT] = {0};
	alu_reg_t num, val;
	alu_bit_t n;
	int ret = 0;
	char32_t c = -1;
	char
		*base_upper = ALU_BASE_STR_0toZtoz "+-",
		*base_lower = ALU_BASE_STR_0toztoZ "+-",
		*base_str = base.lowercase ? base_lower : base_upper;
	void *part;
	
	if ( !(src.nextpos) )
		return EDESTADDRREQ;
	
	if ( *(src.nextpos) < 0 )
		*(src.nextpos) = 0;
	
	if ( !(src.next) )
		return EADDRNOTAVAIL;
	
	switch ( base.digsep )
	{
	case 0: case '\'': case '_': case ',': break;
	default:
		alu_error(ret);
		alu_printf( "base.digsep = '%c'", base.digsep );
		return EINVAL;
	}
	
	if ( !(base.base) || base.base > strlen(base_str) )
		return ERANGE;
		
	ret = alu_get_reg_nodes( alu, nodes, ALU_BASE_COUNT, 0 );
	
	if ( ret != 0 )
	{
		alu_error( ret );
		return ret;
	}
	
	alu_reg_init( alu, num, nodes[ALU_BASE_NUM], 0 );
	alu_reg_init( alu, val, nodes[ALU_BASE_VAL], 0 );

	/* Clear all registers in direct use */
	perN = alu_size_perN( alu );
	
	alu_uint_set_raw( alu, val.node, base.base );
	
	part = alu_reg_data( alu, num );
	n = alu_bit_set_bit( part, num.upto - 1 );
	
	alu_set_bounds( alu, &num, 0, -1 );
	alu_reg_clr( alu, num );

	do
	{	
		/* Failsafe if failed to get character but return code is 0 */
		c = -1;
		ret = src.next( &c, src.src, src.nextpos );
		
		if ( ret != 0 && ret != EOF )
			goto fail;
		
		if ( base.base > 36 )
		{
			for ( b = 0; b < base.base; ++b )
			{
				if ( c == (char32_t)(base_str[b]) )
					break;
			}
		}
		else
		{
			for ( b = 0; b < base.base; ++b )
			{
				if ( c == (char32_t)(base_upper[b]) )
					break;
			}
			
			for ( b = 0; b < base.base; ++b )
			{
				if ( c == (char32_t)(base_lower[b]) )
					break;
			}
		}
		
		if ( b == base.base )
		{
			if ( c == base.digsep )
			{
				++(*(src.nextpos));
				continue;
			}
			break;
		}
		
		++(*(src.nextpos));
		
		if ( *(n.ptr) & n.mask )
		{
			ret = alu_setup_reg
			(
				alu
				, alu_upto( alu )
				, alu_used( alu )
				, perN + sizeof(size_t)
			);
			
			if ( ret != 0 )
				goto fail;
			
			alu_set_bounds( alu, &num, 0, -1 );
			
			perN = alu_size_perN( alu );
			part = alu_reg_data( alu, num );
			n = alu_bit_set_bit( part, num.upto - 1 );
		}
		
		alu_uint_set_raw( alu, num.node, b );
		
		(void)alu_reg_mul( alu, dst, val );
		(void)alu_reg_add( alu, dst, num );
	}
	while ( ret == 0 );
	
	fail:
	alu_rem_reg_nodes( alu, nodes, ALU_BASE_COUNT );
	
	return ret;
}

bool alu_reg_is_zero( alu_t *alu, alu_reg_t reg, alu_bit_t *end_bit )
{
	alu_bit_t n = alu_reg_end_bit( alu, reg );
	if ( end_bit ) *end_bit = n;
	return !(*(n.ptr) & n.mask);
}

int_t alu_lit2reg
(
	alu_t *alu
	, alu_src_t src
	, alu_reg_t dst
	, alu_base_t base
)
{
	size_t i, perN, bits, _base, _one = 1;
	uint_t nodes[ALU_LIT_COUNT] = {0};
	alu_reg_t NIL, VAL, BASE, ONE, DOT, EXP, BIAS, MAN, TMP;
	alu_bit_t n;
	int ret;
	long pos, prevpos, exp_dig, man_dig;
	char32_t c = -1, exponent_char = 'e';
	bool neg = false, exp_neg = false, closing_bracket = false;
	void *part;
	
	if ( !(src.nextpos) )
		return EDESTADDRREQ;
	
	if ( *(src.nextpos) < 0 )
		*(src.nextpos) = 0;
		
	ret = alu_get_reg_nodes( alu, nodes, ALU_LIT_COUNT, 0 );
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	dst.node %= alu_used( alu );
	ret = alu_check1( alu, dst.node );
	
	if ( !(src.next) )
	{
		ret = EADDRNOTAVAIL;
		alu_error( ret );
		goto fail;
	}
	
	switch ( base.digsep )
	{
	case 0: case '\'': case '_': case ',': break;
	default:
		ret = EINVAL;
		alu_error(ret);
		alu_printf( "base.digsep = '%c'", base.digsep );
		goto fail;
	}
		
	ret = src.next( &c, src.src, src.nextpos );
	if ( ret != 0 )
	{
		alu_error( ret );
		goto fail;
	}
	
	if ( c == '-' || c == '+' )
	{
		if ( c == '-' )
			neg = true;
		++(*(src.nextpos));
	}
		
	ret = src.next( &c, src.src, src.nextpos );
	if ( ret != 0 )
		goto fail;
		
	if ( c > '0' && c <= '9' )
	{
		base.base = 10;
		++(*(src.nextpos));
	}
	else if ( c != '0' )
	{
		ret = EILSEQ;
		goto fail;
	}
	else
	{
		++(*(src.nextpos));
		
		ret = src.next( &c, src.src, src.nextpos );
		if ( ret != 0 )
			goto fail;
		
		if ( c >= '0' && c <= '7' )
		{
			++(*(src.nextpos));
			base.base = 8;
		}
		else
		{
			switch ( c )
			{
			case '~':
				closing_bracket = true;
				base.base = 0;
				++(*(src.nextpos));
				ret = src.next( &c, src.src, src.nextpos );
				
				if ( ret != 0 )
					goto fail;
				
				if ( c == 'l' || c == 'L' )
				{
					++(*(src.nextpos));
					base.lowercase = true;
				}
				
				while ( 1 )
				{
					ret = src.next( &c, src.src, src.nextpos );
					if ( ret != 0 || c <= '0' || c >= '9' )
						break;
					
					base.base *= 10;
					base.base += (c - '0');
					++(*(src.nextpos));
				}
				
				if ( ret != 0 && ret != EOF )
					goto fail;
				
				if ( !base.base )
				{
					ret = EILSEQ;
					alu_error(ret);
					alu_puts( "Invalid base provided" );
					goto fail;
				}
				
				break;
			case 'b': case 'B': base.base = 2; ++(*(src.nextpos)); break;
			case 'o': case 'O': base.base = 8; ++(*(src.nextpos)); break;
			case 'x': case 'X':
				base.base = 16;
				++(*(src.nextpos));
				exponent_char = 'p';
				break;
			default:
				ret = EILSEQ;
				alu_error(ret);
				alu_puts( "Expected x, X, o, O, b, B, ~ or 1 to 9" );
				goto fail;
			}
		}
	}
	
	alu_reg_init( alu, NIL, ALU_REG_ID_ZERO, 0 );
	alu_reg_init( alu, BASE, nodes[ALU_LIT_BASE], 0 );
	alu_reg_init( alu, ONE, nodes[ALU_LIT_ONE], 0 );
	alu_reg_init( alu, VAL, nodes[ALU_LIT_VAL], 0 );
	alu_reg_init( alu, TMP, nodes[ALU_LIT_TMP], 0 );
	alu_reg_init( alu, DOT, nodes[ALU_LIT_DOT], 0 );
	alu_reg_init( alu, EXP, nodes[ALU_LIT_EXP], 0 );
	alu_reg_init( alu, BIAS, nodes[ALU_LIT_EXP_BIAS], 0 );
	alu_reg_init( alu, MAN, nodes[ALU_LIT_MAN], 0 );
	
	alu_uint_set_raw( alu, BASE.node, base.base );
	
	ret = alu_str2reg( alu, src, dst, base );
	
	switch ( ret )
	{
	case 0: case EOF: c = 0; break;
	default: goto fail;
	}

	perN = alu_size_perN( alu );
	
	/* Check if reading a floating point number */
	if ( c == '.' )
	{
		++(*(src.nextpos));
		prevpos = *(src.nextpos);
		
		ret = alu_str2reg( alu, src, DOT, base );
		
		switch ( ret )
		{
		case 0: case EOF: c = 0; break;
		default: goto fail;
		}
		
		/* Make sure have enough space for later calculations */
		bits = dst.upto - dst.from;
		if ( bits == (perN * CHAR_BIT) )
		{
			ret = alu_setup_reg
			(
				alu
				, alu_upto( alu )
				, alu_used( alu )
				, perN * 2
			);
			
			if ( ret != 0 )
			{
				alu_error(ret);
				goto fail;
			}
			
			perN = alu_size_perN( alu );
		}
		
		/* Check how many bits to assign to exponent & mantissa */
		man_dig = SET2IF
		(
			perN < 2
			, (CHAR_BIT / 2) - 1
			, SET2IF
			(
				perN < sizeof(float)
				, CHAR_BIT - 1
				, SET2IF
				(
					perN < sizeof(double)
					, FLT_MANT_DIG
					, SET2IF
					(
						perN < sizeof(long double)
						, DBL_MANT_DIG
						, LDBL_MANT_DIG
					)
				)
			)
		);
		
		exp_dig = ((perN * CHAR_BIT) - man_dig) - 1;
		
		/* Update Exponent & Mantissa Bounds */
		alu_set_bounds( alu, &BIAS, 0, exp_dig );
		alu_set_bounds( alu, &EXP, 0, exp_dig );
		alu_set_bounds( alu, &MAN, 0, man_dig );
		
		/* Setup bias for exponent */
		alu_reg_set_max( alu, BIAS );
		alu_reg__shr( alu, BIAS, TMP, 1 );
		
		/* Set '0.1' */
		alu_uint_set_raw( alu, ONE.node, _one );
		
		/* Multiply '0.1' by base to get '1.0',
		 * maybe be more digits than size_t supports */
		for ( pos = prevpos; pos < *(src.nextpos); ++pos )
			(void)alu_reg_mul( alu, ONE, BASE );
		
		/* Only universal number literals need this, example of one:
		 * 0~L36(A.z)e+10 */
		if ( closing_bracket )
		{
			if ( c == ')' )
			{
				++(*(src.nextpos));
				ret = src.next( &c, src.src, src.nextpos );
				if ( ret != 0 && ret != EOF )
					goto fail;
			}
			else
			{
				ret = EILSEQ;
				goto fail;
			}
		}
		
		if ( c == exponent_char || c == (char32_t)toupper(exponent_char) )
		{
			++(*(src.nextpos));
			ret = src.next( &c, src.src, src.nextpos );
			if ( ret != 0 && ret != EOF )
				return ret;
			
			if ( c == '-' || c == '+' )
			{
				if ( c == '-' )
					exp_neg = true;
				++(*(src.nextpos));
			}
			
			_base = base.base;
			base.base = 10;
			alu_uint_set_raw( alu, BASE.node, 10 );
			
			ret = alu_str2reg( alu, src, EXP, base );
			
			alu_uint_set_raw( alu, BASE.node, _base );
			base.base = _base;
			
			switch ( ret )
			{
			case 0: case EOF: c = 0; break;
			default: goto fail;
			}
		}
		
		/* Begin Constructing float */
		
		if ( alu_reg_is_zero( alu, DOT, &n ) )
		{
			pos = 0;
			alu_uint_set_raw( alu, ONE.node, _one );
		}
		
		if ( !exp_neg )
		{
			for
			(
				; !alu_reg_is_zero( alu, EXP, NULL )
				; ++pos, (void)alu_reg_dec( alu, EXP ) )
			{
				/* Check we haven't hit 1 */
				if ( pos == 0 || alu_reg_is_zero( alu, ONE, &n ) )
				{
					break;
				}
				(void)alu_reg_divide( alu, ONE, BASE, VAL );
			}
		}
		
		if ( alu_reg_cmp( alu, EXP, BIAS ) > 0 )
		{
			if ( exp_neg )
			{
				set_nil:
				alu_reg_clr( alu, dst );
				goto set_sign;
			}
			else
			{
				set_inf:
				alu_reg_clr( alu, dst );
				alu_reg_set_max( alu, EXP );
				goto set_exp;
			}
		}
		
		
		for
		(
			; alu_reg_cmp( alu, EXP, NIL ) > 0
			; (void)alu_reg_dec( alu, EXP )
		)
		{
			ret = alu_reg_mul( alu, dst, BASE );
			if ( ret == EOVERFLOW )
				goto set_inf;
		}
		
		for
		(
			; alu_reg_cmp( alu, EXP, NIL ) < 0
			; (void)alu_reg_inc( alu, EXP )
		)
		{
			ret = alu_reg_mul( alu, ONE, BASE );
			
			if ( ret == EOVERFLOW )
				goto set_nil;
		}
		
		if ( alu_reg_is_zero( alu, ONE, &n ) )
			goto set_nil;
			
		(void)alu_reg_divide( alu, dst, ONE, DOT );
		
		if ( alu_reg_is_zero( alu, DOT, &n ) )
			alu_uint_set_raw( alu, ONE.node, _one );
		
		pos = 0;
		
		/* Calculate final exponent */
		
		if ( alu_reg_cmp( alu, dst, ONE ) >= 0 )
		{
			for
			(
				alu_reg_mov( alu, VAL, dst )
				; alu_reg_cmp( alu, VAL, ONE ) > 0
				; ++pos, alu_reg__shr( alu, VAL, TMP, 1 )
			);
		}
		else
		{
			for
			(
				alu_reg_mov( alu, VAL, ONE )
				; alu_reg_cmp( alu, VAL, DOT ) > 0
				; --pos, alu_reg__shr( alu, VAL, TMP, 1 )
			);
			
			if ( alu_reg_cmp( alu, VAL, DOT ) == 0 )
			{
				--pos;
			}
		}
		
		/* Set bias */
		alu_reg_mov( alu, dst, BIAS );
		alu_uint_set_raw( alu, VAL.node, man_dig );
		
		
		/* Construct Mantissa */
		if ( pos > man_dig )
		{
			pos -= man_dig;
			alu_reg_set_max( alu, DOT );
			alu_reg__shl( alu, DOT, TMP, pos );
			alu_reg_not( alu, DOT );
			alu_reg_and( alu, DOT, dst );
			alu_reg__shl( alu, ONE, TMP, pos );
			alu_reg_mov( alu, MAN, VAL );
			alu_reg_mov( alu, VAL, ONE );
			alu_reg__shr( alu, VAL, TMP, 1 );
			
			i = alu_reg_cmp( alu, DOT, VAL );
			
			if ( i > 0 )
				(void)alu_reg_inc( alu, MAN );
			else if ( i == 0 )
			{
				switch ( FLT_ROUNDS )
				{
				case 1:
					part = alu_reg_data( alu, MAN );
					n = alu_bit_set_bit( part, MAN.from );
					
					if ( *(n.ptr) & n.mask )
						(void)alu_reg_inc( alu, MAN );
					
					break;
				}
			}
		}
		else
		{
			(void)alu_reg_mov( alu, MAN, dst );	
			(void)alu_reg_mov( alu, VAL, DOT );
			
			part = alu_reg_data( alu, MAN );
			n = alu_bit_set_bit( part, MAN.from );
			
			for ( bits = 0; pos < man_dig; ++pos )
			{
				bits++;
				
				if ( alu_reg_cmp( alu, VAL, ONE ) >= 0 )
				{
					(void)alu_reg_sub( alu, VAL, ONE );
					(void)alu_reg__shl( alu, MAN, TMP, bits );
					*(n.ptr) |= n.mask;
					bits = 0;
				}
			}
			
			if ( bits )
				(void)alu_reg__shl( alu, MAN, TMP, bits );
		}
		
		/* TODO: Continue referencing code made in mitsy to build fpn */
		
		/* Construct FPN from modified values */
		alu_reg_mov( alu, dst, MAN );
		
		set_exp:
		/* Align and append Exponent */
		(void)alu_set_bounds( alu, &EXP, 0, -1 );
		alu_reg__shl( alu, EXP, TMP, man_dig );
		alu_reg__or( alu, dst, EXP );
		
		set_sign:
		if ( neg )
		{
			alu_uint_set_raw( alu, ONE.node, _one );
			alu_reg__shl( alu, ONE, TMP, exp_dig + man_dig );
			alu_reg__or( alu, dst, ONE );
		}
	}
	else 
	{
		if ( closing_bracket )
		{
			if ( c == ')' )
			{
				++(*(src.nextpos));
				ret = src.next( &c, src.src, src.nextpos );
				if ( ret != 0 && ret != EOF )
					goto fail;
			}
			else
			{
				ret = EILSEQ;
				alu_error(ret);
				alu_puts( "Expected ')' to close Exponent or Float" );
				alu_printf( "Current character is '%c'", c );
				goto fail;
			}
		}
		
		if ( neg )
			alu_reg_neg( alu, dst );
	}
	
	fail:
	
	alu_rem_reg_nodes( alu, nodes, ALU_LIT_COUNT );
	return 0;
}

int_t alu_reg2str( alu_t *alu, alu_dst_t dst, alu_reg_t src, alu_base_t base )
{
	alu_reg_t NUM, VAL, TMP;
	int ret;
	size_t digit = 0, _num, _val, failsafe = 0;
	uint_t nodes[ALU_BASE_COUNT] = {0};
	bool neg;
	char *base_str =
		base.lowercase ? ALU_BASE_STR_0toztoZ : ALU_BASE_STR_0toZtoz;
	
	src.upto = LOWEST( alu_bits_perN( alu ), src.upto );
	
	if ( !(dst.next) || !(dst.flip) )
	{
		ret = EADDRNOTAVAIL;
		alu_error(ret);
		return ret;
	}
		
	if ( !base.base )
		base.base = 10;
	
	if ( base.base > strlen(base_str) )
	{
		ret = ERANGE;
		alu_error(ret);
		return ret;
	}
	
	ret = alu_get_reg_nodes( alu, nodes, ALU_BASE_COUNT, 0 );
	
	if ( ret != 0 )
	{
		alu_error( ret );
		return ret;
	}
	
	alu_reg_init( alu, NUM, nodes[ALU_BASE_NUM], 0 );
	alu_reg_init( alu, VAL, nodes[ALU_BASE_VAL], 0 );
	alu_reg_init( alu, TMP, nodes[ALU_BASE_TMP], 0 );
	
	neg = alu_reg_below0( alu, src );
	
	alu_reg_mov( alu, NUM, src );
	alu_set_raw( alu, VAL.node, base.base, 0 );
	
	if ( neg )
		(void)alu_reg_neg( alu, NUM );
	
	switch ( base.base )
	{
	case 2: case 8: case 10: case 16: break;
	default:
		ret = dst.next( ')', dst.dst );
		if ( ret != 0 )
			goto fail;
	}

	for ( failsafe = NUM.from; failsafe < NUM.upto; ++failsafe )
	{
		ret = alu_reg_divide( alu, NUM, VAL, TMP );
		
		if ( ret == ENODATA )
		{
			alu_error( ret );
			goto fail;
		}
		
		if ( digit == 3 )
		{
			switch ( base.digsep )
			{
			case '\'': case '_': case ',':
				ret = dst.next( base.digsep, dst.dst );
				if ( ret != 0 )
					goto fail;
			}
			digit = 0;
		}
		
		alu_get_raw( alu, TMP.node, &_val );
		ret = dst.next( base_str[_val], dst.dst );
		
		if ( ret != 0 )
			goto fail;
		
		++digit;
		
		if ( alu_reg_cmp( alu, NUM, VAL ) < 0 )
			break;
	}
	
	alu_get_raw( alu, NUM.node, &_num );
	ret = dst.next( base_str[_num], dst.dst );
	
	if ( ret != 0 )
		goto fail;
		
	if ( base.base != 10 )
	{
		if ( base.base == 2 )
		{
			ret = dst.next( 'b', dst.dst );
			if ( ret != 0 )
				goto fail;
			
			ret = dst.next( '0', dst.dst );
			if ( ret != 0 )
				goto fail;
		}
		else if ( base.base == 8 )
		{
			ret = dst.next( 'o', dst.dst );
			if ( ret != 0 )
				goto fail;
			
			ret = dst.next( '0', dst.dst );
			if ( ret != 0 )
				goto fail;
		}
		else if ( base.base == 16 )
		{
			ret = dst.next( 'x', dst.dst );
			if ( ret != 0 )
				goto fail;
			
			ret = dst.next( '0', dst.dst );
			if ( ret != 0 )
				goto fail;
		}
		else
		{
			ret = dst.next( '(', dst.dst );
			if ( ret != 0 )
				goto fail;
			
			while ( base.base > 10 )
			{
				ret = dst.next( base_str[base.base%10], dst.dst );
				if ( ret != 0 )
					goto fail;
				base.base /= 10;
			}
			
			ret = dst.next( base_str[base.base], dst.dst );
			if ( ret != 0 )
				goto fail;
			
			ret = dst.next( '~', dst.dst );
			if ( ret != 0 )
				goto fail;
			
			ret = dst.next( '0', dst.dst );
			if ( ret != 0 )
				goto fail;
		}
	}
	
	dst.flip( dst.dst );
	
	fail:
	
	alu_rem_reg_nodes( alu, nodes, ALU_BASE_COUNT );
	
	return ret;
}
