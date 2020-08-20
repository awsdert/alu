#include "alu.h"

int alu_zero( alu_t *alu, uint_t num )
{
	alu_reg_t *N;
	alu_bit_t n;
	int ret = alu_check1( alu, num );
	
	if ( ret != 0 )
	{
		alu_error( ret );
		return ret;
	}
	
	N = alu->regv + num;
	
	for ( n = N->init; n.b < N->upto.b; n = alu_bit_inc(n) )
		*(n.S) &= ~(n.B);
		
	return 0;
}

int alu_copy( alu_t *alu, uint_t num, uint_t val )
{
	alu_reg_t *N, *V;
	alu_bit_t n, v, e;
	int ret = alu_check2( alu, num, val );
	
	if ( ret != 0 )
	{
		alu_error( ret );
		return ret;
	}
	
	(void)alu_zero( alu, num );
	
	N = alu->regv + num;
	V = alu->regv + val;
	
	n = N->init;
	v = V->init;
	e = alu_bit_set_bit( N->part,
		n.b + ((N->upto.b - n.b) < (V->upto.b - v.b)
			?  N->upto.b : V->upto.b)
	);
	
	for ( n = N->init; n.b < e.b; n = alu_bit_inc(n) )
	{
		if ( *(v.S) & v.B )
			*(n.S) |= n.B;
	}
		
	return 0;
}

alu_bit_t alu_end_bit( alu_reg_t val )
{
	alu_bit_t v;
	
	for ( v = val.last; v.b > val.init.b; v = alu_bit_dec( v ) )
	{
		if ( *(v.S) & v.B )
			return v;
	}
	
	return val.init;
}

int alu_cmp( alu_t *alu, uint_t num, uint_t val, int *cmp, size_t *bit )
{
	int ret = 0;
	
	if ( !cmp )
	{
		ret = EDESTADDRREQ;
		alu_error(ret);
		return ret;
	}
	
	ret = alu_check2( alu, num, val );
	
	if ( ret != 0 )
		return ret;
	
	*cmp = alu_compare( alu->regv[num], alu->regv[val], bit );
	return 0;
}

int alu_compare( alu_reg_t num, alu_reg_t val, size_t *bit )
{
	int a, b, c = 0;
	alu_bit_t n = {0}, v = {0};
	size_t ndiff = 0, vdiff = 0;
	bool nNeg, vNeg;
	
	n = alu_end_bit( num );
	v = alu_end_bit( val );
	
	nNeg = ( num.info & ALU_REG_F__SIGN && n.b == num.last.b );
	vNeg = ( val.info & ALU_REG_F__SIGN && v.b == val.last.b );
	
	if ( nNeg != vNeg )
	{
		c = -1 + vNeg + vNeg;
		goto done;
	}
	
	ndiff = n.b - num.init.b;
	vdiff = v.b - val.init.b;
	
	a = nNeg;
	b = vNeg;

#if 0
	alu_printf(
		"a = %i, b = %i, ndiff = %zu, vdiff = %zu",
		a, b, ndiff, vdiff
	);
#endif
	
	/* Deal with different sized integers */

	while ( ndiff < vdiff )
	{
		b = (*(v.S) & v.B) ? 1 : 0;
		
		c = a - b;
		if ( c != 0 )
			goto done;
		
		vdiff--;
		v = alu_bit_dec( v );
	}
	
	while ( ndiff > vdiff )
	{
		a = (*(n.S) & n.B) ? 1 : 0;
		
		c = a - b;
		if ( c != 0 )
			goto done;
		
		ndiff--;
		n = alu_bit_dec( n );
	}
	
	do
	{
		a = (*(n.S) & n.B) ? 1 : 0;
		b = (*(v.S) & v.B) ? 1 : 0;
		
		c = a - b;
		if ( c != 0 )
			goto done;
			
		if ( !ndiff )
			break;
		
		ndiff--;
		n = alu_bit_dec( n );
		v = alu_bit_dec( v );
	}
	while ( 1 );
	
	done:
	if ( bit )
		*bit = ndiff;
	return c;
}

int alu_size2reg( alu_t *alu, uint_t num, size_t val )
{
	int ret = alu_check1( alu, num );
	size_t i;
	uchar_t *dst;
	alu_reg_t *N;
	
	if ( ret != 0 )
		return ret;
	
	N = alu->regv + num;
	dst = N->part;
	
	for ( i = 0; i < sizeof(size_t); ++i )
	{
		/* I believe this method ignores endian, do tell if I'm wrong */
		dst[i] = (val >> (i * CHAR_BIT)) & UCHAR_MAX;
	}
	
	return 0;
}

