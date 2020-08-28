#include "alu.h"
#include <string.h>
#include <stdio.h>

int alu_mov( alu_t *alu, uintptr_t num, uintptr_t val )
{
	uint_t tmp = -1;
	int ret = 0;
	alu_reg_t *REG = NULL;
	alu_vec_t *NUM = NULL, *VAL = NULL;
	size_t size = 0;
	
	if ( num >= ALU_REG_ID_LIMIT )
	{
		NUM = (alu_vec_t*)num;
		size = (NUM->mem.bytes.upto);
		
		if ( val >= ALU_REG_ID_LIMIT )
		{
			VAL = (alu_vec_t*)val;
			
			if ( size > VAL->mem.bytes.upto )
				size = VAL->mem.bytes.upto;
			
			errno = 0;
			(void)memmove( NUM->mem.block, VAL->mem.block, size );
			ret = errno;
			
			return ret;
		}
		
		ret = alu_check2( alu, ALU_REG_ID_NEED, val );
		
		if ( ret != 0 )
		{
			alu_error( ret );
			return ret;
		}
		
		ret = alu_get_reg( alu, &tmp, size );
		
		if ( ret != 0 )
		{
			alu_error( ret );
			return ret;
		}
		
		REG = (alu->regv + tmp);
		
		(void)alu__or( alu, tmp, val );
		
		(void)memset( NUM->mem.block, 0, size );
		
		if ( size > alu->buff.perN )
			size = alu->buff.perN;
		
		(void)memmove( NUM->mem.block, REG->part, size );
		
		(void)alu_rem_reg( alu, tmp );
		return 0;
	}
	
	if ( val >= ALU_REG_ID_LIMIT )
	{	
		VAL = (alu_vec_t*)val;
		size = (VAL->mem.bytes.upto);
		
		ret = alu_check1( alu, num );
		
		if ( ret != 0 )
		{
			alu_error( ret );
			return ret;
		}
		
		ret = alu_get_reg( alu, &tmp, size );
		
		if ( ret != 0 )
		{
			alu_error( ret );
			return ret;
		}
		
		REG = (alu->regv + tmp);
		
		if ( size > alu->buff.perN )
			size = alu->buff.perN;
		
		(void)memmove( REG->part, VAL->mem.block, size );
		
		(void)alu_xor( alu, num, num );
		(void)alu__or( alu, num, tmp );
		
		(void)alu_rem_reg( alu, tmp );
		return 0;
	}
	
	alu_zero( alu, num );
	alu__or( alu, num, val );
	
	return ret;
}

int alu_check_reg( alu_t *alu, uint_t reg )
{
	int ret = 0;
	alu_reg_t *REG;
	
	if ( !alu )
	{
		ret = EDESTADDRREQ;
		return ret;
	}
	
	if ( reg >= alu->_regv.qty.used )
	{
		ret = EADDRNOTAVAIL;
		return ret;
	}
	
	REG = alu->regv + reg;
	if ( REG->info & ALU_INFO_VALID )
	{
		ret = EADDRINUSE;
		return ret;
	}
	
	return 0;
}

int alu_check1( alu_t *alu, uint_t num )
{
	return (
		alu_check_reg( alu, num ) == EADDRINUSE
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
		alu_check_reg( alu, val ) == EADDRINUSE
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
		alu_check_reg( alu, rem ) == EADDRINUSE
		&& rem >= ALU_REG_ID_NEED
	) ? 0 : EDESTADDRREQ;
}

void alu_print_reg( char *pfx, alu_reg_t reg, bool print_info )
{
	alu_bit_t n;
	alu_block_t p = {0};
	size_t len = 0;
	char *_pfx;

	if ( print_info )
	{
		len = strlen(pfx);
		if ( alu_block( &p, len + 10, 0 ) == 0 )
		{
			_pfx = p.block;
			alu_printf( "%s.part = %p", pfx, reg.part );
			sprintf( _pfx, "%s.upto", pfx );
			alu_print_bit( _pfx, reg.upto, 0 );
			sprintf( _pfx, "%s.last", pfx );
			alu_print_bit( _pfx, reg.last, 0 );
			sprintf( _pfx, "%s.init", pfx );
			alu_print_bit( _pfx, reg.init, 0 );
			alu_block( &p, 0, 0 );
		}
	}
	
	n = reg.upto;
	fprintf( stderr, "%s = ", pfx );
	if ( n.b == reg.init.b )
	{
		fputc( '0', stderr );
		alu_error( ERANGE );
	}
	else
	{
		while ( n.b > reg.init.b )
		{
			n = alu_bit_dec(n);
			(void)fputc( '0' + !!(*(n.S) & n.B), stderr );
		}
	}
	fputc( '\n', stderr );
}

