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
	if ( REG->info & ALU_REG_F_VALID )
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

int alu_reset_reg( alu_t *alu, uint_t reg, bool preserve_positions )
{
	int ret = alu_check_reg( alu, reg );
	alu_reg_t *REG;
	
	if ( ret != EADDRINUSE && ret != 0 )
	{
		alu_error( ret );
		return ret;
	}
	
	REG = alu->regv + reg;
	
	if ( preserve_positions )
	{
		REG->upto = alu_bit_set_bit( REG->part, REG->upto.b );
		REG->last = alu_bit_set_bit( REG->part, REG->last.b );
		REG->init = alu_bit_set_bit( REG->part, REG->init.b );
	}
	else
	{
		REG->upto = alu_bit_set_byte( REG->part, alu->buff.perN );
		REG->last = alu_bit_set_bit( REG->part, REG->upto.b - 1 );
		REG->init = alu_bit_set_byte( REG->part, 0 );
	}
#if 0
	alu_print_reg( alu, reg );
#endif
	
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
		alu_reset_reg( alu, i, 1 );
	}
	
	for ( i = 0; i < ALU_REG_ID_NEED; ++i )
	{
		REG = alu->regv + i;
		REG->info = ALU_REG_F_VALID;
		alu_reset_reg( alu, i, 0 );
	}
	
	REG = alu->regv + ALU_REG_ID_ZERO;
	(void)memset( REG->part, 0, perN );
	
	REG = alu->regv + ALU_REG_ID_UMAX;
	(void)memset( REG->part, -1, perN );
	
	REG = alu->regv + ALU_REG_ID_IMAX;
	REG->info |= ALU_REG_F__SIGN;
	(void)memset( REG->part, -1, perN );
	*(REG->last.S) ^= REG->last.B;
	
	REG = alu->regv + ALU_REG_ID_IMIN;
	REG->info |= ALU_REG_F__SIGN;
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
		if ( REG->info & ALU_REG_F_VALID )
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
	REG->info = ALU_REG_F_VALID;
	(void)alu_reset_reg( alu, r, 0 );
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
	uint_t dst,
	alu_func_rdChar32_t nextchar,
	long *nextpos,
	size_t base,
	bool lowercase
)
{
	size_t b, perN;
	alu_reg_t *MUL, *VAL, *DST;
	alu_bit_t n;
	int ret;
	uint_t mul = -1, val = -1;
	char32_t *V, c = 0;
	char *base_str = lowercase ?
		ALU_BASE_STR_0toztoZ :
		ALU_BASE_STR_0toZtoz;
	
	if ( !nextpos )
		return EDESTADDRREQ;
	
	if ( *nextpos < 0 )
		*nextpos = 0;
	
	ret = alu_check1( alu, dst );
	
	if ( ret != 0 )
		return ret;
	
	if ( !val || !nextchar )
		return EADDRNOTAVAIL;
	
	if ( base < 2 || base > strlen(base_str) )
		return ERANGE;
		
		
	ret = alu_get_reg( alu, &val, sizeof(size_t) );
		
	if ( ret != 0 )
		return ret;
	
	ret = alu_get_reg( alu, &mul, sizeof(size_t) );
		
	if ( ret != 0 )
	{
		alu_rem_reg( alu, val );
		return ret;
	}
	
	/* Hook registers */
	MUL = alu->regv + mul;
	VAL = alu->regv + val;
	DST = alu->regv + dst;

	/* Clear all registers in direct use */
	perN = alu->buff.perN;
	
	V = MUL->part;
	*V = base;
	
	V = VAL->part;
	n = DST->last;
	
	alu_reset_reg( alu, dst, false );
	memset( DST->part, 0, alu->buff.perN );

	do
	{	
		/* Failsafe if failed to get character but return code is 0 */
		c = -1;
		ret = nextchar( &c, src, nextpos );
		
		if ( ret != 0 && ret != EOF )
			return ret;
		
		for ( b = 0; b < base; ++b )
		{
			if ( c == (char32_t)(base_str[b]) )
				break;
		}
		
		if ( b == base )
			break;
		
		++(*nextpos);
		
		if ( *(n.S) & n.B )
		{
			ret = alu_setup_reg(
				alu, alu->buff.qty.upto, perN + sizeof(size_t)
			);
			
			if ( ret != 0 )
				return ret;
				
			(void)alu_reset_reg( alu, dst, false );
			
			perN = alu->buff.perN;
			V = VAL->part;
			n = DST->last;
		}
		
		*V = b;
		
		(void)alu_mul( alu, dst, mul );
		(void)alu_add( alu, dst, val );
	}
	while ( ret == 0 );
	
	alu_rem_reg( alu, mul );
	alu_rem_reg( alu, val );
	
	return 0;
}

int alu_reg2str
(
	alu_t *alu,
	void *dst,
	uint_t src,
	alu_func_wrChar32_t nextchar,
	alu_func_flipstr_t flipstr,
	size_t base,
	bool lowercase
)
{
	alu_reg_t *DIV, *VAL, *NUM;
	int ret;
	uint_t num = -1, div = -1, val = -1;
	size_t *V, *N;
	char *base_str = lowercase ?
		ALU_BASE_STR_0toztoZ :
		ALU_BASE_STR_0toZtoz;
		
	ret = alu_check1( alu, src );
	
	if ( ret != 0 )
		return ret;
	
	if ( !val || !nextchar || !flipstr )
		return EADDRNOTAVAIL;
	
	if ( base < 2 || base > strlen(base_str) )
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
	
	N = NUM->part;
	V = VAL->part;
	
	//alu_printf( "num = %zu", *N );

	do
	{	
		(void)alu_divide( alu, num, div, val );
		ret = nextchar( base_str[*V], dst );
		
		alu_printf( "num = %zu", *N );
		
		if ( ret != 0 )
		{
			alu_rem_reg( alu, num );
			alu_rem_reg( alu, div );
			alu_rem_reg( alu, val );
			return ret;
		}
	}
	while ( alu_compare( *NUM, *DIV, NULL ) >= 0 );
	
	ret = nextchar( base_str[*N], dst );
	
	alu_rem_reg( alu, num );
	alu_rem_reg( alu, div );
	alu_rem_reg( alu, val );
	
	if ( ret != 0 )
		return ret;
	
	flipstr( dst );
	
	return 0;
}