int alu_reg2size( alu_t *alu, uint_t num, size_t *val )
{
	int ret = alu_check1( alu, num );
	size_t i, dst;
	uchar_t *src;
	alu_reg_t *N;
	
	if ( !val )
		return EDESTADDRREQ;
	
	*val = 0;
	
	if ( ret != 0 )
	{
		if ( ret == EDESTADDRREQ )
			ret = EADDRNOTAVAIL;
		return ret;
	}
	
	N = alu->regv + num;
	src = N->part;
	
	for ( i = sizeof(size_t); i; )
	{
		/* I believe this method ignores endian, do tell if I'm wrong */
		--i;
		dst <<= CHAR_BIT;
		dst |= src[i];
	}
	
	*val = dst;
	
	return 0;
}

int alu_not( alu_t *alu, uint_t num )
{
	int ret = alu_check1( alu, num );
	alu_reg_t *N;
	alu_bit_t n;
	
	if ( ret != 0 )
		return ret;
	
	N = alu->regv + num;
	n = N->init;
	
	while ( n.b < N->upto.b )
	{
		if ( *(n.S) & n.B )
			*(n.S) ^= n.B;
		else
			*(n.S) |= n.B;
			
		n = alu_bit_inc( n );
	}
	
	return 0;
}

int alu_inc( alu_t *alu, uint_t num )
{
	bool carry = 0;
	int ret = alu_check1( alu, num );
	alu_reg_t *N;
	alu_bit_t n;
	
	if ( ret != 0 )
		return ret;
	
	N = alu->regv + num;
	n = N->init;
	
	if ( *(n.S) & n.B )
	{
		*(n.S) ^= n.B;
		carry = true;
		for (
			n = alu_bit_inc( n );
			n.b < N->upto.b;
			n = alu_bit_inc( n )
		)
		{
			if ( (*(n.S) & n.B) )
				*(n.S) ^= n.B;
			else
			{
				*(n.S) |= n.B;
				carry = false;
				break;
			}
		}
	}
	else
		*(n.S) |= n.B;
	
	return carry ? EOVERFLOW : 0;
}

int alu_dec( alu_t *alu, uint_t num )
{
	bool carry = 0;
	int ret = alu_check1( alu, num );
	alu_reg_t *N;
	alu_bit_t n;
	
	if ( ret != 0 )
		return ret;
	
	N = alu->regv + num;
	n = N->init;
	
	if ( *(n.S) & n.B )
		*(n.S) ^= n.B;
	else
	{
		*(n.S) |= n.B;
		carry = true;
		for (
			n = alu_bit_inc( n );
			n.b < N->upto.b;
			n = alu_bit_inc( n )
		)
		{
			if ( (*(n.S) & n.B) )
			{
				*(n.S) ^= n.B;
				carry = false;
				break;
			}
			else
				*(n.S) |= n.B;
		}
	}
	
	return carry ? EOVERFLOW : 0;
}

int alu_add( alu_t *alu, uint_t num, uint_t val )
{
	bool carry = false, changed = false;
	int ret = alu_check2( alu, num, val );
	alu_reg_t *N, *V;
	alu_bit_t n, v = {0};
	
	if ( ret != 0 )
		return ret;
	
	N = alu->regv + num;
	V = alu->regv + val;
	
	for (
		n = N->init, v = V->init;
		n.b < N->upto.b;
		n = alu_bit_inc( n ), v = alu_bit_inc( v )
	)
	{
		if ( carry )
		{
			if ( (*(n.S) & n.B) )
				*(n.S) ^= n.B;
			else
			{
				*(n.S) |= n.B;
				carry = false;
			}
		}
		
		if ( *(v.S) & v.B )
		{
			changed = true;
			if ( *(n.S) & n.B )
			{
				*(n.S) ^= n.B;
				carry = true;
			}
			else
				*(n.S) |= n.B;
		}
	}
	
	return changed ? (carry ? EOVERFLOW : 0) : ENODATA;
}

