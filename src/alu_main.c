#include "alu.h"
#include <string.h>
#include <stdio.h>

void alup__print
(
	char *file
	, uint_t line
	, const char *func
	, char *pfx
	, alup_t _PTR
	, bool print_info
	, bool print_value
)
{
	alup_t _EXP;
	
	alur_init_exponent( _PTR, _EXP );

	if ( print_info )
	{
		size_t exp_dig = alup_floating( _PTR ) ? _EXP.upto - _EXP.from : 0;
		size_t man_dig = alup_floating( _PTR ) ? _EXP.from - _PTR.from : 0;
		
		alu__printf
		(
			"%s: part = %p, from = %zu, upto = %zu"
			", signed = %c, floating = %c"
			", exp_dig = %zu, man_dig = %zu"
			, file
			, line
			, func
			, pfx
			, _PTR.data
			, _PTR.from
			, _PTR.upto
			, '0' + alup___signed( _PTR )
			, '0' + alup_floating( _PTR )
			, exp_dig
			, man_dig
		);
	}
	
	if ( print_value )
	{
		alub_t p = alub( _PTR.data, _PTR.upto );
		
		fprintf( stderr, "%s:%u: %s() %s = ", file, line, func, pfx );
		
		if ( alup___signed( _PTR ) )
		{
			alub_dec(&p);
			(void)fputc( '0' + !!(*(p.ptr) & p.mask), stderr );
			(void)fputc( ' ', stderr );
		}
		
		if ( alup_floating( _PTR ) )
		{
			size_t exp;
			alup_t _TMP;
			char str[bitsof(size_t)] = {0};
			
			alup_init_unsigned( _TMP, &exp, sizeof(size_t) );
			
			(void)alup_mov_int2int( _TMP, _EXP );
			
			exp -= alup_get_exponent_bias(_PTR);
			
			sprintf( str, "%+05zd ", (ssize_t)exp );
			
			fputs( str, stderr );
			
			while ( p.bit > _EXP.from )
			{
				alub_dec(&p);
				(void)fputc( '0' + !!(*(p.ptr) & p.mask), stderr );
			}
			
			(void)fputc( ' ', stderr );
			while ( p.bit > _PTR.from )
			{
				alub_dec(&p);
				(void)fputc( '0' + !!(*(p.ptr) & p.mask), stderr );
			}
		}
		else
		{
			while ( p.bit > _PTR.from )
			{
				alub_dec(&p);
				(void)fputc( '0' + !!(*(p.ptr) & p.mask), stderr );
			}
		}
		
		fputc( '\n', stderr );
	}
}

