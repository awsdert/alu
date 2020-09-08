#include "alu.h"
#include <string.h>
#include <stdio.h>

int alu_mov( alu_t *alu, uintptr_t num, uintptr_t val )
{
	alu_register_t regv[2] = {{0}}, _num, _val;
	int ret = 0;
	alu_uint_t *NUM = NULL, *VAL = NULL;
	void *part;
	
	if ( num < ALU_USED( *alu ) )
		_num = alu->regv[num];
	else
	{
		NUM = (alu_uint_t*)num;
		ret = alu_get_reg( alu, regv, NUM->vec.mem.bytes.upto );
		
		if ( ret != 0 )
			return ret;
			
		_num = regv[0];
		_num.info = NUM->info | ALU_INFO_VALID;
		part = ALU_PART( *alu, _num.node );
		(void)memcpy( part, NUM->vec.mem.block, NUM->vec.mem.bytes.upto );
	}
	
	if ( val < ALU_USED( *alu ) )
		_val = alu->regv[val];
	else
	{
		VAL = (alu_uint_t*)val;
		ret = alu_get_reg( alu, regv + 1, VAL->vec.mem.bytes.upto );
		if ( ret != 0 )
		{
			alu_rem_reg( *alu, *regv );
			return ret;
		}
		
		_val = regv[0];
		_val.info = VAL->info | ALU_INFO_VALID;
		part = ALU_PART( *alu, _val.node );
		(void)memcpy( part, VAL->vec.mem.block, VAL->vec.mem.bytes.upto );
	}
	
	(void)alu_reg_copy( *alu, _num, _val );
	
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
	if ( want > ALU_REG_ID_LIMIT )
		return ERANGE;
	
	perN =
		(sizeof(size_t) * !perN)
		| (perN * !(perN % sizeof(size_t)))
		| ((perN / sizeof(size_t))+1) * !!(perN % sizeof(size_t))
	;
	
	ret = alu_vec_expand( &(alu->buff), want, perN );
	if ( ret != 0 )
		return ret;
		
	ret = alu_vec_expand( &(alu->_regv), want, sizeof(alu_reg_t) );
	if ( ret != 0 )
		return ret;
	
	alu->regv = alu->_regv.mem.block;
	alu->buff.qty.used = HIGHEST( alu->buff.qty.used, ALU_REG_ID_NEED );
	alu->_regv.qty.used = HIGHEST( alu->buff.qty.used, ALU_REG_ID_NEED );
	
	for ( i = 0; i < ALU_REG_ID_NEED; ++i )
	{
		REG = alu->regv + i;
		REG->node = i;
		REG->info = ALU_INFO_VALID;
		alu_set_bounds( *alu, i, 0, -1 );
	}
	
	alu_set_constants( *alu );
	
	return 0;
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

int alu_get_regv( alu_t *alu, alu_register_t *regv, int count, size_t need )
{
	int ret = 0, r;
	
	for ( r = 0; r < count; ++r )
	{
		ret = alu_get_reg( alu, regv + r, need );
		if ( ret != 0 )
		{
			alu_error( ret );
			alu_rem_regv( *alu, regv, r );
			break;
		}
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
	alu_t *alu,
	void *src,
	alu_register_t reg_dst,
	alu_register_t reg_base,
	alu_register_t reg_mod,
	char32_t digsep,
	alu_func_rdChar32_t nextchar,
	long *nextpos,
	size_t base,
	bool lowercase
)
{
	size_t b, perN;
	alu_register_t reg_tmp;
	alu_bit_t n;
	int ret;
	char32_t *M, c = -1;
	char
		*base_upper = ALU_BASE_STR_0toZtoz "+-",
		*base_lower = ALU_BASE_STR_0toztoZ "+-",
		*base_str = lowercase ? base_lower : base_upper;
	
	if ( !nextpos )
		return EDESTADDRREQ;
	
	if ( *nextpos < 0 )
		*nextpos = 0;
	
	ret = alu_check3( alu, reg_dst.node, reg_base.node, reg_mod.node );
	
	if ( ret != 0 )
		return ret;
	
	if ( !nextchar )
		return EADDRNOTAVAIL;
	
	switch ( digsep )
	{
	case 0: case '\'': case '_': case ',': break;
	default: return EINVAL;
	}
	
	if ( !base || base > strlen(base_str) )
		return ERANGE;
	
	ret = alu_get_reg( alu, &reg_tmp, sizeof(size_t) );
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}

	/* Clear all registers in direct use */
	perN = alu->buff.perN;
	
	M = ALU_PART( *alu, reg_base.node );
	*M = base;
	
	M = ALU_PART( *alu, reg_mod.node );
	n = alu_bit_set_bit( ALU_PART( *alu, reg_dst.node ), reg_dst.upto - 1 );
	
	alu_set_bounds( *alu, reg_dst.node, 0, -1 );
	alu_reg_set_nil( *alu, reg_dst );

	do
	{	
		/* Failsafe if failed to get character but return code is 0 */
		c = -1;
		ret = nextchar( &c, src, nextpos );
		
		if ( ret != 0 && ret != EOF )
		{
			alu_rem_reg( *alu, reg_tmp );
			return ret;
		}
		
		if ( base > 36 )
		{
			for ( b = 0; b < base; ++b )
			{
				if ( c == (char32_t)(base_str[b]) )
					break;
			}
		}
		else
		{
			for ( b = 0; b < base; ++b )
			{
				if ( c == (char32_t)(base_upper[b]) )
					break;
			}
			
			for ( b = 0; b < base; ++b )
			{
				if ( c == (char32_t)(base_lower[b]) )
					break;
			}
		}
		
		if ( b == base )
		{
			if ( c == digsep )
			{
				++(*nextpos);
				continue;
			}
			break;
		}
		
		++(*nextpos);
		
		if ( *(n.S) & n.B )
		{
			ret = alu_setup_reg(
				alu, alu->buff.qty.upto, perN + sizeof(size_t)
			);
			
			if ( ret != 0 )
				return ret;
			
			alu_set_bounds( *alu, reg_dst.node, 0, -1 );
			
			perN = alu->buff.perN;
			M = ALU_PART( *alu, reg_mod.node );
			n = alu_bit_set_bit
			(
				ALU_PART( *alu, reg_dst.node )
				, reg_dst.upto - 1
			);
		}
		
		*M = b;
		
		(void)alu_reg_mul( *alu, reg_dst, reg_base, reg_tmp );
		(void)alu_reg_add( *alu, reg_dst, reg_mod );
	}
	while ( ret == 0 );
	
	alu_rem_reg( *alu, reg_tmp );
	
	return 0;
}

enum
{
	ALU_L2R_BASE = 0,
	ALU_L2R_NUM,
	ALU_L2R_DOT,
	ALU_L2R_ONE,
	ALU_L2R_EXP,
	ALU_L2R_EXP_BIAS,
	ALU_L2R_MAN,
	ALU_L2R_VAL,
	ALU_L2R_TMP,
	ALU_L2R_COUNT
};

bool alu_reg_is_zero( alu_t alu, alu_register_t reg, alu_bit_t *end_bit )
{
	alu_bit_t n = alu_end_bit( alu, reg );
	if ( end_bit ) *end_bit = n;
	return !(*(n.S) & n.B);
}

int alu_lit2reg
(
	alu_t *alu,
	void *src,
	alu_register_t dst,
	char32_t digsep,
	alu_func_rdChar32_t nextchar,
	long *nextpos,
	bool lowercase
)
{
	size_t i, base, perN, bits, *BASE;
	alu_register_t regv[ALU_L2R_COUNT], NIL;
	alu_bit_t n;
	int ret;
	long pos, prevpos, exp_dig, man_dig;
	char32_t c = -1, exponent_char = 'e';
	bool neg = false, exp_neg = false, closing_bracket = true;
	
	if ( !nextpos )
		return EDESTADDRREQ;
	
	if ( *nextpos < 0 )
		*nextpos = 0;
		
	ret = alu_check1( alu, dst.node );
	
	if ( ret != 0 )
	{
		alu_error( ret );
		alu_print_reg( "dst", *alu, dst, true, false );
		return ret;
	}
	
	if ( !nextchar )
		return EADDRNOTAVAIL;
	
	switch ( digsep )
	{
	case 0: case '\'': case '_': case ',': break;
	default: return EINVAL;
	}
		
	ret = nextchar( &c, src, nextpos );
	if ( ret != 0 )
		return ret;
	
	if ( c == '-' || c == '+' )
	{
		if ( c == '-' )
			neg = true;
		++(*nextpos);
	}
		
	ret = nextchar( &c, src, nextpos );
	if ( ret != 0 )
		return ret;
		
	if ( c > '0' && c <= '9' )
	{
		base = 10;
		++(*nextpos);
	}
	else if ( c != '0' )
		return EINVAL;
	else
	{
		++(*nextpos);
		
		ret = nextchar( &c, src, nextpos );
		if ( ret != 0 )
			return ret;
		
		if ( c >= '0' && c <= '7' )
		{
			++(*nextpos);
			base = 8;
		}
		else
		{
			switch ( c )
			{
			case '~':
				base = 0;
				++(*nextpos);
				ret = nextchar( &c, src, nextpos );
				if ( ret != 0 )
					return ret;
				if ( c == 'l' || c == 'L' )
				{
					++(*nextpos);
					lowercase = true;
				}
				while ( 1 )
				{
					ret = nextchar( &c, src, nextpos );
					if ( ret != 0 || c <= '0' || c >= '9' )
						break;
					
					base *= 10;
					base += (c - '0');
					++(*nextpos);
				}
				if ( ret != 0 && ret != EOF )
					return ret;
				if ( !base )
					return EINVAL;
				break;
			case 'b': case 'B': base = 2; ++(*nextpos); break;
			case 'o': case 'O': base = 8; ++(*nextpos); break;
			case 'x': case 'X':
				base = 16;
				++(*nextpos);
				exponent_char = 'p';
				break;
			default: return EINVAL;
			}
		}
	}
	
	ret = alu_get_regv( alu, regv, ALU_L2R_COUNT, sizeof(size_t) );
	
	if ( ret != 0 )
	{
		alu_error( ret );
		return ret;
	}
	
	
	NIL = alu->regv[ALU_REG_ID_ZERO];
	
	BASE = ALU_PART( *alu, regv[ALU_L2R_BASE].node );
	*BASE = base;
	
	ret = alu_str2reg(
		alu,
		src,
		dst,
		regv[ALU_L2R_BASE],
		regv[ALU_L2R_VAL],
		digsep,
		nextchar,
		nextpos,
		base,
		lowercase
	);
	
	switch ( ret )
	{
	case 0: case EOF: c = 0; break;
	case EINVAL:
		(void)nextchar( &c, src, nextpos );
		break;
	default: return ret;
	}

	perN = alu->buff.perN;
	
	/* Check if reading a floating point number */
	if ( c == '.' )
	{
		++(*nextpos);
		prevpos = *nextpos;
		
		ret = alu_str2reg(
			alu,
			src,
			regv[ALU_L2R_DOT],
			regv[ALU_L2R_BASE],
			regv[ALU_L2R_VAL],
			digsep,
			nextchar,
			nextpos,
			base,
			lowercase
		);
		
		switch ( ret )
		{
		case 0: case EOF: c = 0; break;
		case EINVAL:
			(void)nextchar( &c, src, nextpos );
			break;
		default:
			return ret;
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
			BASE = ALU_PART( *alu, regv[ALU_L2R_BASE].node );
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
		alu_set_bounds( *alu, regv[ALU_L2R_EXP_BIAS].node, 0, exp_dig );
		alu_set_bounds( *alu, regv[ALU_L2R_EXP].node, 0, exp_dig );
		alu_set_bounds( *alu, regv[ALU_L2R_MAN].node, 0, man_dig );
		
		/* Setup bias for exponent */
		alu_reg_set_max( *alu, regv[ALU_L2R_EXP_BIAS] );
		alu_reg__shr( *alu, regv[ALU_L2R_EXP_BIAS], 1 );
		
		/* Set '0.1' */
		alu_reg_set_raw( *alu, regv[ALU_L2R_ONE], 1, 0 );
		
		/* Multiply '0.1' by base to get '1.0',
		 * maybe be more digits than size_t supports */
		for ( pos = prevpos; pos < *nextpos; ++pos )
			(void)alu_reg_mul( *alu, regv[ALU_L2R_ONE], regv[ALU_L2R_BASE], regv[ALU_L2R_TMP] );
		
		/* Only universal number literals need this, example of one:
		 * 0~L36(A.z)e+10 */
		if ( closing_bracket )
		{
			if ( c == ')' )
			{
				++(*nextpos);
				ret = nextchar( &c, src, nextpos );
				if ( ret != 0 && ret != EOF )
					return ret;
			}
			else
				return EINVAL;
		}
		
		if ( c == exponent_char || c == (char32_t)toupper(exponent_char) )
		{
			++(nextpos);
			ret = nextchar( &c, src, nextpos );
			if ( ret != 0 && ret != EOF )
				return ret;
			
			if ( c == '-' || c == '+' )
			{
				if ( c == '-' )
					exp_neg = true;
				++(nextpos);
			}
			
			*BASE = 10;
			ret = alu_str2reg(
				alu,
				src,
				regv[ALU_L2R_EXP],
				regv[ALU_L2R_BASE],
				regv[ALU_L2R_VAL],
				digsep,
				nextchar,
				nextpos,
				10,
				lowercase
			);
			*BASE = base;
			
			switch ( ret )
			{
			case 0: case EOF: c = 0; break;
			case EINVAL:
				(void)nextchar( &c, src, nextpos );
				break;
			default:
				return ret;
			}
		}
		
		/* Begin Constructing float */
		
		if ( alu_reg_is_zero( *alu, regv[ALU_L2R_DOT], &n ) )
		{
			pos = 0;
			alu_reg_set_raw( *alu, regv[ALU_L2R_ONE], 1, 0 );
		}
		
		if ( !exp_neg )
		{
			for
			(
				; !alu_reg_is_zero( *alu, regv[ALU_L2R_EXP], NULL )
				; ++pos, (void)alu_reg_dec( *alu, regv[ALU_L2R_EXP] ) )
			{
				/* Check we haven't hit 1 */
				if ( pos == 0
					|| alu_reg_is_zero( *alu, regv[ALU_L2R_ONE], &n )
				)
				{
					break;
				}
				(void)alu_reg_divide(
					*alu
					, regv[ALU_L2R_ONE]
					, regv[ALU_L2R_BASE]
					, regv[ALU_L2R_VAL]
				);
			}
		}
		
		if
		(
			alu_reg_cmp
			(
				*alu
				, regv[ALU_L2R_EXP]
				, regv[ALU_L2R_EXP_BIAS]
				, NULL
			) > 0
		)
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
				alu_reg_set_max( *alu, regv[ALU_L2R_EXP] );
				goto set_exp;
			}
		}
		
		
		for
		(
			; alu_reg_cmp( *alu, regv[ALU_L2R_EXP], NIL, NULL ) > 0
			; (void)alu_reg_dec( *alu, regv[ALU_L2R_EXP] )
		)
		{
			ret = alu_reg_mul( *alu, dst, regv[ALU_L2R_BASE], regv[ALU_L2R_TMP] );
			if ( ret == EOVERFLOW )
				goto set_inf;
		}
		
		for
		(
			; alu_reg_cmp( *alu, regv[ALU_L2R_EXP], NIL, NULL ) < 0
			; (void)alu_reg_inc( *alu, regv[ALU_L2R_EXP] )
		)
		{
			ret = alu_reg_mul(
				*alu
				, regv[ALU_L2R_ONE]
				, regv[ALU_L2R_BASE]
				, regv[ALU_L2R_TMP]
			);
			
			if ( ret == EOVERFLOW )
				goto set_nil;
		}
		
		if ( alu_reg_is_zero( *alu, regv[ALU_L2R_ONE], &n ) )
			goto set_nil;
			
		(void)alu_reg_divide(
			*alu,
			dst,
			regv[ALU_L2R_ONE],
			regv[ALU_L2R_DOT]
		);
		
		if ( alu_reg_is_zero( *alu, regv[ALU_L2R_DOT], &n ) )
			alu_reg_set_raw( *alu, regv[ALU_L2R_ONE], 1, 0 );
		
		pos = 0;
		
		/* Calculate final exponent */
		
		if ( alu_reg_cmp( *alu, dst, regv[ALU_L2R_ONE], NULL ) >= 0 )
		{
			for
			(
				alu_mov( alu, regv[ALU_L2R_VAL].node, dst.node )
				; alu_reg_cmp(
					*alu
					, regv[ALU_L2R_VAL]
					, regv[ALU_L2R_ONE]
					, NULL
				) > 0
				; ++pos, alu_reg__shr( *alu, regv[ALU_L2R_VAL], 1 )
			);
		}
		else
		{
			for
			(
				alu_mov
				(
					alu
					, regv[ALU_L2R_VAL].node
					, regv[ALU_L2R_ONE].node
				)
				; alu_reg_cmp
				(
					*alu
					, regv[ALU_L2R_VAL]
					, regv[ALU_L2R_DOT]
					, NULL
				) > 0
				; --pos, alu_reg__shr( *alu, regv[ALU_L2R_VAL], 1 )
			);
			if
			(
				alu_reg_cmp
				(
					*alu
					, regv[ALU_L2R_VAL]
					, regv[ALU_L2R_DOT]
					, NULL
				) == 0
			)
			{
				--pos;
			}
		}
		
		/* Set bias */
		alu_mov( alu, dst.node, regv[ALU_L2R_EXP_BIAS].node );
		alu_reg_set_raw( *alu, regv[ALU_L2R_VAL], man_dig, 0 );
		
		
		/* Construct Mantissa */
		if ( pos > man_dig )
		{
			pos -= man_dig;
			alu_reg_set_max( *alu, regv[ALU_L2R_DOT] );
			alu_reg__shl( *alu, regv[ALU_L2R_DOT], pos );
			alu_reg_not( *alu, regv[ALU_L2R_DOT] );
			alu_reg_and( *alu, regv[ALU_L2R_DOT], dst );
			alu_reg__shl( *alu, regv[ALU_L2R_ONE], pos );
			(void)alu_mov( alu, regv[ALU_L2R_MAN].node, regv[ALU_L2R_VAL].node );
			(void)alu_mov( alu, regv[ALU_L2R_VAL].node, regv[ALU_L2R_ONE].node );
			alu_reg__shr( *alu, regv[ALU_L2R_VAL], 1 );
			
			i = alu_reg_cmp
			(
				*alu
				, regv[ALU_L2R_DOT]
				, regv[ALU_L2R_VAL]
				, NULL
			);
			
			if ( i > 0 )
				(void)alu_reg_inc( *alu, regv[ALU_L2R_MAN] );
			else if ( i == 0 )
			{
				switch ( FLT_ROUNDS )
				{
				case 1:
					n = alu_bit_set_bit
					(
						ALU_PART( *alu, regv[ALU_L2R_MAN].node )
						, regv[ALU_L2R_MAN].from
					);
					if ( *(n.S) & n.B )
						(void)alu_reg_inc( *alu, regv[ALU_L2R_MAN] );
					break;
				}
			}
		}
		else
		{
			(void)alu_mov
			(
				alu
				, regv[ALU_L2R_MAN].node
				, dst.node
			);
			
			(void)alu_mov
			(
				alu
				, regv[ALU_L2R_VAL].node
				, regv[ALU_L2R_DOT].node
			);
			
			n = alu_bit_set_bit
			(
				ALU_PART( *alu, regv[ALU_L2R_MAN].node )
				, regv[ALU_L2R_MAN].from
			);
			for ( bits = 0; pos < man_dig; ++pos )
			{
				bits++;
				if
				(
					alu_reg_cmp
					(
						*alu
						, regv[ALU_L2R_VAL]
						, regv[ALU_L2R_ONE]
						, NULL
					) >= 0
				)
				{
					(void)alu_reg_sub
					(
						*alu
						, regv[ALU_L2R_VAL]
						, regv[ALU_L2R_ONE]
					);
					(void)alu_reg__shl( *alu, regv[ALU_L2R_MAN], bits );
					*(n.S) |= n.B;
					bits = 0;
				}
			}
			
			if ( bits )
				(void)alu_reg__shl( *alu, regv[ALU_L2R_MAN], bits );
		}
		
		/* TODO: Continue referencing code made in mitsy to build fpn */
		
		/* Construct FPN from modified values */
		alu_mov( alu, dst.node, regv[ALU_L2R_MAN].node );
		set_exp:
		/* Align and append Exponent */
		(void)alu_set_bounds( *alu, regv[ALU_L2R_EXP].node, 0, -1 );
		alu_reg__shl( *alu, regv[ALU_L2R_EXP], man_dig );
		alu_reg__or( *alu, dst, regv[ALU_L2R_EXP] );
		set_sign:
		if ( neg )
		{
			alu_reg_set_raw( *alu, regv[ALU_L2R_ONE], 1, 0 );
			alu_reg__shl( *alu, regv[ALU_L2R_ONE], exp_dig + man_dig );
			alu_reg__or( *alu, dst, regv[ALU_L2R_ONE] );
		}
	}
	else 
	{
		if ( closing_bracket )
		{
			if ( c == ')' )
			{
				++(*nextpos);
				ret = nextchar( &c, src, nextpos );
				if ( ret != 0 && ret != EOF )
					return ret;
			}
			else
				return EINVAL;
		}
		if ( neg )
			alu_reg_neg( *alu, dst );
	}
	
	
	alu_rem_regv( *alu, regv, ALU_L2R_COUNT );
	
	return 0;
}

