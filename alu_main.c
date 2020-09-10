#include "alu.h"
#include <string.h>
#include <stdio.h>

int alu_mov( alu_t *alu, uintptr_t num, uintptr_t val )
{
	alu_register_t regv[2] = {{0}}, _num, _val;
	int ret = 0;
	alu_uint_t *NUM = NULL, *VAL = NULL;
	void *part;
	
	if ( num < ALU_REG_ID_LIMIT )
	{
		num %= ALU_USED( *alu );
		_num = alu->regv[num];
	}
	else
	{
		NUM = (alu_uint_t*)num;
		
		ret = alu_get_reg( alu, &_num, NUM->vec.mem.bytes.upto );
		
		if ( ret != 0 )
			return ret;
		
		_num.info = NUM->info | ALU_INFO_VALID;
		part = ALU_PART( *alu, _num.node );
		(void)memcpy( part, NUM->vec.mem.block, NUM->vec.mem.bytes.upto );
		
		regv[0] = _num;
	}
	
	if ( val < ALU_REG_ID_LIMIT )
	{
		val %= ALU_USED( *alu );
		_val = alu->regv[val];
	}
	else
	{
		VAL = (alu_uint_t*)val;
		ret = alu_get_reg( alu, &_val, VAL->vec.mem.bytes.upto );
		
		if ( ret != 0 )
		{
			alu_rem_reg( *alu, _num );
			return ret;
		}
		
		_val.info = VAL->info | ALU_INFO_VALID;
		part = ALU_PART( *alu, _val.node );
		
		(void)memcpy( part, VAL->vec.mem.block, VAL->vec.mem.bytes.upto );
		
		regv[1] = _val;
	}
	
	alu_reg_copy( *alu, _num, _val );
	
	if ( NUM )
	{
		part = ALU_PART( *alu, _num.node );
		(void)memcpy( NUM->vec.mem.block, part, NUM->vec.mem.bytes.upto );
	}
	
	alu_rem_regv( *alu, regv, 2 );
	
	return ret;
}

int alu_check_reg( alu_t alu, uint_t reg )
{
	reg %= ALU_USED( alu );	
	return EADDRINUSE * !!( alu.regv[reg].info & ALU_INFO_VALID );
}

int alu_check1( alu_t *alu, uint_t num )
{
	return (
		alu_check_reg( *alu, num ) == EADDRINUSE
		&& num >= ALU_REG_ID_NEED
	) ? 0 : EDESTADDRREQ;
}

int alu_check2( alu_t *alu, uint_t num, uint_t val )
{
	int ret = alu_check1( alu, num );
	
	if ( ret != 0 )
	{
		alu_error( ret );
		alu_printf( "alu = %p, num = %i", alu, num );
		return ret;
	}
	
	return (
		alu_check_reg( *alu, val ) == EADDRINUSE
	) ? 0 : EADDRNOTAVAIL;
}

int alu_check3( alu_t *alu, uint_t num, uint_t val, uint_t rem )
{
	int ret = alu_check2( alu, num, val );
	
	if ( ret != 0 )
	{
		alu_error( ret );
		return ret;
	}
	
	return (
		alu_check_reg( *alu, rem ) == EADDRINUSE
		&& rem >= ALU_REG_ID_NEED
	) ? 0 : EDESTADDRREQ;
}

void alu_print_reg( char *pfx, alu_t alu, alu_register_t reg, bool print_info, bool print_value )
{
	void *part;
	alu_bit_t n;
	
	reg.node %= ALU_USED( alu );
	part = ALU_PART( alu, reg.node );
	n = alu_bit_set_bit( part, reg.upto );

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
	}
	
	if ( print_value )
	{
		fprintf( stderr, "%s = ", pfx );
		if ( n.b == reg.from )
		{
			fputc( '0', stderr );
			alu_error( ERANGE );
		}
		else
		{
			while ( n.b > reg.from )
			{
				n = alu_bit_dec(n);
				(void)fputc( '0' + !!(*(n.S) & n.B), stderr );
			}
		}
		fputc( '\n', stderr );
	}
}