void alu_reg_set_bounds( alu_reg_t *reg, size_t init, size_t upto )
{
	if ( !upto ) upto = 1;
	reg->upto = alu_bit_set_bit( reg->part, upto );
	reg->last = alu_bit_set_bit( reg->part, upto - 1 );
	reg->init = alu_bit_set_bit( reg->part, init );
}

int alu_reg_reset_bounds( alu_t *alu, alu_reg_t *reg )
{
	int ret = 0;
	
	if ( !alu )
	{
		ret = EADDRNOTAVAIL;
		alu_error( ret );
		return ret;
	}
	
	if ( !reg )
	{
		ret = EDESTADDRREQ;
		alu_error( ret );
		return ret;
	}
	
	alu_reg_set_bounds( reg, 0, alu->buff.perN * CHAR_BIT );
	return 0;
}

int alu_reset_bounds( alu_t *alu, uint_t reg )
{
	int ret = alu_check1( alu, reg );
	alu_reg_t *REG;
	
	if ( ret != 0 )
	{
		alu_error( ret );
		return ret;
	}
	
	REG = alu->regv + reg;
	
	alu_reg_set_bounds( REG, 0, alu->buff.perN * CHAR_BIT );
	return 0;
}

int alu_setup_reg( alu_t *alu, uint_t want, size_t perN )
{
	int ret;
	uint_t i;
	alu_reg_t *REG;
	void *mem;
	
	if ( want < ALU_REG_ID_NEED )
		want = ALU_REG_ID_NEED;
	/* Needed for alu_mov() to support both register numbers and
	 * arbitrary addresses */
	if ( want > ALU_REG_ID_LIMIT )
		return ERANGE;
	
	if ( !perN )
		perN = sizeof(size_t);
	else if ( perN % sizeof(size_t) )
	{
		perN /= sizeof(size_t);
		perN++;
		perN *= sizeof(size_t);
	}
	
	ret = alu_vec_expand( &(alu->buff), want, perN );
	if ( ret != 0 )
		return ret;
		
	ret = alu_vec_expand( &(alu->_regv), want, sizeof(alu_reg_t) );
	if ( ret != 0 )
		return ret;
	
	alu->regv = alu->_regv.mem.block;
	
	mem = alu->buff.mem.block;

	for ( i = 0; i < alu->_regv.qty.upto; ++i )
	{
		REG = alu->regv + i;
		REG->part = mem + (perN * i);
		alu_reg_set_bounds( REG, REG->init.b, REG->upto.b );
	}
	
	for ( i = 0; i < ALU_REG_ID_NEED; ++i )
	{
		REG = alu->regv + i;
		REG->info = ALU_INFO_VALID;
		alu_reg_set_bounds( REG, 0, perN * CHAR_BIT );
	}
	
	REG = alu->regv + ALU_REG_ID_ZERO;
	(void)memset( REG->part, 0, perN );
	
	REG = alu->regv + ALU_REG_ID_UMAX;
	(void)memset( REG->part, -1, perN );
	
	REG = alu->regv + ALU_REG_ID_IMAX;
	REG->info |= ALU_INFO__SIGN;
	(void)memset( REG->part, -1, perN );
	*(REG->last.S) ^= REG->last.B;
	
	REG = alu->regv + ALU_REG_ID_IMIN;
	REG->info |= ALU_INFO__SIGN;
	(void)memset( REG->part, 0, perN );
	*(REG->last.S) |= REG->last.B;
	
	return 0;
}

