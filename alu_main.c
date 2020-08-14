#include "alu.h"
#include <string.h>
#include <stdio.h>

int alu_mov( alu_t *alu, uintptr_t num, uintptr_t val )
{
	int ret = 0, tmp = -1;
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
	
	alu_xor( alu, num, num );
	alu__or( alu, num, val );
	
	return ret;
}

int alu_check_reg( alu_t *alu, int reg )
{
	int ret = 0;
	alu_reg_t *REG;
	
	if ( !alu )
	{
		ret = EDESTADDRREQ;
		return ret;
	}
	
	if ( reg < 0 || reg >= alu->_regv.qty.used )
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

int alu_check1( alu_t *alu, int num )
{
	return (
		alu_check_reg( alu, num ) == EADDRINUSE
		&& num >= ALU_REG_ID_NEED
	) ? 0 : EDESTADDRREQ;
}

int alu_check2( alu_t *alu, int num, int val )
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

int alu_check3( alu_t *alu, int num, int val, int rem )
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

int alu_pri_reg( alu_t *alu, int reg )
{
	int ret = alu_check_reg( alu, reg );
	alu_reg_t *REG;
	alu_bit_t n;
	
	if ( ret != EADDRINUSE && ret != 0 )
	{
		alu_error( ret );
		return ret;
	}
	
	REG = alu->regv + reg;
	
	alu_printf( "reg = %i, REG->part = %p", reg, REG->part );
	
	alu_printf(
		"REG->upto.b = %zu, REG->last.b = %zu, REG->init.b = %zu",
		REG->upto.b, REG->last.b, REG->init.b
	);
	
	alu_printf(
		"REG->upto.s = %zu, REG->last.s = %zu, REG->init.s = %zu",
		REG->upto.s, REG->last.s, REG->init.s
	);
	
	alu_printf(
		"REG->upto.S = %p, REG->last.S = %p, REG->init.S = %p",
		REG->upto.S, REG->last.S, REG->init.S
	);
	
	n = REG->upto;
	if ( n.b == REG->init.b )
	{
		fputc( '0', stderr );
		alu_error( ERANGE );
	}
	else
	{
		while ( n.b > REG->init.b )
		{
			n = alu_bit_dec(n);
			(void)fputc( '0' + !!(*(n.S) & n.B), stderr );
		}
	}
	fputc( '\n', stderr );
	
	return 0;
}

int alu_reset_reg( alu_t *alu, int reg, bool preserve_positions )
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
	alu_pri_reg( alu, reg );
#endif
	
	return 0;
}

int alu_setup_reg( alu_t *alu, int want, size_t perN )
{
	int ret, i;
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

int alu_get_reg( alu_t *alu, int *reg, size_t size )
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

int alu_rem_reg( alu_t *alu, int reg )
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


int alu_str2int
(
	alu_t *alu,
	void *val,
	alu_func_nextchar_t nextchar,
	size_t *nextpos,
	size_t base,
	bool lowercase
)
{
	size_t b, perN;
	alu_reg_t *N, *M, *V;
	int ret = 0, r, n[3] = {-1};
	char32_t c = 0;
	char *base_str = lowercase ?
		ALU_BASE_STR_0toztoZ :
		ALU_BASE_STR_0toZtoz;
	uchar_t *D, *S;
	
	if ( !alu || !nextpos )
		return EDESTADDRREQ;
	
	if ( !val || !nextchar )
		return EADDRNOTAVAIL;
	
	if ( base < 2 || base > strlen(base_str) )
		return ERANGE;
	
	/* Clear counts */
	alu->buff.mem.bytes.used = 0;
	alu->buff.qty.used = 0;
		
	/* Ensure we have enough registers for our own referencing */
	for ( r = 0; r < 3; ++r )
	{
		ret = alu_get_reg( alu, n + r, sizeof(size_t) );
		
		if ( ret != 0 )
			return ret;
	}
	
	/* Hook registers */
	M = alu->regv + n[0];
	V = alu->regv + n[1];
	N = alu->regv + n[2];

	/* Clear all registers in direct use */
	perN = alu->buff.perN;
	
	*((uchar_t*)M->part) = *((uchar_t*)&base);
	D = V->part;
	S = (uchar_t*)(&c);

	do
	{	
		ret = nextchar( &c, val, nextpos );
		
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
		
		if ( *(N->last.S) & SIZE_END_BIT )
		{
			ret = alu_setup_reg(
				alu, alu->buff.qty.upto, perN + sizeof(size_t)
			);
			
			if ( ret != 0 )
				return ret;
			
			perN = alu->buff.perN;
			D = V->part;
		}
		
		*D = *S;
		
		ret = alu_mul( alu, n[2], n[0] );
		
		if ( ret != 0 )
			break;
		
		ret = alu_add( alu, n[2], n[1] );
	}
	while ( ret == 0 );
	
	return 0;
}