size_t alu_set_bounds( alu_t alu, uint_t reg, size_t from, size_t upto )
{
	alu_register_t *REG;
	size_t full;
	reg %= ALU_USED( alu );
	
	full = alu.buff.perN * CHAR_BIT;
	upto = HIGHEST( upto, 1 );
	upto = LOWEST( upto, full );
	
	REG = alu.regv + reg;
	REG->upto = upto;
	REG->from = from * (from < upto);
	
	return upto;
}

int alu_setup_reg( alu_t *alu, uint_t want, size_t perN )
{
	int ret;
	uint_t i;
	alu_register_t *REG;
	
	want = HIGHEST( want, ALU_REG_ID_NEED );
	
	/* Needed for alu_mov() to support both register numbers and
	 * arbitrary addresses */
	if ( want < ALU_REG_ID_LIMIT )
	{
		perN =
			(sizeof(size_t) * !perN)
			| (perN * !(perN % sizeof(size_t)))
			| ((perN / sizeof(size_t))+1) * !!(perN % sizeof(size_t))
		;
		
		ret = alu_vec_expand( &(alu->buff), want, perN );
		if ( ret == 0 )
		{	
			ret = alu_vec_expand( &(alu->_regv), want, sizeof(alu_register_t) );
			if ( ret == 0 )
			{
				alu->regv = alu->_regv.mem.block;
				alu->buff.qty.used = HIGHEST( alu->buff.qty.used, ALU_REG_ID_NEED );
				alu->_regv.qty.used = HIGHEST( alu->buff.qty.used, ALU_REG_ID_NEED );
				
				for ( i = 0; i < ALU_REG_ID_NEED; ++i )
				{
					REG = alu->regv + i;
					REG->node = i;
					REG->info = ALU_INFO_VALID;
					REG->from = 0;
					REG->upto = perN * CHAR_BIT;
				}
				
				alu_set_constants( *alu );
			}
			else alu_error( ret );
		}
		else alu_error( ret );
	}
	else
	{
		ret = ERANGE;
		alu_error( ret );
	}
	
	return ret;
}

int alu_get_reg( alu_t *alu, alu_register_t *dst, size_t size )
{
	int ret;
	size_t perN;
	uint_t count = 0, r = count;
	alu_register_t *REG;

	if ( !dst )
	{
		nodst:
		ret = EDESTADDRREQ;
		alu_error( ret );
		return ret;
	}
	
	(void)memset( dst, 0, sizeof(alu_register_t) );
	
	if ( !alu )
		goto nodst;
	
	perN = HIGHEST( size, alu->buff.perN );
	count = HIGHEST( ALU_REG_ID_NEED, ALU_USED( *alu ) );
	
	if ( perN > alu->buff.perN || count > ALU_UPTO( *alu ) )
	{
		ret = alu_setup_reg( alu, count, perN );
		
		if ( ret != 0 )
		{
			alu_error( ret );
			return ret;
		}
		
		alu->buff.qty.used = count;
		alu->_regv.qty.used = count;
		alu->buff.mem.bytes.used = count * perN;
		alu->_regv.mem.bytes.used = count * sizeof(alu_register_t);
	}
	
	for ( r = ALU_REG_ID_NEED; r < count; ++r )
	{
		REG = alu->regv + r;
		count *= !!( REG->info & ALU_INFO_VALID );
	}
	
	count = alu->buff.qty.used;
	
	if ( r == ALU_UPTO( *alu ) )
	{
		count = r + 1;
		ret = alu_setup_reg( alu, count, perN );
			
		if ( ret != 0 )
		{
			alu_error( ret );
			return ret;
		}
	}
	else if ( r == count )
	{
		++count;
	}
	
	alu->buff.qty.used = count;
	alu->_regv.qty.used = count;
	alu->buff.mem.bytes.used = count * perN;
	alu->_regv.mem.bytes.used = count * sizeof(alu_register_t);
	
	REG = alu->regv + r;
	REG->node = r;
	REG->info = ALU_INFO_VALID;
	alu_set_bounds( *alu, r, 0, -1 );
	(void)memset( ALU_PART( *alu, REG->node ), 0, perN );
	(void)memcpy( dst, REG, sizeof( alu_register_t ) );
	
	return 0;
}

