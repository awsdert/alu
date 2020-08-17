#include "alu.h"

int alu_zero( alu_t *alu, int num )
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

int alu_copy( alu_t *alu, int num, int val )
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

int alu_end_bit( alu_t *alu, int val, alu_bit_t *bit )
{
	int ret = alu_check_reg( alu, val );
	alu_reg_t *V;
	alu_bit_t v;
	
	if ( ret != 0 && ret != EADDRINUSE )
		return ret;
	
	V = alu->regv + val;
	v = V->upto;
	
	while ( v.b > V->init.b )
	{
		v = alu_bit_dec( v );
		
		if ( *(v.S) & v.B )
			break;
	}
	
	*bit = v;
	return 0;
}

int alu_cmp( alu_t *alu, int num, int val, int *cmp, size_t *bit )
{
	int ret = 0, a, b, c;
	alu_bit_t n = {0}, v = {0};
	alu_reg_t *N, *V;
	size_t ndiff, vdiff;
	bool nNeg, vNeg;
	
	if ( !cmp || !bit )
	{
		alu_error( EDESTADDRREQ );
		return EDESTADDRREQ;
	}
	
	*bit = 0;
	*cmp = 0;
	
	ret = alu_end_bit( alu, num, &n );
	
	if ( ret != 0 )
		return ret;
	
	ret = alu_end_bit( alu, val, &v );
	
	if ( ret != 0 )
		return ret;

	N = alu->regv + num;
	V = alu->regv + val;
	
	nNeg = ( N->info & ALU_REG_F__SIGN && n.b == N->last.b );
	vNeg = ( V->info & ALU_REG_F__SIGN && v.b == V->last.b );
	
	if ( nNeg != vNeg )
	{
		if ( vNeg )
		{
			*bit = n.b;
			*cmp = 1;
			return 0;
		}
		else
		{
			*bit = n.b;
			*cmp = -1;
			return 0;
		}
	}
	
	ndiff = n.b - N->init.b;
	vdiff = v.b - V->init.b;
	
	a = (*(n.S) & n.B) ? 1 : 0;
	b = (*(v.S) & v.B) ? 1 : 0;
	
	c = a - b;
	if ( c != 0 )
	{
		*bit = n.b;
		*cmp = c;
		return 0;
	}
	
	/* Deal with different sized integers */
	
	a = (*(n.S) & n.B) ? 1 : 0;
	while ( ndiff < vdiff )
	{
		vdiff--;
		v = alu_bit_dec( v );
		b = (*(v.S) & v.B) ? 1 : 0;
		
		c = a - b;
		if ( c != 0 )
		{
			*bit = n.b;
			*cmp = c;
			return 0;
		}
	}
	
	b = (*(v.S) & v.B) ? 1 : 0;
	while ( ndiff > vdiff )
	{
		ndiff--;
		n = alu_bit_dec( n );
		a = (*(n.S) & n.B) ? 1 : 0;
		
		c = a - b;
		if ( c != 0 )
		{
			*bit = n.b;
			*cmp = c;
			return 0;
		}
	}
	
	while ( ndiff )
	{
		ndiff--;
		n = alu_bit_dec( n );
		v = alu_bit_dec( v );
		
		a = (*(n.S) & n.B) ? 1 : 0;
		b = (*(v.S) & v.B) ? 1 : 0;
		
		c = a - b;
		if ( c != 0 )
		{
			*bit = n.b;
			*cmp = c;
			return 0;
		}
	}
	
	return 0;
}

int alu_size2reg( alu_t *alu, int num, size_t val )
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

int alu_reg2size( alu_t *alu, int num, size_t *val )
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

int alu_inc( alu_t *alu, int num )
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

int alu_dec( alu_t *alu, int num )
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

int alu_add( alu_t *alu, int num, int val )
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

int alu_sub( alu_t *alu, int num, int val )
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

int alu__shl( alu_t *alu, int num, size_t by )
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

int alu__shr( alu_t *alu, int num, size_t by )
{
	alu_reg_t *N;
	alu_bit_t n, v, e;
	int ret = alu_check1( alu, num );
	
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
		
		n = alu_bit_inc( n );
	}
	
	return 0;
}

int alu__shift( alu_t *alu, int num, int val, bool left )
{
	alu_reg_t *N;
	int ret = alu_check2( alu, num, val ), tmp = -1, cmp;
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

int alu_mul( alu_t *alu, int num, int val )
{
	bool carry = 0;
	int ret = alu_check2( alu, num, val ), tmp = -1;
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

int alu_divide( alu_t *alu, int num, int val, int rem )
{
	int ret = alu_check3( alu, num, val, rem ), cmp = 0;
	alu_reg_t *N, *V, *R, TR, TV;
	alu_bit_t n, v;
	size_t bits = 0, bit;
	
	if ( ret != 0 )
		return ret;
		
	N = alu->regv + num;
	R = alu->regv + rem;
	V = alu->regv + val;
	
	TR = *R;
	TV = *V;
	
	memcpy( R->part, N->part, alu->buff.perN );
	(void)alu_zero( alu, num );

	(void)alu_end_bit( alu, rem, &(n) );
	(void)alu_end_bit( alu, val, &(v) );
	
	R->upto = alu_bit_inc(n); R->last = n;
	V->upto = alu_bit_inc(v); V->last = v;
	
	for (
		R->init = n; R->init.b > TR.init.b;
		++bits, R->init = alu_bit_dec(R->init)
	)
	{
		(void)alu_cmp( alu, rem, val, &cmp, &bit );
		
		if ( cmp >= 0 )
		{			
			ret = alu_sub( alu, rem, val );
			
			if ( ret == ENODATA )
				break;
			
			(void)alu__shl( alu, num, bits );
			*(N->init.S) |= N->init.B;
			bits = 0;
			ret = 0;
		}
	}
	
	if ( bits )
	{
		(void)alu_cmp( alu, rem, val, &cmp, &bit );
		(void)alu__shl( alu, num, bits );
		
		if ( cmp >= 0 )
		{
			ret = alu_sub( alu, rem, val );
			*(N->init.S) |= N->init.B;
		}
	}
	
	*R = TR;
	*V = TV;
	
	return ret;
}

int alu_div( alu_t *alu, int num, int val )
{
	int tmp = -1, ret = alu_get_reg( alu, &tmp, sizeof(size_t) );
	
	if ( ret != 0 )
	{
		alu_error( ret );
		return ret;
	}
	
	ret = alu_divide( alu, num, val, tmp );
	alu_rem_reg( alu, tmp );
	
	return ret;
}

int alu_rem( alu_t *alu, int num, int val )
{
	int tmp = -1, ret = alu_get_reg( alu, &tmp, sizeof(size_t) );
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

int alu__rol( alu_t *alu, int num, size_t by )
{
	alu_reg_t *N, *T;
	alu_bit_t n = {0}, v;
	int ret = alu_check1( alu, num ), tmp = -1;
	
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

int alu__ror( alu_t *alu, int num, size_t by )
{
	alu_reg_t *N, *T;
	alu_bit_t n = {0}, v, e;
	int ret = alu_check1( alu, num ), tmp = -1;
	
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

int alu__rotate( alu_t *alu, int num, int val, bool left )
{
	alu_reg_t *N;
	int ret = alu_check2( alu, num, val ), tmp = -1, cmp;
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

int alu_upto( alu_t *alu, int num, int val, alu_bit_t *pos )
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

int alu_and( alu_t *alu, int num, int val )
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

int alu__or( alu_t *alu, int num, int val )
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

int alu_xor( alu_t *alu, int num, int val )
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