int alu_sub( alu_t *alu, uint_t num, uint_t val )
{
	bool carry = false, changed = false;
	int ret = alu_check2( alu, num, val );
	alu_reg_t *N, *V;
	alu_bit_t n, v = {0};
	
	if ( ret != 0 )
		return ret;
	
	N = alu->regv + num;
	V = alu->regv + val;
	
	for (
		n = N->init, v = V->init;
		n.b < N->upto.b;
		n = alu_bit_inc( n ), v = alu_bit_inc( v )
	)
	{
		if ( carry )
		{
			if ( (*(n.S) & n.B) )
			{
				*(n.S) ^= n.B;
				carry = false;
			}
			else
				*(n.S) |= n.B;
		}
		
		if ( *(v.S) & v.B )
		{
			changed = true;
			if ( *(n.S) & n.B )
				*(n.S) ^= n.B;
			else
			{
				*(n.S) |= n.B;
				carry = true;
			}
		}
	}
	
	return changed ? (carry ? EOVERFLOW : 0) : ENODATA;
}

int alu__shl( alu_t *alu, uint_t num, size_t by )
{
	alu_reg_t *N;
	alu_bit_t n = {0}, v;
	int ret = alu_check1( alu, num );
	
	if ( ret != 0 )
	{
		alu_error( ret );
		return ret;
	}
	
	if ( !by )
		return 0;
	
	N = alu->regv + num;
	
	n = N->upto;
	v = N->init;
	if ( by < (n.b - v.b) )
		v = alu_bit_set_bit( N->part, n.b - by );
	
	while ( v.b > N->init.b )
	{
		n = alu_bit_dec( n );
		v = alu_bit_dec( v );
	
		*(n.S) &= ~(n.B);
		
		if ( *(v.S) & v.B )
			*(n.S) |= n.B;
	}

	while ( n.b > N->init.b )
	{
		n = alu_bit_dec( n );
		
		*(n.S) &= ~(n.B);
	}
	
	return 0;
}

int alu__shr( alu_t *alu, uint_t num, size_t by )
{
	alu_reg_t *N;
	alu_bit_t n, v, e;
	int ret = alu_check1( alu, num );
	size_t neg = 0;
	
	if ( ret != 0 )
	{
		alu_error( ret );
		return ret;
	}
	
	if ( !by )
		return 0;
	
	N = alu->regv + num;
	
	n = N->init;
	e = v = N->upto;
	if ( by < (e.b - n.b) )
		v = alu_bit_set_bit( N->part, n.b + by );
	
	if ( (N->info & ALU_REG_F__SIGN) && (*(N->last.S) & N->last.B) )
		neg = ~neg;
	
	while ( v.b < e.b )
	{	
		*(n.S) &= ~(n.B);
		
		if ( *(v.S) & v.B )
			*(n.S) |= n.B;
			
		v = alu_bit_inc( v );
		n = alu_bit_inc( n );
	}
	
	while ( n.b < e.b )
	{
		*(n.S) &= ~(n.B);
		*(n.S) |= (neg & n.B);
		
		n = alu_bit_inc( n );
	}
	
	return 0;
}

int alu__shift( alu_t *alu, uint_t num, uint_t val, bool left )
{
	alu_reg_t *N;
	uint_t tmp = -1;
	int ret = alu_check2( alu, num, val ), cmp;
	size_t by = 0, bit;
	alu_uint_t bits = {0};
	
	if ( ret != 0 )
	{
		alu_error( ret );
		return ret;
	}
	
	ret = alu_get_reg( alu, &tmp, sizeof(size_t) );
	
	if ( ret != 0 )
	{
		alu_error( ret );
		return ret;
	}
	
	N = alu->regv + num;
	
	bits.perN = sizeof(size_t);
	bits.qty.upto = bits.qty.used = 1;
	bits.qty.last = 0;
	bits.mem.block = &(N->upto.b);
	bits.mem.bytes.upto = bits.mem.bytes.used = sizeof(size_t);
	bits.mem.bytes.last = sizeof(size_t) - 1;
	ret = alu_mov( alu, tmp, (uintptr_t)(&bits) );
	
	if ( ret != 0 )
	{
		(void)alu_rem_reg( alu, tmp );
		alu_error( ret );
		return ret;
	}
	
	ret = alu_cmp( alu, val, tmp, &cmp, &bit );
	
	(void)alu_rem_reg( alu, tmp );
	
	if ( ret != 0 )
	{
		alu_error( ret );
		return ret;
	}
	
	bits.mem.block = &by;
	if ( left )
	{
		if ( cmp >= 0 )
			return alu__shl( alu, num, -1 );
		
		ret = alu_mov( alu, (uintptr_t)(&bits), val );
		
		if ( ret )
		{
			alu_error( ret );
			return ret;
		}
		
		return alu__shl( alu, num, by );
	}
	if ( cmp >= 0 )
		return alu__shr( alu, num, -1 );
	
	ret = alu_mov( alu, (uintptr_t)(&bits), val );
	
	if ( ret )
	{
		alu_error( ret );
		return ret;
	}
	
	return alu__shr( alu, num, by );
}

