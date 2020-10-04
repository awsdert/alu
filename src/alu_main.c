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

void alu__print_reg
(
	char *file
	, uint_t line
	, const char *func
	, char *pfx
	, alu_t *alu
	, alu_reg_t REG
	, bool print_info
	, bool print_value
	, bool print_flags
)
{
	void *R;
	alu_bit_t n;
	
	REG.node %= alu_used( alu );
	R = alu_reg_data( alu, REG );

	if ( print_info )
	{
		alu__printf
		(
			"%s: node = %u, part = %p, from = %zu, upto = %zu"
			", active = %c, signed = %c, floating = %c"
			, file
			, line
			, func
			, pfx
			, REG.node
			, R
			, REG.from
			, REG.upto
			, '0' + alu_reg_get_active( alu, REG )
			, '0' + alu_reg_signed( REG )
			, '0' + alu_reg_floating( REG )
		);
	}
	
	if ( print_flags )
	{
		alu_print_info( pfx, alu, REG, -1 );
	}
	
	if ( print_value )
	{
		n = alu_bit( R, REG.upto );
		
		fprintf( stderr, "%s:%u: %s() %s = ", file, line, func, pfx );
		if ( n.bit == REG.from )
		{
			fputc( '0', stderr );
			alu_error( ERANGE );
		}
		while ( n.bit > REG.from )
		{
			alu_bit_dec(&n);
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
	
	full = alu_Nsize( alu ) * CHAR_BIT;
	upto = HIGHEST( upto, 1 );
	upto = LOWEST( upto, full );
	
	REG->upto = upto;
	REG->from = from * (from < upto);
	
	return upto;
}

int_t alu_setup_reg( alu_t *alu, uint_t want, size_t Nsize )
{
	size_t need;
	uintmax_t *block;
	
	if ( alu )
	{
		want = HIGHEST( want, ALU_REG_ID_NEED );
		want = LOWEST( want, ALU_REG_ID_LIMIT );
		
		need = (want / CHAR_BIT) + !!(want % CHAR_BIT);
		Nsize = HIGHEST( Nsize, need );
		need = Nsize % sizeof(uintmax_t);
		Nsize += SET1IF( need, sizeof(uintmax_t) - need );
		
		block = alu_vec_expand( alu, want, Nsize );
		
		if ( block )
		{
			alu->taken = HIGHEST( alu->taken, ALU_REG_ID_NEED );
			alu_set_constants( alu );		
			return 0;
		}
		
		alu_error( alu->block.fault );
		return alu->block.fault;
	}
	
	return alu_err_null_ptr( "alu" );
}
#include <assert.h>

uint_t alu_get_reg_node( alu_t *alu, size_t Nsize )
{
	uint_t count = 0, index, reg;
	
	if ( alu )
	{
		index = ALU_REG_ID_NEED;
		count = alu_used( alu );
		
		for ( reg = index; index < count; ++index )
		{
			reg = index;
			count = SET1IF( alu_get_active( alu, index ), count );
		}
		
		index = HIGHEST( count, reg );
		count = HIGHEST( index + 1, alu_used(alu) );
		Nsize = HIGHEST( Nsize, alu_Nsize( alu ) );
		reg = HIGHEST( count, alu_upto( alu ) );
		
		alu_errno(alu) = alu_setup_reg( alu, reg, Nsize );
				
		if ( alu->given >= count )
		{		
			alu->taken = count;
			
			(void)memset( alu_data( alu, index ), 0, alu->Nsize );
			
			alu_set_active( alu, index );
			
			return index;
		}
		
		alu->block.fault = SET2IF( alu_errno(alu), alu_errno(alu), ERANGE );
		alu_error( alu_errno(alu) );
		alu_printf( "given = %u, needed %u", alu->given, count );
		return 0;
	}
	
	(void)alu_err_null_ptr( "alu" );
	return 0;
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
	uint_t index;
	
	if ( alu )
	{
		for ( index = 0; index < count; ++index )
		{		
			nodes[index] = alu_get_reg_node( alu, need );
			count *= (nodes[index] != 0);
		}
		
		if ( !count )
		{
			alu_error( alu_errno(alu) );
			alu_rem_reg_nodes( alu, nodes, index );
			return alu_errno(alu);
		}
		
		return 0;
	}
	
	return alu_err_null_ptr( "alu" );
}

void alu_set_constants( alu_t *alu )
{
	int r, used = alu_used( alu );
	size_t Nsize = alu_Nsize( alu ), last = Nsize - 1;
	uchar_t *part;
	alu_bit_t c = alu_bit( (void*)alu_valid(alu), 0 );
	
	used = LOWEST( used, ALU_REG_ID_NEED );
	
	*(c.ptr) |= c.mask;
	
	for ( r = 1, alu_bit_inc(&c); r < used; ++r, alu_bit_inc(&c) )
	{
		*(c.ptr) |= c.mask;
		
		part = alu_data( alu, r );
		
		memset
		(
			part
			, -1 * ((r == ALU_REG_ID_UMAX) | (r == ALU_REG_ID_IMAX))
			, Nsize
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
	, alu_reg_t DST
	, alu_base_t base
)
{
	size_t b;
	uint_t nodes[ALU_BASE_COUNT] = {0};
	alu_reg_t NUM, VAL;
	int ret = 0;
	char32_t c = -1;
	char
		*base_upper = ALU_BASE_STR_0toZtoz "+-",
		*base_lower = ALU_BASE_STR_0toztoZ "+-",
		*base_str = base.lowercase ? base_lower : base_upper;
	
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
		alu_error( ret );
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
	
	alu_reg_init( alu, NUM, nodes[ALU_BASE_NUM], 0 );
	alu_reg_init( alu, VAL, nodes[ALU_BASE_VAL], 0 );

	ret = alu_reg_set_raw( alu, VAL, &(base.base), sizeof(size_t), 0 );
	
	if ( ret != 0 )
	{
		alu_error( ret );
		goto fail;
	}

	do
	{	
		/* Failsafe if failed to get character but return code is 0 */
		c = -1;
		ret = src.next( &c, src.src, src.nextpos );
		
		ret = SET1IF( ret != EOF, ret );
		
		if ( ret != 0 )
		{
			alu_error( ret );
			break;
		}
		
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
		
		alu_reg_set_raw( alu, NUM, &b, sizeof(size_t), 0 );
		
		ret = alu_reg_mul( alu, DST, VAL );
		
		if ( ret == EOVERFLOW )
		{
			goto maxout;
		}
		
		ret = alu_reg_add( alu, DST, NUM );
		
		ret = SET1IF( ret != ENODATA, ret );
	}
	while ( ret == 0 );
	
	if ( ret == EOVERFLOW )
	{
		maxout:
		alu_reg_set_max( alu, DST );
	}
	
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
	size_t i, Nsize, bits, _base, _one = 1;
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
		alu_error( ret );
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
		alu_error( ret );
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
					alu_error( ret );
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
				alu_error( ret );
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
		
		Nsize = alu_Nsize( alu );
		/* Make sure have enough space for later calculations */
		bits = dst.upto - dst.from;
		if ( bits >= (alu_Nbits(alu) / 2) )
			return ERANGE;
		
		/* Check how many bits to assign to exponent & mantissa */
		man_dig = SET2IF
		(
			Nsize < 2
			, (CHAR_BIT / 2) - 1
			, SET2IF
			(
				Nsize < sizeof(float)
				, CHAR_BIT - 1
				, SET2IF
				(
					Nsize < sizeof(double)
					, FLT_MANT_DIG
					, SET2IF
					(
						Nsize < sizeof(long double)
						, DBL_MANT_DIG
						, LDBL_MANT_DIG
					)
				)
			)
		);
		
		exp_dig = ((Nsize * CHAR_BIT) - man_dig) - 1;
		
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
				(void)alu_reg_divide( alu, ONE, BASE, VAL, TMP );
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
			
		(void)alu_reg_divide( alu, dst, ONE, DOT, TMP );
		
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
					n = alu_bit( part, MAN.from );
					
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
			n = alu_bit( part, MAN.from );
			
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
				alu_error( ret );
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
	alu_reg_t NUM, VAL, REM, TMP;
	int ret;
	size_t digit = 0, b;
	uint_t nodes[ALU_BASE_COUNT] = {0};
	bool neg;
	char *base_str =
		base.lowercase ? ALU_BASE_STR_0toztoZ : ALU_BASE_STR_0toZtoz;
	
	src.upto = LOWEST( alu_Nbits( alu ), src.upto );
	
	if ( !(dst.next) || !(dst.flip) )
	{
		ret = EADDRNOTAVAIL;
		alu_error( ret );
		return ret;
	}
		
	if ( !base.base )
		base.base = 10;
	
	if ( base.base > strlen(base_str) )
	{
		ret = ERANGE;
		alu_error( ret );
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
	alu_reg_init( alu, REM, nodes[ALU_BASE_REM], 0 );
	alu_reg_init( alu, TMP, nodes[ALU_BASE_TMP], 0 );
	
	neg = alu_reg_below0( alu, src );
	
	alu_reg_mov( alu, NUM, src );
	alu_set_raw( alu, VAL.node, base.base, 0 );
	
	if ( neg )
		(void)alu_reg_neg( alu, NUM );

	while ( alu_reg_cmp( alu, NUM, VAL ) >= 0 )
	{
		ret = alu_reg_divide( alu, NUM, VAL, REM, TMP );
		
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
		
		alu_reg_get_raw( alu, REM, &b, sizeof(size_t) );
		ret = dst.next( base_str[b], dst.dst );
		
		if ( ret != 0 )
		{
			alu_error( ret );
			goto fail;
		}
		
		++digit;
	}
	
	alu_reg_get_raw( alu, NUM, &b, sizeof(size_t) );
	ret = dst.next( base_str[b], dst.dst );
	
	if ( ret != 0 )
	{
		alu_error( ret );
		goto fail;
	}
	
	dst.flip( dst.dst );
	
	fail:
	alu_rem_reg_nodes( alu, nodes, ALU_BASE_COUNT );
	
	return ret;
}