int alu_reg2str
(
	alu_t *alu,
	void *dst,
	alu_register_t src,
	char32_t digsep,
	alu_func_wrChar32_t nextchar,
	alu_func_flipstr_t flipstr,
	size_t base,
	bool lowercase,
	bool noPfx,
	bool noSign
)
{
	alu_register_t regv[3], num, div, val;
	alu_bit_t l;
	int ret;
	size_t *V, *N, digit = 0;
	bool neg;
	char *base_str = lowercase ? ALU_BASE_STR_0toztoZ : ALU_BASE_STR_0toZtoz;
		
	ret = alu_check1( alu, src.node );
	
	if ( ret != 0 )
		return ret;
	
	l = alu_bit_set_bit( ALU_PART( *alu, src.node ), src.upto - 1 );
	neg = !!(src.info & ALU_INFO__SIGN) & !!(*(l.S) & l.B);
	
	if ( neg && noSign )
		return EINVAL;
	
	if ( !nextchar || !flipstr )
		return EADDRNOTAVAIL;
		
	if ( !base )
		base = 10;
	
	if ( base > strlen(base_str) )
		return ERANGE;
	
	ret = alu_get_regv( alu, regv, 3, sizeof(size_t) );
	
	if ( ret != 0 )
		return ret;
	
	num = regv[0];
	val = regv[1];
	div = regv[2];
	
	(void)alu_mov( alu, num.node, src.node );
	
	V = ALU_PART( *alu, div.node );
	*V = base;
	
	if ( neg )
		(void)alu_reg_neg( *alu, num );
	
	N = ALU_PART( *alu, num.node );
	V = ALU_PART( *alu, val.node );
	
	if ( !noPfx )
	{
		switch ( base )
		{
		case 2: case 8: case 10: case 16: break;
		default:
			ret = nextchar( ')', dst );
			if ( ret != 0 )
				return ret;
		}
	}

	do
	{
		(void)alu_reg_divide( *alu, num, div, val );
		
		if ( digit == 3 )
		{
			switch ( digsep )
			{
			case '\'': case '_': case ',':
				ret = nextchar( base_str[*V], dst );
				if ( ret != 0 )
				{
					alu_rem_regv( *alu, regv, 3 );
					return ret;
				}
			}
			digit = 0;
		}
		
		ret = nextchar( base_str[*V], dst );
		
		if ( ret != 0 )
		{
			alu_rem_regv( *alu, regv, 3 );
			return ret;
		}
		
		++digit;
	}
	while ( alu_reg_cmp( *alu, num, div, NULL ) >= 0 );
	
	ret = nextchar( base_str[*N], dst );
	
	alu_rem_regv( *alu, regv, 3 );
	
	if ( ret != 0 )
		return ret;
		
	if ( !noPfx && base != 10 )
	{
		if ( base == 2 )
		{
			ret = nextchar( 'b', dst );
			if ( ret != 0 )
				return ret;
			
			ret = nextchar( '0', dst );
			if ( ret != 0 )
				return ret;
		}
		else if ( base == 8 )
		{
			ret = nextchar( 'o', dst );
			if ( ret != 0 )
				return ret;
			
			ret = nextchar( '0', dst );
			if ( ret != 0 )
				return ret;
		}
		else if ( base == 16 )
		{
			ret = nextchar( 'x', dst );
			if ( ret != 0 )
				return ret;
			
			ret = nextchar( '0', dst );
			if ( ret != 0 )
				return ret;
		}
		else
		{
			ret = nextchar( '(', dst );
			if ( ret != 0 )
				return ret;
			
			while ( base > 10 )
			{
				ret = nextchar( base_str[base%10], dst );
				if ( ret != 0 )
					return ret;
				base /= 10;
			}
			
			ret = nextchar( base_str[base], dst );
			if ( ret != 0 )
				return ret;
			
			ret = nextchar( '~', dst );
			if ( ret != 0 )
				return ret;
			
			ret = nextchar( '0', dst );
			if ( ret != 0 )
				return ret;
		}
	}
	
	if ( neg )
		ret = nextchar( '-', dst );
	
	flipstr( dst );
	
	return ret;
}