int alu_mul( alu_t *alu, uint_t num, uint_t val )
{
	bool carry = 0;
	uint_t tmp = -1;
	int ret = alu_check2( alu, num, val );
	alu_reg_t *V;
	alu_bit_t p, v, e;
	
	if ( ret != 0 )
		return ret;
	
	ret = alu_get_reg( alu, &tmp, sizeof(size_t) );
	
	if ( ret != 0 )
		return ret;
	
	V = alu->regv + val;
	p = v = V->init;
	e = V->upto;
	
	alu__or( alu, tmp, num );
	alu_xor( alu, num, num );
	
	for ( ; v.b < e.b; v = alu_bit_inc( v ) )
	{
		if ( *(v.S) & v.B )
		{	
			(void)alu__shl( alu, tmp, v.b - p.b );
			ret = alu_add( alu, num, tmp );
			p = v;
			
			if ( ret == EOVERFLOW )
				carry = true;
			else if ( ret == ENODATA )
				break;
			else if ( ret != 0 )
			{
				alu_error( ret );
				break;
			}
		}
	}
	
	alu_rem_reg( alu, tmp );
	
	return carry ? EOVERFLOW : ret;
}

int alu_neg( alu_t *alu, uint_t num )
{
	int ret = alu_not( alu, num );
	
	if ( ret != 0 )
		return ret;
	
	return alu_inc( alu, num );
}

int alu_divide( alu_t *alu, uint_t num, uint_t val, uint_t rem )
{
	int ret = alu_check3( alu, num, val, rem );
	alu_reg_t *N, *V, *R, TR;
	alu_bit_t n;
	size_t bits = 0;
	bool nNeg, vNeg;
	
	if ( ret != 0 )
		return ret;
		
	N = alu->regv + num;
	V = alu->regv + val;
	R = alu->regv + rem;
	
	TR = *R;
	
	nNeg = ((N->info & ALU_REG_F__SIGN) && (*(N->last.S) & N->last.B));
	vNeg = ((V->info & ALU_REG_F__SIGN) && (*(V->last.S) & V->last.B));
	
	if ( nNeg )
		(void)alu_neg( alu, num );
	
	if ( vNeg )
		(void)alu_neg( alu, val );
	
	(void)alu_zero( alu, rem );
	(void)alu__or( alu, rem, num );
	(void)alu_zero( alu, num );

	n = alu_end_bit( *R );
	
	for (
		; alu_compare( TR, *V, NULL ) >= 0
		; ++bits, n = alu_bit_dec(n)
	)
	{
		R->init = n;
		if ( alu_compare( *R, *V, NULL ) >= 0 )
		{
			ret = alu_sub( alu, rem, val );
			
			if ( ret == ENODATA )
				break;
			
			(void)alu__shl( alu, num, bits );
			*(N->init.S) |= N->init.B;
			bits = 0;
		}
	}
	
	if ( R->init.b )
		ret = alu__shl( alu, num, bits + 1 );
	
	*R = TR;
	
	if ( nNeg != vNeg )
		(void)alu_neg( alu, num );
	
	if ( nNeg )
		(void)alu_neg( alu, rem );
	
	if ( vNeg )
		(void)alu_neg( alu, val );
	
	return ret;
}

int alu_div( alu_t *alu, uint_t num, uint_t val )
{
	uint_t tmp = -1;
	int ret = alu_get_reg( alu, &tmp, sizeof(size_t) );
	
	if ( ret != 0 )
	{
		alu_error( ret );
		return ret;
	}
	
	ret = alu_divide( alu, num, val, tmp );
	alu_rem_reg( alu, tmp );
	
	return ret;
}