void alu_rem_regv( alu_t alu, alu_register_t *regv, int count )
{
	for ( --count; count >= 0; --count )
	{
		alu_rem_reg( alu, regv[count] );
	}
}

int alu_get_regv( alu_t *alu, alu_register_t *regv, uint_t count, size_t need )
{
	int ret = 0;
	uint_t r;
	alu_register_t *REG;
	
	//alu_printf( "count = %u", count );
	
	for ( r = 0; r < count; ++r )
	{
		REG = regv + r;
		ret = alu_get_reg( alu, REG, need );
		count *= (ret == 0);
		//alu_printf( "REG = %p, node = %u", REG, REG->node );
	}
	
	if ( ret != 0 )
	{
		alu_error( ret );
		alu_rem_regv( *alu, regv, r );
	}
	
	return ret;
}

void alu_rem_reg( alu_t alu, alu_register_t reg )
{
	reg.node %= ALU_USED( alu );
	if ( reg.node >= ALU_REG_ID_NEED )
		(void)memset( alu.regv + reg.node, 0, sizeof(alu_register_t) );
}

void alu_set_constants( alu_t alu )
{
	int r, used = ALU_USED( alu );
	size_t perN = alu.buff.perN, full = perN * CHAR_BIT, last = perN - 1;
	alu_register_t *REG;
	uchar_t *part;
	
	used =
		(ALU_REG_ID_NEED * (used >= ALU_REG_ID_NEED))
		| (used * (used < ALU_REG_ID_NEED));
	
	for ( r = 0; r < used; ++r )
	{
		REG = alu.regv + r;
		part = ALU_PART( alu, r );
		
		REG->node = r;
		REG->info = ALU_INFO_VALID;
		REG->from = 0;
		REG->upto = full;
		
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

int alu_str2reg
(
	alu_t *alu
	, alu_src_t src
	, alu_register_t dst
	, alu_base_t base
)
{
	size_t b, perN;
	alu_register_t
		num = base.regv[ALU_BASE_NUM]
		, val = base.regv[ALU_BASE_VAL]
		, tmp = base.regv[ALU_BASE_TMP]
	;
	alu_bit_t n;
	int ret;
	char32_t c = -1;
	char
		*base_upper = ALU_BASE_STR_0toZtoz "+-",
		*base_lower = ALU_BASE_STR_0toztoZ "+-",
		*base_str = base.lowercase ? base_lower : base_upper;
	
	if ( !(src.nextpos) )
		return EDESTADDRREQ;
	
	if ( *(src.nextpos) < 0 )
		*(src.nextpos) = 0;
	
	ret = alu_check3( alu, num.node, val.node, tmp.node );
	
	if ( ret != 0 )
		return ret;
	
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

	/* Clear all registers in direct use */
	perN = alu->buff.perN;
	
	alu_set_raw( *alu, val.node, base.base, val.info );
	
	n = alu_bit_set_bit( ALU_PART( *alu, num.node ), num.upto - 1 );
	
	alu_set_bounds( *alu, num.node, 0, -1 );
	alu_reg_set_nil( *alu, num );

	do
	{	
		/* Failsafe if failed to get character but return code is 0 */
		c = -1;
		ret = src.next( &c, src.src, src.nextpos );
		
		if ( ret != 0 && ret != EOF )
			return ret;
		
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
		
		if ( *(n.S) & n.B )
		{
			ret = alu_setup_reg(
				alu, alu->buff.qty.upto, perN + sizeof(size_t)
			);
			
			if ( ret != 0 )
				return ret;
			
			alu_set_bounds( *alu, num.node, 0, -1 );
			
			perN = alu->buff.perN;
			n = alu_bit_set_bit
			(
				ALU_PART( *alu, num.node )
				, num.upto - 1
			);
		}
		
		alu_reg_set_raw( *alu, num, b, num.info );
		
		(void)alu_reg_mul( *alu, dst, val, tmp );
		(void)alu_reg_add( *alu, dst, num );
	}
	while ( ret == 0 );
	
	return 0;
}

bool alu_reg_is_zero( alu_t alu, alu_register_t reg, alu_bit_t *end_bit )
{
	alu_bit_t n = alu_end_bit( alu, reg );
	if ( end_bit ) *end_bit = n;
	return !(*(n.S) & n.B);
}

int alu_lit2reg
(
	alu_t *alu
	, alu_src_t src
	, alu_register_t dst
	, alu_base_t base
	, alu_lit_t lit
)
{
	size_t i, perN, bits, _base;
	alu_register_t NIL, VAL, BASE, ONE, DOT, EXP, BIAS, MAN;
	alu_bit_t n;
	int ret;
	long pos, prevpos, exp_dig, man_dig;
	char32_t c = -1, exponent_char = 'e';
	bool neg = false, exp_neg = false, closing_bracket = false;
	
	if ( !(src.nextpos) )
		return EDESTADDRREQ;
	
	if ( *(src.nextpos) < 0 )
		*(src.nextpos) = 0;
	
	if ( !alu )
	{
		ret = EDESTADDRREQ;
		alu_error( ret );
		alu_print_reg( "dst", *alu, dst, true, false );
		return ret;
	}
	
	dst.node %= ALU_USED( *alu );
	ret = alu_check1( alu, dst.node );
	
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
		
	ret = src.next( &c, src.src, src.nextpos );
	if ( ret != 0 )
		return ret;
	
	if ( c == '-' || c == '+' )
	{
		if ( c == '-' )
			neg = true;
		++(*(src.nextpos));
	}
		
	ret = src.next( &c, src.src, src.nextpos );
	if ( ret != 0 )
		return ret;
		
	if ( c > '0' && c <= '9' )
	{
		base.base = 10;
		++(*(src.nextpos));
	}
	else if ( c != '0' )
		return EILSEQ;
	else
	{
		++(*(src.nextpos));
		
		ret = src.next( &c, src.src, src.nextpos );
		if ( ret != 0 )
			return ret;
		
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
					return ret;
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
					return ret;
				if ( !base.base )
				{
					ret = EILSEQ;
					alu_error(ret);
					alu_puts( "Invalid base provided" );
					return ret;
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
				return ret;
			}
		}
	}
	
	NIL = alu->regv[ALU_REG_ID_ZERO];
	BASE = base.regv[ALU_BASE_VAL];
	ONE = lit.regv[ALU_LIT_ONE];
	VAL = lit.regv[ALU_LIT_VAL];
	DOT = lit.regv[ALU_LIT_DOT];
	EXP = lit.regv[ALU_LIT_EXP];
	BIAS = lit.regv[ALU_LIT_EXP_BIAS];
	MAN = lit.regv[ALU_LIT_MAN];
	
	alu_reg_set_raw( *alu, BASE, base.base, BASE.info );
	
	ret = alu_str2reg( alu, src, dst, base );
	
	switch ( ret )
	{
	case 0: case EOF: c = 0; break;
	default: return ret;
	}

	perN = alu->buff.perN;
	
	/* Check if reading a floating point number */
	if ( c == '.' )
	{
		++(*(src.nextpos));
		prevpos = *(src.nextpos);
		
		ret = alu_str2reg( alu, src, DOT, base );
		
		switch ( ret )
		{
		case 0: case EOF: c = 0; break;
		default: return ret;
		}
		
		/* Make sure have enough space for later calculations */
		bits = dst.upto - dst.from;
		if ( bits == (perN * CHAR_BIT) )
		{
			ret = alu_setup_reg( alu, alu->buff.qty.upto, perN * 2 );
			if ( ret != 0 )
			{
				alu_error(ret);
				return ret;
			}
			
			perN = alu->buff.perN;
		}
		
		/* Check how many bits to assign to exponent & mantissa */
		if ( bits < bitsof(float) )
		{
			if ( perN > 1 )
				man_dig = CHAR_BIT - 1;
			else
				man_dig = (CHAR_BIT / 2) - 1;
		}
		else if ( bits < bitsof(double) )
			man_dig = FLT_MANT_DIG;
		else if ( bits < bitsof(long double) )
			man_dig = DBL_MANT_DIG;
		else if ( bits == bitsof(long double) )
			man_dig = LDBL_MANT_DIG;
		else
			man_dig = (bits / 4) - 1;
		
		exp_dig = ((perN * CHAR_BIT) - man_dig) - 1;
		
		/* Update Exponent & Mantissa Bounds */
		alu_set_bounds( *alu, BIAS.node, 0, exp_dig );
		alu_set_bounds( *alu, EXP.node, 0, exp_dig );
		alu_set_bounds( *alu, MAN.node, 0, man_dig );
		
		/* Setup bias for exponent */
		alu_reg_set_max( *alu, BIAS );
		alu_reg__shr( *alu, BIAS, 1 );
		
		/* Set '0.1' */
		alu_reg_set_raw( *alu, ONE, 1, 0 );
		
		/* Multiply '0.1' by base to get '1.0',
		 * maybe be more digits than size_t supports */
		for ( pos = prevpos; pos < *(src.nextpos); ++pos )
			(void)alu_reg_mul( *alu, ONE, BASE, base.regv[ALU_BASE_TMP] );
		
		/* Only universal number literals need this, example of one:
		 * 0~L36(A.z)e+10 */
		if ( closing_bracket )
		{
			if ( c == ')' )
			{
				++(*(src.nextpos));
				ret = src.next( &c, src.src, src.nextpos );
				if ( ret != 0 && ret != EOF )
					return ret;
			}
			else
				return EILSEQ;
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
			
			alu_reg_set_raw( *alu, BASE, 10, BASE.info );
			_base = base.base;
			base.base = 10;
			
			ret = alu_str2reg( alu, src, lit.regv[ALU_LIT_EXP], base );
			
			alu_reg_set_raw( *alu, BASE, _base, BASE.info );
			base.base = _base;
			
			switch ( ret )
			{
			case 0: case EOF: c = 0; break;
			default: return ret;
			}
		}
		
		/* Begin Constructing float */
		
		if ( alu_reg_is_zero( *alu, DOT, &n ) )
		{
			pos = 0;
			alu_reg_set_raw( *alu, ONE, 1, 0 );
		}
		
		if ( !exp_neg )
		{
			for
			(
				; !alu_reg_is_zero( *alu, lit.regv[ALU_LIT_EXP], NULL )
				; ++pos, (void)alu_reg_dec( *alu, lit.regv[ALU_LIT_EXP] ) )
			{
				/* Check we haven't hit 1 */
				if ( pos == 0
					|| alu_reg_is_zero( *alu, ONE, &n )
				)
				{
					break;
				}
				(void)alu_reg_divide( *alu, ONE, BASE, VAL );
			}
		}
		
		if ( alu_reg_cmp( *alu, EXP, BIAS, NULL ) > 0 )
		{
			if ( exp_neg )
			{
				set_nil:
				alu_reg_set_nil( *alu, dst );
				goto set_sign;
			}
			else
			{
				set_inf:
				alu_reg_set_nil( *alu, dst );
				alu_reg_set_max( *alu, EXP );
				goto set_exp;
			}
		}
		
		
		for
		(
			; alu_reg_cmp( *alu, EXP, NIL, NULL ) > 0
			; (void)alu_reg_dec( *alu, EXP )
		)
		{
			ret = alu_reg_mul( *alu, dst, BASE, base.regv[ALU_BASE_TMP] );
			if ( ret == EOVERFLOW )
				goto set_inf;
		}
		
		for
		(
			; alu_reg_cmp( *alu, EXP, NIL, NULL ) < 0
			; (void)alu_reg_inc( *alu, EXP )
		)
		{
			ret = alu_reg_mul( *alu, ONE, BASE, base.regv[ALU_BASE_TMP] );
			
			if ( ret == EOVERFLOW )
				goto set_nil;
		}
		
		if ( alu_reg_is_zero( *alu, ONE, &n ) )
			goto set_nil;
			
		(void)alu_reg_divide( *alu, dst, ONE, DOT );
		
		if ( alu_reg_is_zero( *alu, DOT, &n ) )
			alu_reg_set_raw( *alu, ONE, 1, 0 );
		
		pos = 0;
		
		/* Calculate final exponent */
		
		if ( alu_reg_cmp( *alu, dst, ONE, NULL ) >= 0 )
		{
			for
			(
				alu_reg_copy( *alu, VAL, dst )
				; alu_reg_cmp( *alu, VAL, ONE, NULL ) > 0
				; ++pos, alu_reg__shr( *alu, VAL, 1 )
			);
		}
		else
		{
			for
			(
				alu_reg_copy( *alu, VAL, ONE )
				; alu_reg_cmp( *alu, VAL, DOT, NULL ) > 0
				; --pos, alu_reg__shr( *alu, VAL, 1 )
			);
			
			if ( alu_reg_cmp( *alu, VAL, DOT, NULL ) == 0 )
			{
				--pos;
			}
		}
		
		/* Set bias */
		alu_reg_copy( *alu, dst, BIAS );
		alu_reg_set_raw( *alu, VAL, man_dig, 0 );
		
		
		/* Construct Mantissa */
		if ( pos > man_dig )
		{
			pos -= man_dig;
			alu_reg_set_max( *alu, DOT );
			alu_reg__shl( *alu, DOT, pos );
			alu_reg_not( *alu, DOT );
			alu_reg_and( *alu, DOT, dst );
			alu_reg__shl( *alu, ONE, pos );
			alu_reg_copy( *alu, MAN, VAL );
			alu_reg_copy( *alu, VAL, ONE );
			alu_reg__shr( *alu, VAL, 1 );
			
			i = alu_reg_cmp( *alu, DOT, VAL, NULL );
			
			if ( i > 0 )
				(void)alu_reg_inc( *alu, MAN );
			else if ( i == 0 )
			{
				switch ( FLT_ROUNDS )
				{
				case 1:
					n = alu_bit_set_bit
					(
						ALU_PART( *alu, MAN.node )
						, MAN.from
					);
					if ( *(n.S) & n.B )
						(void)alu_reg_inc( *alu, MAN );
					break;
				}
			}
		}
		else
		{
			(void)alu_reg_copy( *alu, MAN, dst );	
			(void)alu_reg_copy( *alu, VAL, DOT );
			
			n = alu_bit_set_bit
			(
				ALU_PART( *alu, MAN.node )
				, MAN.from
			);
			for ( bits = 0; pos < man_dig; ++pos )
			{
				bits++;
				
				if ( alu_reg_cmp( *alu, VAL, ONE, NULL ) >= 0 )
				{
					(void)alu_reg_sub( *alu, VAL, ONE );
					(void)alu_reg__shl( *alu, MAN, bits );
					*(n.S) |= n.B;
					bits = 0;
				}
			}
			
			if ( bits )
				(void)alu_reg__shl( *alu, MAN, bits );
		}
		
		/* TODO: Continue referencing code made in mitsy to build fpn */
		
		/* Construct FPN from modified values */
		alu_reg_copy( *alu, dst, MAN );
		
		set_exp:
		/* Align and append Exponent */
		(void)alu_set_bounds( *alu, EXP.node, 0, -1 );
		alu_reg__shl( *alu, EXP, man_dig );
		alu_reg__or( *alu, dst, EXP );
		
		set_sign:
		if ( neg )
		{
			alu_reg_set_raw( *alu, ONE, 1, 0 );
			alu_reg__shl( *alu, ONE, exp_dig + man_dig );
			alu_reg__or( *alu, dst, ONE );
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
					return ret;
			}
			else
			{
				ret = EILSEQ;
				alu_error(ret);
				alu_puts( "Expected ')' to close Exponent or Float" );
				alu_printf( "Current character is '%c'", c );
				return ret;
			}
		}
		
		if ( neg )
			alu_reg_neg( *alu, dst );
	}
	
	return 0;
}