void alur__print
(
	char *file
	, uint_t line
	, const char *func
	, char *pfx
	, alu_t *alu
	, alur_t REG
	, bool print_info
	, bool print_value
)
{
	alup_t _PTR;
	
	REG.node %= alu_used( alu );
	
	alup_init_register( alu, _PTR, REG );

	if ( print_info )
	{		
		alu__printf
		(
			"%s: node = %u, active = %c"
			, file
			, line
			, func
			, pfx
			, REG.node
			, '0' + alur_get_active( alu, REG )
		);
	}
	
	alup__print( file, line, func, pfx, _PTR, print_info, print_value );
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

int_t alu__err_domain
(
	char const * const file
	, uint_t line
	, char const * const func
	, char const * const name
)
{
	alu__printf( "'%s' was out of alu domain", file, line, func, name );
	return EDOM;
}

size_t alu_set_bounds( alu_t *alu, alur_t *REG, size_t from, size_t upto )
{
	size_t full;
	
	full = alu_Nsize( alu ) * CHAR_BIT;
	upto = HIGHEST( upto, 1 );
	upto = LOWEST( upto, full );
	
	REG->upto = upto;
	REG->from = from * (from < upto);
	
	return upto;
}

int_t alu_setup_reg( alu_t *alu, uint_t want, size_t need )
{
	if ( alu )
	{
		uintmax_t *block;
		
		want = HIGHEST( want, ALU_REG_ID_NEED );
		
		want = (want / bitsof(uintmax_t)) + !!(want % bitsof(uintmax_t));
		want *= bitsof(uintmax_t);
		
		/* Ensure we can set active state for each register via reserved
		 * register 0 */
		need = HIGHEST( need, want );
		
		/* Ensure we give at least the same amount as what other registers have
		 * been given */
		need = HIGHEST( need, alu_Nsize(alu) );
		
		/* Ensure the size is compatible with ((alub_t)({0}).ptr */
		need = (need / sizeof(uintmax_t)) + !!(need % sizeof(uintmax_t));
		need *= sizeof(uintmax_t);
		
		block = alu_vec_expand( alu, want, need );
		
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

uint_t alur_get_node( alu_t *alu, size_t Nsize )
{
	if ( alu )
	{
		uint_t count = alu_used( alu ), index = ALU_REG_ID_NEED, reg;
		
		for ( reg = index; index < count; ++index )
		{
			reg = index;
			count = IFTRUE( alu_get_active( alu, index ), count );
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
		else
		{
			int ret = alu_errno(alu);
			
			ret = EITHER( ret, ret, EIO );
			alu->block.fault = ret;
			alu_error( ret );
			alu_printf( "given = %u, needed %u", alu->given, count );
		}
		
		return 0;
	}
	
	(void)alu_err_null_ptr( "alu" );
	return 0;
}

int_t alur_rem_nodes( alu_t *alu, uint_t *nodes, int count )
{	
	if ( alu )
	{
		if ( nodes )
		{		
			for ( --count; count >= 0; --count )
			{
				alur_rem_node( alu, nodes + count );
			}
			
			return 0;
		}
		
		return alu_err_null_ptr( "nodes" );
	}
	
	return alu_err_null_ptr( "alu" );
}

int_t alur_get_nodes( alu_t *alu, uint_t *nodes, uint_t count, size_t need )
{
	uint_t index;
	
	if ( alu )
	{
		for ( index = 0; index < count; ++index )
		{		
			nodes[index] = alur_get_node( alu, need );
			count *= (nodes[index] != 0);
		}
		
		if ( !count )
		{
			alu_error( alu_errno(alu) );
			alur_rem_nodes( alu, nodes, index );
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
	alub_t c = alub( (void*)alu_valid(alu), 0 );
	
	used = LOWEST( used, ALU_REG_ID_NEED );
	
	*(c.ptr) |= c.mask;
	
	for ( r = 1, alub_inc(&c); r < used; ++r, alub_inc(&c) )
	{
		uchar_t *part = alu_data( alu, r );
		
		*(c.ptr) |= c.mask;
		
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
	, alur_t DST
	, alu_base_t base
)
{
	size_t b;
	uint_t nodes[ALU_BASE_COUNT] = {0};
	alur_t NUM, VAL;
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
		
	ret = alur_get_nodes( alu, nodes, ALU_BASE_COUNT, 0 );
	
	if ( ret != 0 )
	{
		alu_error( ret );
		return ret;
	}
	
	alur_init_unsigned( alu, NUM, nodes[ALU_BASE_NUM] );
	alur_init_unsigned( alu, VAL, nodes[ALU_BASE_VAL] );

	ret = alur_set_raw( alu, VAL, &(base.base), sizeof(size_t), 0 );
	
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
		
		ret = IFTRUE( ret != EOF, ret );
		
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
		
		alur_set_raw( alu, NUM, &b, sizeof(size_t), 0 );
		
		ret = alur_mul( alu, DST, VAL );
		
		if ( ret == EOVERFLOW )
		{
			goto maxout;
		}
		
		ret = alur_add( alu, DST, NUM );
		
		ret = IFTRUE( ret != ENODATA, ret );
	}
	while ( ret == 0 );
	
	if ( ret == EOVERFLOW )
	{
		maxout:
		alur_set_max( alu, DST );
	}
	
	fail:
	alur_rem_nodes( alu, nodes, ALU_BASE_COUNT );
	
	return ret;
}

bool alur_is_zero( alu_t *alu, alur_t reg, alub_t *end_bit )
{
	alub_t n = alur_end_bit( alu, reg );
	if ( end_bit ) *end_bit = n;
	return !(*(n.ptr) & n.mask);
}

int_t alu_lit2reg
(
	alu_t *alu
	, alu_src_t src
	, alur_t DST
	, alu_base_t base
)
{
	size_t i, Nsize, bits, _base, _one = 1;
	uint_t nodes[ALU_LIT_COUNT] = {0};
	alur_t NIL, VAL, BASE, ONE, DOT, EXP, BIAS, MAN;
	alub_t n;
	int ret;
	long pos, prevpos, exp_dig, man_dig;
	char32_t c = -1, exponent_char = 'e';
	bool neg = false, exp_neg = false, closing_bracket = false;
	void *part;
	
	if ( !(src.nextpos) )
		return EDESTADDRREQ;
	
	if ( *(src.nextpos) < 0 )
		*(src.nextpos) = 0;
	
	ret = alur_get_nodes( alu, nodes, ALU_LIT_COUNT, 0 );
	
	if ( ret != 0 )
	{
		alu_error( ret );
		return ret;
	}
	
	DST.node %= alu_used( alu );
	ret = alu_check1( alu, DST.node );
	
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
	
	alur_init_unsigned( alu, NIL, ALU_REG_ID_ZERO );
	alur_init_unsigned( alu, BASE, nodes[ALU_LIT_BASE] );
	alur_init_unsigned( alu, ONE, nodes[ALU_LIT_ONE] );
	alur_init_unsigned( alu, VAL, nodes[ALU_LIT_VAL] );
	alur_init_unsigned( alu, DOT, nodes[ALU_LIT_DOT] );
	alur_init_unsigned( alu, EXP, nodes[ALU_LIT_EXP] );
	alur_init_unsigned( alu, BIAS, nodes[ALU_LIT_EXP_BIAS] );
	alur_init_unsigned( alu, MAN, nodes[ALU_LIT_MAN] );
	
	alu_uint_set_raw( alu, BASE.node, base.base );
	
	ret = alu_str2reg( alu, src, DST, base );
	
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
		bits = DST.upto - DST.from;
		if ( bits >= (alu_Nbits(alu) / 4) )
			return EDOM;
		
		/* Check how many bits to assign to exponent & mantissa */
		man_dig = alu_man_dig( bits );
		exp_dig = ((Nsize * UNIC_CHAR_BIT) - man_dig) - 1;
		
		/* Update Exponent & Mantissa Bounds */
		alu_set_bounds( alu, &BIAS, 0, exp_dig );
		alu_set_bounds( alu, &EXP, 0, exp_dig );
		alu_set_bounds( alu, &MAN, 0, man_dig );
		
		/* Setup bias for exponent */
		alur_set_max( alu, BIAS );
		alur__shr( alu, BIAS, nodes[ALU_LIT_TMP], 1 );
		
		/* Set '0.1' */
		alu_uint_set_raw( alu, ONE.node, _one );
		
		/* Multiply '0.1' by base to get '1.0',
		 * maybe be more digits than size_t supports */
		for ( pos = prevpos; pos < *(src.nextpos); ++pos )
			(void)alur_mul( alu, ONE, BASE );
		
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
		
		if ( alur_is_zero( alu, DOT, &n ) )
		{
			pos = 0;
			alu_uint_set_raw( alu, ONE.node, _one );
		}
		
		if ( !exp_neg )
		{
			for
			(
				; !alur_is_zero( alu, EXP, NULL )
				; ++pos, (void)alur_dec( alu, EXP ) )
			{
				/* Check we haven't hit 1 */
				if ( pos == 0 || alur_is_zero( alu, ONE, &n ) )
				{
					break;
				}
				(void)alur_divide
				(
					alu
					, ONE
					, BASE
					, nodes[ALU_LIT_VAL]
					, nodes[ALU_LIT_TMP]
				);
			}
		}
		
		if ( alur_cmp( alu, EXP, BIAS ) > 0 )
		{
			if ( exp_neg )
			{
				set_nil:
				alur_clr( alu, DST );
				goto set_sign;
			}
			else
			{
				set_inf:
				alur_clr( alu, DST );
				alur_set_max( alu, EXP );
				goto set_exp;
			}
		}
		
		
		for
		(
			; alur_cmp( alu, EXP, NIL ) > 0
			; (void)alur_dec( alu, EXP )
		)
		{
			ret = alur_mul( alu, DST, BASE );
			if ( ret == EOVERFLOW )
				goto set_inf;
		}
		
		for
		(
			; alur_cmp( alu, EXP, NIL ) < 0
			; (void)alur_inc( alu, EXP )
		)
		{
			ret = alur_mul( alu, ONE, BASE );
			
			if ( ret == EOVERFLOW )
				goto set_nil;
		}
		
		if ( alur_is_zero( alu, ONE, &n ) )
			goto set_nil;
			
		(void)alur_divide
		(
			alu
			, DST
			, ONE
			, nodes[ALU_LIT_DOT]
			, nodes[ALU_LIT_TMP]
		);
		
		if ( alur_is_zero( alu, DOT, &n ) )
			alu_uint_set_raw( alu, ONE.node, _one );
		
		pos = 0;
		
		/* Calculate final exponent */
		
		if ( alur_cmp( alu, DST, ONE ) >= 0 )
		{
			for
			(
				alur_mov( alu, VAL, DST )
				; alur_cmp( alu, VAL, ONE ) > 0
				; ++pos, alur__shr( alu, VAL, nodes[ALU_LIT_TMP], 1 )
			);
		}
		else
		{
			for
			(
				alur_mov( alu, VAL, ONE )
				; alur_cmp( alu, VAL, DOT ) > 0
				; --pos, alur__shr( alu, VAL, nodes[ALU_LIT_TMP], 1 )
			);
			
			if ( alur_cmp( alu, VAL, DOT ) == 0 )
			{
				--pos;
			}
		}
		
		/* Set bias */
		alur_mov( alu, DST, BIAS );
		alu_uint_set_raw( alu, VAL.node, man_dig );
		
		
		/* Construct Mantissa */
		if ( pos > man_dig )
		{
			pos -= man_dig;
			alur_set_max( alu, DOT );
			alur__shl( alu, DOT, nodes[ALU_LIT_TMP], pos );
			alur_not( alu, DOT );
			alur_and( alu, DOT, DST );
			alur__shl( alu, ONE, nodes[ALU_LIT_TMP], pos );
			alur_mov( alu, MAN, VAL );
			alur_mov( alu, VAL, ONE );
			alur__shr( alu, VAL, nodes[ALU_LIT_TMP], 1 );
			
			i = alur_cmp( alu, DOT, VAL );
			
			if ( i > 0 )
				(void)alur_inc( alu, MAN );
			else if ( i == 0 )
			{
				switch ( FLT_ROUNDS )
				{
				case 1:
					part = alur_data( alu, MAN );
					n = alub( part, MAN.from );
					
					if ( *(n.ptr) & n.mask )
						(void)alur_inc( alu, MAN );
					
					break;
				}
			}
		}
		else
		{
			(void)alur_mov( alu, MAN, DST );	
			(void)alur_mov( alu, VAL, DOT );
			
			part = alur_data( alu, MAN );
			n = alub( part, MAN.from );
			
			for ( bits = 0; pos < man_dig; ++pos )
			{
				bits++;
				
				if ( alur_cmp( alu, VAL, ONE ) >= 0 )
				{
					(void)alur_sub( alu, VAL, ONE );
					(void)alur__shl( alu, MAN, nodes[ALU_LIT_TMP], bits );
					*(n.ptr) |= n.mask;
					bits = 0;
				}
			}
			
			if ( bits )
				(void)alur__shl( alu, MAN, nodes[ALU_LIT_TMP], bits );
		}
		
		/* TODO: Continue referencing code made in mitsy to build fpn */
		
		/* Construct FPN from modified values */
		alur_mov( alu, DST, MAN );
		
		set_exp:
		/* Align and append Exponent */
		(void)alu_set_bounds( alu, &EXP, 0, -1 );
		alur__shl( alu, EXP, nodes[ALU_LIT_TMP], man_dig );
		alur__or( alu, DST, EXP );
		
		set_sign:
		if ( neg )
		{
			alu_uint_set_raw( alu, ONE.node, _one );
			alur__shl( alu, ONE, nodes[ALU_LIT_TMP], exp_dig + man_dig );
			alur__or( alu, DST, ONE );
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
			alur_neg( alu, DST );
	}
	
	fail:
	alur_rem_nodes( alu, nodes, ALU_LIT_COUNT );
	
	return 0;
}

int_t alur2str( alu_t *alu, alu_dst_t dst, alur_t SRC, alu_base_t base )
{
	alur_t NUM, VAL, REM;
	int ret;
	size_t digit = 0, b;
	uint_t nodes[ALU_BASE_COUNT] = {0};
	bool neg;
	char *base_str =
		base.lowercase ? ALU_BASE_STR_0toztoZ : ALU_BASE_STR_0toZtoz;
	
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
	
	ret = alur_get_nodes( alu, nodes, ALU_BASE_COUNT, 0 );
	
	if ( ret == 0 )
	{	
		alur_init_unsigned( alu, NUM, nodes[ALU_BASE_NUM] );
		alur_init_unsigned( alu, VAL, nodes[ALU_BASE_VAL] );
		alur_init_unsigned( alu, REM, nodes[ALU_BASE_REM] );
		
		neg = alur_below0( alu, SRC );
		
		alur_mov( alu, NUM, SRC );
		alu_set_raw( alu, VAL.node, base.base, 0 );
		
		if ( neg )
			(void)alur_neg( alu, NUM );

		while ( alur_cmp( alu, NUM, VAL ) >= 0 )
		{	
			(void)alur_divide
			(
				alu
				, NUM
				, VAL
				, nodes[ALU_BASE_REM]
				, nodes[ALU_BASE_TMP]
			);
			
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
			
			alur_get_raw( alu, REM, &b, sizeof(size_t) );
			ret = dst.next( base_str[b], dst.dst );
			
			if ( ret != 0 )
			{
				alu_error( ret );
				goto fail;
			}
			
			++digit;
		}
		
		alur_get_raw( alu, NUM, &b, sizeof(size_t) );
		ret = dst.next( base_str[b], dst.dst );
		
		if ( ret != 0 )
		{
			alu_error( ret );
			goto fail;
		}
		
		dst.flip( dst.dst );
		
		fail:
		alur_rem_nodes( alu, nodes, ALU_BASE_COUNT );
	}
	else
	{
		alu_error( ret );
	}
	
	return ret;
}