int alu_rem( alu_t *alu, uint_t num, uint_t val )
{
	uint_t tmp = -1;
	int ret = alu_get_reg( alu, &tmp, sizeof(size_t) );
	alu_reg_t *N, *T;
	
	if ( ret != 0 )
	{
		alu_error( ret );
		return ret;
	}
	
	ret = alu_divide( alu, num, val, tmp );
	
	switch ( ret )
	{
	case 0: case ENODATA: case EOVERFLOW: break;
	default:
		alu_rem_reg( alu, tmp );
		alu_error( ret );
		return ret;
	}
	
	N = alu->regv + num;
	T = alu->regv + tmp;
	(void)memcpy( N->part, T->part, alu->buff.perN );
	
	(void)alu_rem_reg( alu, tmp );
	
	return ret;
}

int alu__rol( alu_t *alu, uint_t num, size_t by )
{
	alu_reg_t *N, *T;
	alu_bit_t n = {0}, v;
	int ret = alu_check1( alu, num );
	uint_t tmp = -1;
	
	if ( ret != 0 )
	{
		alu_error( ret );
		return ret;
	}
	
	if ( !by )
		return 0;
	
	ret = alu_get_reg( alu, &tmp, sizeof(size_t) );
	
	if ( ret != 0 )
	{
		alu_error( ret );
		return ret;
	}
	
	N = alu->regv + num;
	T = alu->regv + tmp;
	
	memcpy( T->part, N->part, alu->buff.perN );
	(void)alu_zero( alu, num );
	
	T->upto = alu_bit_set_bit( T->part, N->upto.b );
	T->last = alu_bit_set_bit( T->part, N->last.b );
	T->init = alu_bit_set_bit( T->part, N->init.b );
	
	n = N->upto;
	v = N->init;
	by %= (n.b - v.b);
	v = alu_bit_set_bit( T->part, n.b - by );
	
	while ( v.b > N->init.b )
	{
		v = alu_bit_dec( v );
		n = alu_bit_dec( n );
		
		if ( *(v.S) & v.B )
			*(n.S) |= n.B;
	}
	
	v = T->upto;
	while ( n.b > N->init.b )
	{
		v = alu_bit_dec( v );
		n = alu_bit_dec( n );
		
		if ( *(v.S) & v.B )
			*(n.S) |= n.B;
	}
	
	(void)alu_rem_reg( alu, tmp );
	
	return 0;
}

int alu__ror( alu_t *alu, uint_t num, size_t by )
{
	alu_reg_t *N, *T;
	alu_bit_t n = {0}, v, e;
	int ret = alu_check1( alu, num );
	uint_t tmp = -1;
	
	if ( ret != 0 )
	{
		alu_error( ret );
		return ret;
	}
	
	if ( !by )
		return 0;
	
	ret = alu_get_reg( alu, &tmp, sizeof(size_t) );
	
	if ( ret != 0 )
	{
		alu_error( ret );
		return ret;
	}
	
	N = alu->regv + num;
	T = alu->regv + tmp;
	
	memcpy( T->part, N->part, alu->buff.perN );
	(void)alu_zero( alu, num );
	
	T->upto = alu_bit_set_bit( T->part, N->upto.b );
	T->last = alu_bit_set_bit( T->part, N->last.b );
	T->init = alu_bit_set_bit( T->part, N->init.b );
	
	n = N->init;
	e = N->upto;
	by %= (e.b - n.b);
	v = alu_bit_set_bit( T->part, n.b + by );
	
	while ( v.b < e.b )
	{	
		if ( *(v.S) & v.B )
			*(n.S) |= n.B;
			
		v = alu_bit_inc( v );
		n = alu_bit_inc( n );
	}
	
	v = T->init;
	while ( n.b < e.b )
	{
		if ( *(v.S) & v.B )
			*(n.S) |= n.B;
		
		v = alu_bit_inc( v );
		n = alu_bit_inc( n );
	}
	
	(void)alu_rem_reg( alu, tmp );
	
	return 0;
}