int alu_reg2str
(
	alu_t *alu
	, alu_dst_t dst
	, alu_register_t src
	, alu_base_t base
)
{
	alu_register_t num, div, val;
	alu_bit_t l;
	int ret;
	size_t *V, *N, digit = 0;
	bool neg;
	char *base_str =
		base.lowercase ? ALU_BASE_STR_0toztoZ : ALU_BASE_STR_0toZtoz;
		
	ret = alu_check1( alu, src.node );
	
	if ( ret != 0 )
		return ret;
	
	l = alu_bit_set_bit( ALU_PART( *alu, src.node ), src.upto - 1 );
	neg = !!(src.info & ALU_INFO__SIGN) & !!(*(l.S) & l.B);
	
	if ( !(dst.next) || !(dst.flip) )
		return EADDRNOTAVAIL;
		
	if ( !base.base )
		base.base = 10;
	
	if ( base.base > strlen(base_str) )
		return ERANGE;
	
	num = base.regv[ALU_BASE_NUM];
	div = base.regv[ALU_BASE_VAL];
	val = base.regv[ALU_BASE_TMP];
	
	alu_reg_copy( *alu, num, src );
	
	alu_reg_set_raw( *alu, div, base.base, div.info );
	
	if ( neg )
		(void)alu_reg_neg( *alu, num );
	
	N = ALU_PART( *alu, num.node );
	V = ALU_PART( *alu, val.node );
	
	switch ( base.base )
	{
	case 2: case 8: case 10: case 16: break;
	default:
		ret = dst.next( ')', dst.dst );
		if ( ret != 0 )
			return ret;
	}

	do
	{
		(void)alu_reg_divide( *alu, num, div, val );
		
		if ( digit == 3 )
		{
			switch ( base.digsep )
			{
			case '\'': case '_': case ',':
				ret = dst.next( base_str[*V], dst.dst );
				if ( ret != 0 )
					return ret;
			}
			digit = 0;
		}
		
		ret = dst.next( base_str[*V], dst.dst );
		
		if ( ret != 0 )
			return ret;
		
		++digit;
	}
	while ( alu_reg_cmp( *alu, num, div, NULL ) >= 0 );
	
	ret = dst.next( base_str[*N], dst.dst );
	
	if ( ret != 0 )
		return ret;
		
	if ( base.base != 10 )
	{
		if ( base.base == 2 )
		{
			ret = dst.next( 'b', dst.dst );
			if ( ret != 0 )
				return ret;
			
			ret = dst.next( '0', dst.dst );
			if ( ret != 0 )
				return ret;
		}
		else if ( base.base == 8 )
		{
			ret = dst.next( 'o', dst.dst );
			if ( ret != 0 )
				return ret;
			
			ret = dst.next( '0', dst.dst );
			if ( ret != 0 )
				return ret;
		}
		else if ( base.base == 16 )
		{
			ret = dst.next( 'x', dst.dst );
			if ( ret != 0 )
				return ret;
			
			ret = dst.next( '0', dst.dst );
			if ( ret != 0 )
				return ret;
		}
		else
		{
			ret = dst.next( '(', dst.dst );
			if ( ret != 0 )
				return ret;
			
			while ( base.base > 10 )
			{
				ret = dst.next( base_str[base.base%10], dst.dst );
				if ( ret != 0 )
					return ret;
				base.base /= 10;
			}
			
			ret = dst.next( base_str[base.base], dst.dst );
			if ( ret != 0 )
				return ret;
			
			ret = dst.next( '~', dst.dst );
			if ( ret != 0 )
				return ret;
			
			ret = dst.next( '0', dst.dst );
			if ( ret != 0 )
				return ret;
		}
	}
	
	dst.flip( dst.dst );
	
	return ret;
}