int alu_get_reg( alu_t *alu, uint_t *reg, size_t size )
{
	int ret, count = 0, r = count;
	size_t perN;
	alu_reg_t *REG;

	if ( !alu )
	{
		ret = EDESTADDRREQ;
		alu_error( ret );
		return ret;
	}
	
	perN = alu->buff.perN;
	count = alu->_regv.qty.used;
	
	if ( size > perN )
		perN = size;
	
	if ( count <= ALU_REG_ID_NEED )
	{
		count = ALU_REG_ID_NEED + 1;
		ret = alu_setup_reg( alu, count, perN );
		if ( ret != 0 )
		{
			alu_error( ret );
			return ret;
		}
		alu->buff.qty.used = count;
		alu->_regv.qty.used = count;
	}
	
	for ( r = 0; r < count; ++r )
	{
		REG = alu->regv + r;
		if ( REG->info & ALU_INFO_VALID )
			continue;
		goto done;
	}
	
	ret = alu_setup_reg( alu, count + 1, perN );
	if ( ret != 0 )
	{
		alu_error( ret );
		return ret;
	}
	
	r = count;
	REG = alu->regv + r;
	alu->_regv.qty.used = alu->buff.qty.used = r + 1;
	
	done:
	REG->info = ALU_INFO_VALID;
	(void)alu_reset_bounds( alu, r );
	(void)memset( REG->part, 0, alu->buff.perN );
	*reg = r;
	
	return 0;
}

int alu_rem_reg( alu_t *alu, uint_t reg )
{
	int ret = alu_check1( alu, reg );
	alu_reg_t *R;
	
	switch ( ret )
	{
	case 0: case EINVAL: break;
	case EDESTADDRREQ: return 0;
	default:
		alu_error( ret );
		return ret;
	}
	
	R = alu->regv + reg;
	R->info = 0;
	return 0;
}

int alu_set_constants( alu_t *alu )
{
	int ret, want = alu->buff.qty.used;
	size_t perN = alu->buff.perN;
	alu_reg_t *REG;

	ret = alu_setup_reg( alu, want, perN );
	if ( ret != 0 )
		return ret;
	
	REG = alu->regv + ALU_REG_ID_ZERO;
	memset( REG->part, 0, perN );

	REG = alu->regv + ALU_REG_ID_UMAX;
	memset( REG->part, -1, perN );
	
	REG = alu->regv + ALU_REG_ID_IMAX;
	memset( REG->part, -1, perN );
	((uchar_t*)(REG->part))[perN-1] = SCHAR_MAX;
	
	REG = alu->regv + ALU_REG_ID_IMAX;
	memset( REG->part, 0, perN );
	((uchar_t*)(REG->part))[perN-1] = SCHAR_MIN;
	
	return 0;
}

int alu_str2reg
(
	alu_t *alu,
	void *src,
	uint_t reg_dst,
	uint_t reg_base,
	uint_t reg_mod,
	char32_t digsep,
	alu_func_rdChar32_t nextchar,
	long *nextpos,
	size_t base,
	bool lowercase
)
{
	size_t b, perN;
	alu_reg_t *BASE, *MOD, *DST;
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
	
	ret = alu_check3( alu, reg_dst, reg_base, reg_mod );
	
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

	/* Hook registers */
	BASE = alu->regv + reg_base;
	MOD = alu->regv + reg_mod;
	DST = alu->regv + reg_dst;

	/* Clear all registers in direct use */
	perN = alu->buff.perN;
	
	M = BASE->part;
	*M = base;
	
	M = MOD->part;
	n = DST->last;
	
	(void)alu_reset_bounds( alu, reg_dst );
	(void)alu_zero( alu, reg_dst );

	do
	{	
		/* Failsafe if failed to get character but return code is 0 */
		c = -1;
		ret = nextchar( &c, src, nextpos );
		
		if ( ret != 0 && ret != EOF )
			return ret;
		
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
			
			(void)alu_reset_bounds( alu, reg_dst );
			
			perN = alu->buff.perN;
			M = MOD->part;
			n = DST->last;
		}
		
		*M = b;
		
		(void)alu_mul( alu, reg_dst, reg_base );
		(void)alu_add( alu, reg_dst, reg_mod );
	}
	while ( ret == 0 );
	
	return 0;
}

enum
{
	ALU_LIT2REG_BASE = 0,
	ALU_LIT2REG_NUM,
	ALU_LIT2REG_DOT,
	ALU_LIT2REG_ONE,
	ALU_LIT2REG_EXP,
	ALU_LIT2REG_MAN,
	ALU_LIT2REG_VAL,
	ALU_LIT2REG_COUNT
};

bool alu_reg_is_zero( alu_reg_t reg, alu_bit_t *end_bit )
{
	alu_bit_t n = alu_end_bit( reg );
	if ( end_bit ) *end_bit = n;
	return !(*(n.S) & n.B);
}