int alu__rotate( alu_t *alu, uint_t num, uint_t val, bool left )
{
	alu_reg_t *N;
	int ret = alu_check2( alu, num, val ), cmp;
	uint_t tmp = -1;
	size_t by = 0, bit;
	alu_uint_t bits = {0};
	
	if ( ret != 0 )
	{
		alu_error( ret );
		return ret;
	}
		
	ret = alu_get_reg( alu, &tmp, sizeof(size_t) );
	
	if ( ret != 0 )
	{
		alu_error( ret );
		return ret;
	}
	
	N = alu->regv + num;
	
	bits.perN = sizeof(size_t);
	bits.qty.upto = bits.qty.used = 1;
	bits.qty.last = 0;
	bits.mem.block = &(N->upto.b);
	bits.mem.bytes.upto = bits.mem.bytes.used = sizeof(size_t);
	bits.mem.bytes.last = sizeof(size_t) - 1;
	ret = alu_mov( alu, tmp, (uintptr_t)(&bits) );
	
	if ( ret != 0 )
	{
		(void)alu_rem_reg( alu, tmp );
		alu_error( ret );
		return ret;
	}
	
	ret = alu_cmp( alu, val, tmp, &cmp, &bit );
	
	(void)alu_rem_reg( alu, tmp );
	
	if ( ret != 0 )
	{
		alu_error( ret );
		return ret;
	}
	
	bits.mem.block = &by;
	if ( left )
	{
		if ( cmp >= 0 )
			return alu__rol( alu, num, -1 );
		
		ret = alu_mov( alu, (uintptr_t)(&bits), val );
		
		if ( ret )
		{
			alu_error( ret );
			return ret;
		}
		
		return alu__rol( alu, num, by );
	}
	if ( cmp >= 0 )
		return alu__ror( alu, num, -1 );
	
	ret = alu_mov( alu, (uintptr_t)(&bits), val );
	
	if ( ret )
	{
		alu_error( ret );
		return ret;
	}
	
	return alu__ror( alu, num, by );
}

int alu_upto( alu_t *alu, uint_t num, uint_t val, alu_bit_t *pos )
{
	int ret = alu_check2( alu, num, val );
	alu_reg_t *N, *V;
	size_t ndiff, vdiff;
	
	if ( ret != 0 )
	{
		alu_error( ret );
		return ret;
	}
	
	if ( !pos )
	{
		ret = EDESTADDRREQ;
		alu_error( ret );
		return ret;
	}
	
	N = alu->regv + num;
	V = alu->regv + val;
	
	ndiff = (N->upto.b - N->init.b);
	vdiff = (V->upto.b - V->init.b);
	
	*pos = alu_bit_set_bit(
		N->part,
		N->init.b + ((ndiff < vdiff) ? ndiff : vdiff)
	);
	
	return 0;
}

int alu_and( alu_t *alu, uint_t num, uint_t val )
{
	alu_bit_t n, v, e = {0};
	int ret = alu_upto( alu, num, val, &e );
	alu_reg_t *N, *V;
	
	if ( ret != 0 )
	{
		alu_error( ret );
		return ret;
	}
	
	N = alu->regv + num;
	V = alu->regv + val;
	
	for
	(
		n = N->init, v = V->init;
		n.b < e.b;
		n = alu_bit_inc( n ), v = alu_bit_inc( v )
	)
	{
		if ( !(*(v.S) & v.B) )
			*(n.S) &= ~(n.B);
	}
	
	while ( n.b < N->upto.b )
	{
		*(n.S) &= ~(n.B);
		n = alu_bit_inc(n);
	}
	
	return 0;
}

int alu__or( alu_t *alu, uint_t num, uint_t val )
{
	alu_bit_t n, v, e = {0};
	int ret = alu_upto( alu, num, val, &e );
	alu_reg_t *N, *V;
	
	if ( ret != 0 )
	{
		alu_error( ret );
		return ret;
	}
	
	N = alu->regv + num;
	V = alu->regv + val;
	
	for
	(
		n = N->init, v = V->init;
		n.b < e.b;
		n = alu_bit_inc( n ), v = alu_bit_inc( v )
	)
	{
		*(n.S) |= (*(v.S) & v.B) ? n.B : SIZE_T_C(0);
	}
	
	return 0;
}

int alu_xor( alu_t *alu, uint_t num, uint_t val )
{
	alu_bit_t n, v, e = {0};
	int ret = alu_upto( alu, num, val, &e );
	alu_reg_t *N, *V;
	
	if ( ret != 0 )
	{
		alu_error( ret );
		return ret;
	}
	
	N = alu->regv + num;
	V = alu->regv + val;
	
	for
	(
		n = N->init, v = V->init;
		n.b < e.b;
		n = alu_bit_inc( n ), v = alu_bit_inc( v )
	)
	{
		*(n.S) ^= (*(v.S) & v.B) ? n.B : SIZE_T_C(0);
	}
	
	return 0;
}