int alu_lit2reg
(
	alu_t *alu,
	void *src,
	uint_t dst,
	char32_t digsep,
	alu_func_rdChar32_t nextchar,
	long *nextpos,
	bool lowercase
)
{
	size_t base, perN, bits, exp_bits, man_bits, *BASE;
	alu_reg_t *_REGV[ALU_LIT2REG_COUNT], *DST;
	alu_bit_t n;
	int ret, i;
	long pos, prevpos;
	ssize_t *EXP, min_exp = 0, max_exp = 0;
	const int regc = 3;
	uint_t _regv[ALU_LIT2REG_COUNT] = {-1};
	char32_t *M, c = -1, exponent_char = 'e';
	bool neg = false, exp_neg = false, closing_bracket = true;
	
	if ( !nextpos )
		return EDESTADDRREQ;
	
	if ( *nextpos < 0 )
		*nextpos = 0;
	
	ret = alu_check1( alu, dst );
	
	if ( ret != 0 )
		return ret;
	
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
	
	for ( i = 0; i < regc; ++i )
	{
		ret = alu_get_reg( alu, _regv + i, sizeof(size_t) );
		
		if ( ret != 0 )
		{
			while ( i > 0 )
			{
				--i;
				alu_rem_reg( alu, _regv[i] );
			}
			return ret;
		}
	}
	
	/* Hook registers */
	for ( i = 0; i < regc; ++i )
		_REGV[i] = alu->regv + _regv[i];
	DST = alu->regv + dst;
	
	EXP = _REGV[ALU_LIT2REG_EXP]->part;
	BASE = _REGV[ALU_LIT2REG_BASE]->part;
	*BASE = base;
	
	ret = alu_str2reg(
		alu,
		src,
		dst,
		_regv[ALU_LIT2REG_BASE],
		_regv[ALU_LIT2REG_VAL],
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
			_regv[ALU_LIT2REG_DOT],
			_regv[ALU_LIT2REG_BASE],
			_regv[ALU_LIT2REG_VAL],
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
		bits = DST->upto.b - DST->init.b;
		if ( bits == (perN * CHAR_BIT) )
		{
			ret = alu_setup_reg( alu, alu->buff.qty.upto, perN * 2 );
			if ( ret != 0 )
			{
				alu_error(ret);
				return ret;
			}
			perN = alu->buff.perN;
			
			/* Reset pointers */
			for ( i = 0; i < regc; ++i )
				_REGV[i] = alu->regv + _regv[i];
			DST = alu->regv + dst;
			
			EXP = _REGV[ALU_LIT2REG_EXP]->part;
			BASE = _REGV[ALU_LIT2REG_BASE]->part;
		}
		
		/* Check how many bits to assign to exponent & mantissa */
		if ( bits < bitsof(float) )
		{
			if ( perN > 1 )
				man_bits = CHAR_BIT - 1;
			else
				man_bits = (CHAR_BIT / 2) - 1;
		}
		else if ( bits < bitsof(double) )
			man_bits = FLT_MANT_DIG;
		else if ( bits < bitsof(long double) )
			man_bits = DBL_MANT_DIG;
		else if ( bits == bitsof(long double) )
			man_bits = LDBL_MANT_DIG;
		else
			man_bits = (bits / 4) - 1;
		
		exp_bits = ((perN * CHAR_BIT) - man_bits) - 1;
		max_exp = ~(~max_exp << (bitsof(ssize_t) - exp_bits));
		min_exp = (-max_exp - 1);
		
		/* Update Exponent & Mantissa Registers */
		n = alu_bit_set_bit( _REGV[ALU_LIT2REG_EXP]->part, exp_bits - 1 );
		_REGV[ALU_LIT2REG_EXP]->upto = n;
		_REGV[ALU_LIT2REG_EXP]->last = alu_bit_dec(n);
		_REGV[ALU_LIT2REG_EXP]->info |= ALU_INFO__SIGN;
		
		n = alu_bit_set_bit( _REGV[ALU_LIT2REG_MAN]->part, man_bits );
		_REGV[ALU_LIT2REG_MAN]->upto = n;
		_REGV[ALU_LIT2REG_MAN]->last = alu_bit_dec(n);
		
		/* Set '0.1' */
		alu_zero( alu, _regv[ALU_LIT2REG_ONE] );
		M = _REGV[ALU_LIT2REG_ONE]->part;
		*M = 1u;
		
		/* Multiply '0.1' by base to get '1.0',
		 * maybe be more digits than size_t supports */
		for ( pos = prevpos; pos < *nextpos; ++pos )
			alu_mul( alu, _regv[ALU_LIT2REG_ONE], _regv[ALU_LIT2REG_BASE] );
		/* Restore base to what it should be */
		*M = base;
		
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
				_regv[ALU_LIT2REG_EXP],
				_regv[ALU_LIT2REG_BASE],
				_regv[ALU_LIT2REG_VAL],
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
			
			if ( exp_neg )
				*EXP = -(*EXP);
		}
		
		/* Exponent had 1 bit less to prevent reading to much */
		n = alu_bit_set_bit( _REGV[ALU_LIT2REG_EXP]->part, exp_bits );
		_REGV[ALU_LIT2REG_EXP]->upto = n;
		_REGV[ALU_LIT2REG_EXP]->last = alu_bit_dec(n);
		_REGV[ALU_LIT2REG_EXP]->info |= ALU_INFO__SIGN;
		
		if ( alu_reg_is_zero( *(_REGV[ALU_LIT2REG_DOT]), &n ) )
		{
			alu_zero( alu, _regv[ALU_LIT2REG_ONE] );
			*M = 1u;
		}
		
		pos = 0;
		for ( ; *EXP > 0; --(*EXP), ++pos )
		{
			/* Check we haven't hit 1 */
			if ( pos == 0
				|| alu_reg_is_zero( *(_REGV[ALU_LIT2REG_ONE]), &n )
			)
			{
				break;
			}
		}
		
		if ( *EXP < min_exp )
		{
			set_nil:
			(void)alu_zero( alu, dst );
			goto set_sign;
		}
		
		if ( *EXP > max_exp )
		{
			set_inf:
			(void)alu_zero( alu, dst );
			(void)alu_not( alu, dst );
			(void)alu_shl( alu, dst, man_bits );
			*(DST->last.S) ^= DST->last.B;
			goto set_sign;
		}
		
		for ( ; *EXP > 0; --(*EXP) )
		{
			ret = alu_mul( alu, dst, _regv[ALU_LIT2REG_BASE] );
			if ( ret == EOVERFLOW )
				goto set_inf;
		}
		
		for ( ; *EXP < 0; ++(*EXP) )
		{
			ret = alu_mul( alu, _regv[ALU_LIT2REG_ONE], _regv[ALU_LIT2REG_BASE] );
			if ( ret == EOVERFLOW )
				goto set_nil;
		}
		
		if ( alu_reg_is_zero( *(_REGV[ALU_LIT2REG_ONE]), &n ) )
			goto set_nil;
			
		(void)alu_divide(
			alu,
			dst,
			_regv[ALU_LIT2REG_ONE],
			_regv[ALU_LIT2REG_DOT]
		);
		
		if ( alu_reg_is_zero( *(_REGV[ALU_LIT2REG_DOT]), &n ) )
		{
			alu_zero( alu, _regv[ALU_LIT2REG_ONE] );
			*M = 1u;
		}
		
		pos = 0;
		
		/* Calculate final exponent */
		
		if ( alu_compare( *DST, *(_REGV[ALU_LIT2REG_ONE]), NULL ) >= 0 )
		{
			for
			(
				alu_mov( alu, _regv[ALU_LIT2REG_NUM], dst )
				; alu_compare(
					*(_REGV[ALU_LIT2REG_NUM]),
					*(_REGV[ALU_LIT2REG_ONE]),
					NULL
				) > 0
				; ++pos, alu__shr( alu, _regv[ALU_LIT2REG_NUM], 1 )
			);
		}
		else
		{
			for
			(
				alu_mov
				(
					alu,
					_regv[ALU_LIT2REG_NUM],
					_regv[ALU_LIT2REG_ONE]
				)
				; alu_compare
				(
					*(_REGV[ALU_LIT2REG_NUM]),
					*(_REGV[ALU_LIT2REG_DOT]),
					NULL
				) > 0
				; --pos, alu__shr( alu, _regv[ALU_LIT2REG_NUM], 1 )
			);
			if
			(
				alu_compare
				(
					*(_REGV[ALU_LIT2REG_NUM]),
					*(_REGV[ALU_LIT2REG_DOT]),
					NULL
				) == 0
			)
			{
				--pos;
			}
		}
		
		*EXP = pos;
		
		/* TODO: Continue referencing code made in mitsy to build fpn */
		
		/* Construct FPN from modified values */
		alu__or( alu, dst, _regv[ALU_LIT2REG_MAN] );
		//set_exp:
		alu_reg_set_bounds( _REGV[ALU_LIT2REG_EXP], 0, (perN * CHAR_BIT) );
		alu__shl( alu, _regv[ALU_LIT2REG_EXP], man_bits );
		alu__or( alu, dst, _regv[ALU_LIT2REG_EXP] );
		set_sign:
		if ( neg )
		{
			alu_zero( alu, _regv[ALU_LIT2REG_ONE] );
			M = _REGV[ALU_LIT2REG_ONE]->part;
			*M = 1u;
			alu__shl( alu, _regv[ALU_LIT2REG_ONE], exp_bits + man_bits );
			alu__or( alu, dst, _regv[ALU_LIT2REG_ONE] );
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
			alu_neg( alu, dst );
		
		alu_mov( alu, dst, _regv[ALU_LIT2REG_NUM] );
	}
	
	for ( i = ALU_LIT2REG_COUNT; i > 0; )
	{
		--i;
		alu_rem_reg( alu, _regv[i] );
	}
	
	return 0;
}

int alu_reg2str
(
	alu_t *alu,
	void *dst,
	uint_t src,
	char32_t digsep,
	alu_func_wrChar32_t nextchar,
	alu_func_flipstr_t flipstr,
	size_t base,
	bool lowercase,
	bool noPfx,
	bool noSign
)
{
	alu_reg_t *DIV, *VAL, *NUM, *SRC;
	int ret;
	uint_t num = -1, div = -1, val = -1;
	size_t *V, *N, digit = 0;
	bool neg = false;
	char *base_str = lowercase ?
		ALU_BASE_STR_0toztoZ :
		ALU_BASE_STR_0toZtoz;
		
	ret = alu_check1( alu, src );
	
	if ( ret != 0 )
		return ret;
	
	SRC = alu->regv + src;
	neg = (
		(SRC->info & ALU_INFO__SIGN)
		&& (*(SRC->last.S) & SRC->last.B)
	);
	
	if ( neg && noSign )
		return EINVAL;
	
	if ( !val || !nextchar || !flipstr )
		return EADDRNOTAVAIL;
		
	if ( !base )
		base = 10;
	
	if ( base > strlen(base_str) )
		return ERANGE;
	
	ret = alu_get_reg( alu, &num, sizeof(size_t) );
		
	if ( ret != 0 )
		return ret;
		
	ret = alu_get_reg( alu, &val, sizeof(size_t) );
		
	if ( ret != 0 )
	{
		(void)alu_rem_reg( alu, num );
		return ret;
	}
	
	ret = alu_get_reg( alu, &div, sizeof(size_t) );
		
	if ( ret != 0 )
	{
		(void)alu_rem_reg( alu, num );
		(void)alu_rem_reg( alu, val );
		return ret;
	}
	
	/* Hook registers */
	DIV = alu->regv + div;
	VAL = alu->regv + val;
	NUM = alu->regv + num;
	
	(void)alu_mov( alu, num, src );
	
	V = DIV->part;
	*V = base;
	
	if ( neg )
		(void)alu_neg( alu, num );
	
	N = NUM->part;
	V = VAL->part;
	
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
		(void)alu_divide( alu, num, div, val );
		
		if ( digit == 3 )
		{
			switch ( digsep )
			{
			case '\'': case '_': case ',':
				ret = nextchar( base_str[*V], dst );
				if ( ret != 0 )
				{
					alu_rem_reg( alu, num );
					alu_rem_reg( alu, div );
					alu_rem_reg( alu, val );
					return ret;
				}
			}
			digit = 0;
		}
		
		ret = nextchar( base_str[*V], dst );
		
		if ( ret != 0 )
		{
			alu_rem_reg( alu, num );
			alu_rem_reg( alu, div );
			alu_rem_reg( alu, val );
			return ret;
		}
		
		++digit;
	}
	while ( alu_compare( *NUM, *DIV, NULL ) >= 0 );
	
	ret = nextchar( base_str[*N], dst );
	
	alu_rem_reg( alu, num );
	alu_rem_reg( alu, div );
	alu_rem_reg( alu, val );
	
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
