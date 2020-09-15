#include "alu.h"

int_t alu_reg_fill( alu_t *alu, alu_reg_t num, bool fillwith )
{
	void *part;
	alu_bit_t n;
	
	/* Branching takes longer to complete so assume intended node is
	 * remainder of division, prevents segfault */
	 
	num.node %= alu_used( alu );
	part = alu_reg_data( alu, num );
	
	/* Force to either 1 or 0 */
	fillwith = !!fillwith;
	
	for
	(
		n = alu_bit_set_bit( part, num.from )
		; n.b < num.upto
		; n = alu_bit_inc(n)
	)
	{
		/* Set bit to 0 */
		*(n.S) &= ~(n.B);
		
		/* Set bit based on fillwith */
		*(n.S) |= (fillwith * n.B);
	}
	
	return 0;
}

int_t alu_fill( alu_t *alu, uint_t num, bool fillwith )
{
	alu_reg_t _num;
	alu_reg_init( alu, _num, num, 0 );
	return alu_reg_fill( alu, _num, fillwith );
}

int_t alu_reg_get_raw
(
	alu_t *alu
	, alu_reg_t num
	, void *raw
	, size_t size
)
{
	void *part;
	alu_bit_t n, v, e;
	bool neg;
	
	if ( raw )
	{
		num.node %= alu_used( alu );
		
		neg = alu_reg_below0( alu, num );
		
		n = alu_bit_set_byte( raw, 0 );
		
		part = alu_reg_data( alu, num );
		v = alu_bit_set_bit( part, num.from );
		
		e = alu_bit_set_bit
		(
			raw
			, LOWEST( size * CHAR_BIT, num.upto - num.from )
		);
		
		for
		(
			; n.b < e.b
			; n = alu_bit_inc(n), v = alu_bit_inc(v)
		)
		{
			*(n.S) &= ~(n.B);
			*(n.S) |= (!!(*(v.S) & v.B) * n.B);
		}
		
		e = alu_bit_set_byte( raw, size );
		
		for ( ; n.b < e.b; n = alu_bit_inc(n) )
		{
			*(n.S) &= ~(n.B);
			*(n.S) |= neg * n.B;
		}
		
		return 0;
	}
		
	return alu_err_null_ptr("raw");
}

int_t alu_get_raw( alu_t *alu, uint_t num, size_t *raw )
{
	alu_reg_t _num;
	alu_reg_init( alu, _num, num, 0 );
	return alu_reg_get_raw( alu, _num, raw, sizeof(size_t) );
}

int_t alu_reg_set_raw
(
	alu_t *alu
	, alu_reg_t num
	, void *raw
	, uint_t info
	, size_t size
)
{
	alu_bit_t n, v, e;
	void *part;
	bool neg;
	
	size = SET2IF( size, size, 1 );
	num.node %= alu_used( alu );
	
	e = alu_bit_set_bit( raw, (size * CHAR_BIT) - 1 );
	neg = !!(info & ALU_INFO__SIGN) & !!(*(e.S) & e.B);
	
	part = alu_reg_data( alu, num );
	n = alu_bit_set_bit( part, num.from );
	v = alu_bit_set_bit( raw, 0 );
	e = alu_bit_set_bit
	(
		part
		, num.from + LOWEST( size * CHAR_BIT, num.upto - num.from )
	);
	
	for ( ; n.b < e.b; n = alu_bit_inc(n), v = alu_bit_inc( v ) )
	{
		*(n.S) &= ~(n.B);
		*(n.S) |= (!!(*(v.S) & v.B) * n.B);
	}
	
	for ( v = alu_bit_dec(v); n.b < num.upto; n = alu_bit_inc(n) )
	{
		*(n.S) &= ~(n.B);
		*(n.S) |= (neg * n.B);
	}
	
	return 0;
}

int_t alu_set_raw( alu_t *alu, uint_t num, size_t raw, uint_t info )
{
	alu_reg_t _num;
	alu_reg_init( alu, _num, num, 0 );
	return alu_reg_set_raw( alu, _num, &raw, info, sizeof(size_t) );
}

int alu_reg_copy(
	alu_t *alu,
	alu_reg_t num,
	alu_reg_t val
)
{
	void *num_part, *val_part;
	alu_bit_t n = {0}, v, e;
	
	if ( alu )
	{
		num.node %= alu_used( alu );
		val.node %= alu_used( alu );
	
		num_part = alu_reg_data( alu, num );
		val_part = alu_reg_data( alu, val );
		
		n = alu_bit_set_bit( num_part, num.from );
		v = alu_bit_set_bit( val_part, val.from );
				
		e = alu_bit_set_bit(
			num_part
			, (num.upto * (num.upto <= val.upto))
			| (val.upto * (val.upto <= num.upto))
		);
		
		for ( ; n.b < e.b; n = alu_bit_inc(n), v = alu_bit_inc(v) )
		{
			*(n.S) &= ~(n.B);
			*(n.S) |= n.B * !!(*v.S & v.B);
		}
		
		for ( ; n.b < num.upto; n = alu_bit_inc(n) )
		{
			*(n.S) &= ~(n.B);
		}
		
		return 0;
	}
	
	return alu_err_null_ptr( "alu" );
}

int_t alu_copy( alu_t *alu, uint_t num, uint_t val )
{	
	alu_reg_t _num, _val;
	alu_reg_init( alu, _num, num, 0 );
	alu_reg_init( alu, _val, val, 0 );
	
	return alu_reg_copy( alu, _num, _val );
}

alu_bit_t alu_reg_end_bit( alu_t *alu, alu_reg_t num )
{
	alu_bit_t n;
	size_t b;
	void *part;
	
	num.node %= alu_used( alu );
	
	part = alu_reg_data( alu, num );
	n = alu_bit_set_bit( part, num.upto - 1 );
	b = n.b * !( *(n.S) & n.B );
	
	while ( b > num.from )
	{
		n = alu_bit_dec( n );
		b = SET1IF( !( *(n.S) & n.B ), n.b );
	}
	
	return n;
}

alu_bit_t alu_end_bit( alu_t *alu, uint_t num )
{
	alu_reg_t _num;
	alu_reg_init( alu, _num, num, 0 );
	return alu_reg_end_bit( alu, _num );
}

int_t alu_reg_cmp(
	alu_t *alu
	, alu_reg_t num
	, alu_reg_t val
	, size_t *bit
)
{
	int a, b, c = 0;
	alu_bit_t n = {0}, v = {0};
	size_t ndiff = 0, vdiff = 0;
	bool nNeg, vNeg;
	
	n = alu_reg_end_bit( alu, num );
	v = alu_reg_end_bit( alu, val );
	
	nNeg = alu_reg_signed( num ) & (n.b == num.upto - 1);
	vNeg = alu_reg_signed( val ) & (v.b == val.upto - 1);
	
#if 0
	alu_print_reg( "num", alu, num, 1, 1 );
	alu_print_reg( "val", alu, val, 1, 1 );
	alu_print_bit( "n", n, 1 );
	alu_print_bit( "v", v, 1 );
#endif
	
	if ( nNeg != vNeg )
	{
		c = -1 + vNeg + vNeg;
		goto done;
	}
	
	ndiff = n.b - num.from;
	vdiff = v.b - val.from;
	
	a = nNeg;
	b = vNeg;
	
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
	
	/* Finally compare what matches bit alignment */
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

int_t alu_cmp( alu_t *alu, uint_t num, uint_t val, size_t *bit )
{
	alu_reg_t _num, _val;
	
	alu_reg_init( alu, _num, num, 0 );
	alu_reg_init( alu, _val, val, 0 );
	
	return alu_reg_cmp( alu, _num, _val, bit );
}

int_t alu_size2reg( alu_t *alu, uint_t num, size_t val )
{
	alu_reg_t _num;
	
	alu_reg_init( alu, _num, num, 0 );
	
	return alu_reg_set_raw( alu, _num, &val, 0, sizeof(size_t) );
}

int_t alu_reg2size( alu_t *alu, uint_t num, size_t *val )
{
	alu_reg_t _num;
	
	alu_reg_init( alu, _num, num, 0 );
	
	return alu_reg_get_raw( alu, _num, val, sizeof(size_t) );
}

int_t alu_reg_not( alu_t *alu, alu_reg_t num )
{
	alu_bit_t n = {0};
	void *part;
	
	if ( alu )
	{
		num.node %= alu_used( alu );
		
		part = alu_reg_data( alu, num );
		n = alu_bit_set_bit( part, num.from );
		
		for ( ; n.b < num.upto; n = alu_bit_inc( n ) )
		{
			*(n.S) ^= n.B;
		}
	
		return 0;
	}
	
	return alu_err_null_ptr( "alu" );
}


int_t alu_not( alu_t *alu, uint_t num )
{
	alu_reg_t _num;
	
	alu_reg_init( alu, _num, num, 0 );
	
	return alu_reg_not( alu, _num );
}

int_t alu_reg_inc( alu_t *alu, alu_reg_t num )
{
	bool carry = 0;
	alu_bit_t n;
	void *part;
	
	num.node %= alu_used( alu );
	part = alu_reg_data( alu, num );
	n = alu_bit_set_bit( part, num.from );
	
	if ( *(n.S) & n.B )
	{
		*(n.S) ^= n.B;
		carry = true;
		for (
			n = alu_bit_inc( n );
			n.b < num.upto;
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

int_t alu_inc( alu_t *alu, uint_t num )
{
	alu_reg_t _num;
	
	alu_reg_init( alu, _num, num, 0 );
	
	return alu_reg_inc( alu, _num );
}

int_t alu_reg_dec( alu_t *alu, alu_reg_t num )
{
	bool carry = 0;
	alu_bit_t n;
	void *part;
	
	num.node %= alu_used( alu );
	
	part = alu_reg_data( alu, num );
	n = alu_bit_set_bit( part, num.from );
	
	if ( *(n.S) & n.B )
		*(n.S) ^= n.B;
	else
	{
		*(n.S) |= n.B;
		carry = true;
		for (
			n = alu_bit_inc( n );
			n.b < num.upto;
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

int_t alu_dec( alu_t *alu, uint_t num )
{
	alu_reg_t _num;
	
	alu_reg_init( alu, _num, num, 0 );
	
	return alu_reg_dec( alu, _num );
}

int_t alu_reg_add( alu_t *alu, alu_reg_t num, alu_reg_t val )
{
	bool carry = false, changed = false;
	alu_bit_t n, v = {0};
	size_t pos = 0;
	void *part;
	
	num.node %= alu_used( alu );
	val.node %= alu_used( alu );
	
	pos = alu_lowest_upto( num, val );
	
	part = alu_reg_data( alu, num );
	n = alu_bit_set_bit( part, num.from );
	
	part = alu_reg_data( alu, val );
	v = alu_bit_set_bit( part, val.from );
	
	for
	(
		; n.b < pos
		; n = alu_bit_inc( n ), v = alu_bit_inc( v )
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
	
	if ( carry )
	{
		for
		(
			; n.b < num.upto
			; n = alu_bit_inc( n )
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
	
	return changed ? (carry ? EOVERFLOW : 0) : ENODATA;
}

int_t alu_add( alu_t *alu, uint_t num, uint_t val )
{
	alu_reg_t _num, _val;
	
	alu_reg_init( alu, _num, num, 0 );
	alu_reg_init( alu, _val, val, 0 );
	
	return alu_reg_add( alu, _num, _val );
}

int_t alu_reg_sub( alu_t *alu, alu_reg_t num, alu_reg_t val )
{
	bool carry = false, changed = false;
	alu_bit_t n, v;
	size_t pos = 0;
	void *part;
	
	num.node %= alu_used( alu );
	val.node %= alu_used( alu );
	
	pos = alu_lowest_upto( num, val );
	
	part = alu_reg_data( alu, num );
	n = alu_bit_set_bit( part, num.from );
	
	part = alu_reg_data( alu, val );
	v = alu_bit_set_bit( part, val.from );
	
	for
	(
		; n.b < pos
		; n = alu_bit_inc( n ), v = alu_bit_inc( v )
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
	
	if ( carry )
	{
		for
		(
			; n.b < num.upto
			; n = alu_bit_inc( n )
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
	
	return changed ? (carry ? EOVERFLOW : 0) : ENODATA;
}

int_t alu_sub( alu_t *alu, uint_t num, uint_t val )
{
	alu_reg_t _num, _val;
	
	alu_reg_init( alu, _num, num, 0 );
	alu_reg_init( alu, _val, val, 0 );
	
	return alu_reg_sub( alu, _num, _val );
}

int_t alu_reg__shl( alu_t *alu, alu_reg_t num, size_t by )
{
	alu_bit_t n = {0}, v;
	void *part;
	
	if ( by )
	{
		num.node %= alu_used( alu );
		
		part = alu_reg_data( alu, num );
		n = alu_bit_set_bit( part, num.upto );
		v = alu_bit_set_bit( part, num.from );
		
		if ( by < (n.b - v.b) )
			v = alu_bit_set_bit( part, n.b - by );
		
		while ( v.b > num.from )
		{
			n = alu_bit_dec( n );
			v = alu_bit_dec( v );
		
			*(n.S) &= ~(n.B);
			
			if ( *(v.S) & v.B )
				*(n.S) |= n.B;
		}

		while ( n.b > num.from )
		{
			n = alu_bit_dec( n );
			
			*(n.S) &= ~(n.B);
		}
	}
	
	return 0;
}

int_t alu__shl( alu_t *alu, uint_t num, size_t by )
{
	alu_reg_t _num;
	
	alu_reg_init( alu, _num, num, 0 );
	
	return alu_reg__shl( alu, _num, by );
}

int_t alu_reg__shr( alu_t *alu, alu_reg_t num, size_t by )
{
	alu_bit_t n, v, e;
	void *part;
	bool neg;
	
	if ( by )
	{
	
		num.node %= alu_used( alu );
		
		part = alu_reg_data( alu, num );
		
		n = alu_bit_set_bit( part, num.from );
		e = v = alu_bit_set_bit( part, num.upto );
		
		if ( by < (e.b - n.b) )
			v = alu_bit_set_bit( part, n.b + by );
		
		neg = alu_reg_below0( alu, num );
		
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
			*(n.S) |= (neg * n.B);
			
			n = alu_bit_inc( n );
		}
	
	}
	
	return 0;
}

int_t alu__shr( alu_t *alu, uint_t num, size_t by )
{
	alu_reg_t _num;
	
	alu_reg_init( alu, _num, num, 0 );
	
	return alu_reg__shr( alu, _num, by );
}

int_t alu_reg__shift
(
	alu_t *alu,
	alu_reg_t num,
	alu_reg_t val,
	alu_reg_t tmp,
	bool left
)
{
	int cmp;
	size_t by = 0, ndiff = num.upto - num.from;
	int_t (*shift)( alu_t*, alu_reg_t, size_t )
		= (int_t (*)( alu_t*, alu_reg_t, size_t ))
		(((uintptr_t)alu_reg__shl * left) | ((uintptr_t)alu_reg__shr * !left));
		
	num.node %= alu_used( alu );
	val.node %= alu_used( alu );
	tmp.node %= alu_used( alu );
	
	alu_reg_set_raw( alu, tmp, &ndiff, tmp.info, sizeof(size_t) );
	cmp = alu_reg_cmp( alu, val, tmp, NULL );
	
	if ( cmp >= 0 )
	{
		return shift( alu, num, -1 );
	}
	
	alu_reg_get_raw( alu, val, &by, sizeof(size_t) );
	return shift( alu, num, by );
}

int_t alu__shift( alu_t *alu, uint_t num, uint_t val, uint_t tmp, bool left )
{
	alu_reg_t _num, _val, _tmp;
	
	alu_reg_init( alu, _num, num, 0 );
	alu_reg_init( alu, _val, val, 0 );
	alu_reg_init( alu, _tmp, tmp, 0 );
	
	return alu_reg__shift( alu, _num, _val, _tmp, left );
}

int_t alu_reg_mul
(
	alu_t *alu
	, alu_reg_t num
	, alu_reg_t val
	, alu_reg_t tmp
)
{
	int ret = 0;
	bool carry = 0;
	alu_bit_t p, v;
	void *part;
	
	num.node %= alu_used( alu );
	val.node %= alu_used( alu );
	tmp.node %= alu_used( alu );
	
	part = alu_reg_data( alu, val );
	p = v = alu_bit_set_bit( part, val.from );
	
	alu_reg_copy( alu, tmp, num );
	alu_reg_set_nil( alu, num );
	
	for ( ; v.b < val.upto; v = alu_bit_inc( v ) )
	{
		if ( *(v.S) & v.B )
		{	
			(void)alu_reg__shl( alu, tmp, v.b - p.b );
			ret = alu_reg_add( alu, num, tmp );
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
	
	return carry ? EOVERFLOW : ret;
}

int_t alu_mul( alu_t *alu, uint_t num, uint_t val, uint_t tmp )
{
	alu_reg_t _num, _val, _tmp;
	
	alu_reg_init( alu, _num, num, 0 );
	alu_reg_init( alu, _val, val, 0 );
	alu_reg_init( alu, _tmp, tmp, 0 );
	
	return alu_reg_mul( alu, _num, _val, _tmp );
}

int_t alu_reg_neg( alu_t *alu, alu_reg_t num )
{
	int ret = alu_reg_not( alu, num );
	
	if ( ret == 0 )
		return alu_reg_inc( alu, num );
		
	alu_error(ret);
	return ret;
}

int_t alu_neg( alu_t *alu, uint_t num )
{
	alu_reg_t _num;
	
	alu_reg_init( alu, _num, num, 0 );
	
	return alu_reg_neg( alu, _num );
}

int_t alu_reg_divide
(
	alu_t *alu
	, alu_reg_t num
	, alu_reg_t val
	, alu_reg_t rem
)
{
	int ret = 0;
	alu_reg_t tmp;
	alu_bit_t n;
	size_t bits = 0;
	bool nNeg, vNeg;
	void *num_part, *val_part;
	
	num.node %= alu_used( alu );
	val.node %= alu_used( alu );
	rem.node %= alu_used( alu );
	
	num_part = alu_reg_data( alu, num );
	val_part = alu_reg_data( alu, val );
	
	tmp = rem;
	
	n = alu_bit_set_bit( num_part, num.upto - 1 );
	nNeg = alu_reg_below0( alu, num );
	
	n = alu_bit_set_bit( val_part, val.upto - 1 );
	vNeg = alu_reg_below0( alu, val );
	
	if ( nNeg )
		alu_reg_neg( alu, num );
	
	if ( vNeg )
		alu_reg_neg( alu, val );
	
	(void)alu_reg_copy( alu, rem, num );
	(void)alu_reg_set_nil( alu, num );

	n = alu_reg_end_bit( alu, rem );
	tmp.upto = tmp.from = n.b + 1;
	n = alu_bit_set_bit( num_part, num.from );
	
	for ( ; alu_reg_cmp( alu, rem, val, NULL ) >= 0; ++bits )
	{
		tmp.from--;
		if ( alu_reg_cmp( alu, tmp, val, NULL ) >= 0 )
		{
			ret = alu_reg_sub( alu, tmp, val );
			
			if ( ret == ENODATA )
				break;
			
			alu_reg__shl( alu, num, bits );
			*(n.S) |= n.B;
			bits = 0;
		}
	}
	
	ret = 0;
	if ( tmp.from > rem.from )
		alu_reg__shl( alu, num, bits + 1 );
	
	if ( nNeg != vNeg )
		alu_reg_neg( alu, num );
	
	if ( nNeg )
		alu_reg_neg( alu, rem );
	
	if ( vNeg )
		alu_reg_neg( alu, val );
	
	return ret;
}

int_t alu_divide( alu_t *alu, uint_t num, uint_t val, uint_t rem )
{
	alu_reg_t _num, _val, _rem;
	
	alu_reg_init( alu, _num, num, 0 );
	alu_reg_init( alu, _val, val, 0 );
	alu_reg_init( alu, _rem, rem, 0 );
	
	return alu_reg_divide( alu, _num, _val, _rem );
}

int_t alu_reg_div
(
	alu_t *alu
	, alu_reg_t num
	, alu_reg_t val
	, alu_reg_t tmp
)
{
	return alu_reg_divide( alu, num, val, tmp );
}

int_t alu_div( alu_t *alu, uint_t num, uint_t val, uint_t tmp )
{
	alu_reg_t _num, _val, _tmp;
	
	alu_reg_init( alu, _num, num, 0 );
	alu_reg_init( alu, _val, val, 0 );
	alu_reg_init( alu, _tmp, tmp, 0 );
	
	return alu_reg_divide( alu, _num, _val, _tmp );
}

int_t alu_reg_rem
(
	alu_t *alu
	, alu_reg_t num
	, alu_reg_t val
	, alu_reg_t tmp
)
{
	int ret = alu_reg_divide( alu, num, val, tmp );
	
	(void)alu_reg_copy( alu, num, tmp );
	
	return ret;
}

int_t alu_rem( alu_t *alu, uint_t num, uint_t val, uint_t tmp )
{	
	alu_reg_t _num, _val, _tmp;
	
	alu_reg_init( alu, _num, num, 0 );
	alu_reg_init( alu, _val, val, 0 );
	alu_reg_init( alu, _tmp, tmp, 0 );
	
	return alu_reg_div( alu, _num, _val, _tmp );
}

int_t alu_reg__rol( alu_t *alu, alu_reg_t num, alu_reg_t tmp, size_t by )
{
	alu_bit_t n = {0}, v;
	void *num_part, *tmp_part;
	
	if ( !by )
		return 0;
	
	by %= (num.upto - num.from);
		
	num.node %= alu_used( alu );
	tmp.node %= alu_used( alu );
	
	tmp.info = num.info;
	tmp.from = num.from;
	tmp.upto = num.upto;
	
	num_part = alu_reg_data( alu, num );
	tmp_part = alu_reg_data( alu, tmp );
	
	(void)memcpy( tmp_part, num_part, alu_size_perN( alu ) );
	(void)memset( num_part, 0, alu_size_perN( alu ) );
	
	n = alu_bit_set_bit( num_part, num.upto );
	v = alu_bit_set_bit( tmp_part, tmp.upto - by );
	
	while ( v.b > tmp.from )
	{
		v = alu_bit_dec( v );
		n = alu_bit_dec( n );
		
		*(n.S) |= (n.B * !!(*(v.S) & v.B));
	}
	
	v = alu_bit_set_bit( tmp_part, tmp.upto );
	while ( n.b > num.from )
	{
		v = alu_bit_dec( v );
		n = alu_bit_dec( n );
		
		*(n.S) |= (n.B * !!(*(v.S) & v.B));
	}
	
	return 0;
}

int_t alu__rol( alu_t *alu, uint_t num, uint_t tmp, size_t by )
{
	alu_reg_t _num, _tmp;
	
	alu_reg_init( alu, _num, num, 0 );
	alu_reg_init( alu, _tmp, tmp, 0 );
	
	return alu_reg__rol( alu, _num, _tmp, by );
}

int_t alu_reg__ror( alu_t *alu, alu_reg_t num, alu_reg_t tmp, size_t by )
{
	alu_bit_t n, v;
	void *num_part, *tmp_part;
	
	if ( !by )
		return 0;
		
	by %= (num.upto - num.from);
		
	num.node %= alu_used( alu );
	tmp.node %= alu_used( alu );
	
	tmp.info = num.info;
	tmp.from = num.from;
	tmp.upto = num.upto;
	
	num_part = alu_reg_data( alu, num );
	tmp_part = alu_reg_data( alu, tmp );
	
	(void)memcpy( tmp_part, num_part, alu_size_perN( alu ) );
	(void)memset( num_part, 0, alu_size_perN( alu ) );
	
	n = alu_bit_set_bit( num_part, num.from );
	v = alu_bit_set_bit( tmp_part, tmp.from + by );
	
	while ( v.b < tmp.upto )
	{	
		*(n.S) |= (n.B * !!(*(v.S) & v.B));

		v = alu_bit_inc( v );
		n = alu_bit_inc( n );
	}
	
	v = alu_bit_set_bit( tmp_part, tmp.from );
	while ( n.b < num.upto )
	{
		*(n.S) |= (n.B * !!(*(v.S) & v.B));
		
		v = alu_bit_inc( v );
		n = alu_bit_inc( n );
	}
	
	return 0;
}

int_t alu__ror( alu_t *alu, uint_t num, uint_t tmp, size_t by )
{
	alu_reg_t _num, _tmp;
	
	alu_reg_init( alu, _num, num, 0 );
	alu_reg_init( alu, _tmp, tmp, 0 );
	
	return alu_reg__ror( alu, _num, _tmp, by );
}

int_t alu_reg__rotate
(
	alu_t *alu
	, alu_reg_t num
	, alu_reg_t val
	, alu_reg_t tmp
	, bool left
)
{
	int cmp;
	size_t by = 0, ndiff = num.upto - num.from;
	int_t (*rotate)( alu_t*, alu_reg_t, alu_reg_t, size_t )
		= (int_t (*)( alu_t*, alu_reg_t, alu_reg_t, size_t ))
		(((uintptr_t)alu_reg__rol * left) | ((uintptr_t)alu_reg__ror * !left));
	
	alu_reg_set_raw( alu, tmp, &ndiff, 0, sizeof(size_t) );
	cmp = alu_reg_cmp( alu, val, tmp, NULL );
	
	if ( cmp >= 0 )
	{
		return rotate( alu, num, tmp, -1 );
	}
	
	alu_reg_get_raw( alu, val, &by, sizeof(size_t) );
	return rotate( alu, num, tmp, by );
}

size_t alu_lowest_upto( alu_reg_t num, alu_reg_t val )
{
	size_t ndiff, vdiff;
	
	ndiff = (num.upto - num.from);
	vdiff = (val.upto - val.from);
		
	return num.from + LOWEST(ndiff,vdiff);
}

int_t alu_reg_and( alu_t *alu, alu_reg_t num, alu_reg_t val )
{
	alu_bit_t n, v;
	size_t pos = 0;
	void *part;
	
	num.node %= alu_used( alu );
	val.node %= alu_used( alu );
	
	pos = alu_lowest_upto( num, val );
	
	part = alu_reg_data( alu, num );
	n = alu_bit_set_bit( part, num.from );
	
	part = alu_reg_data( alu, val );
	v = alu_bit_set_bit( part, val.from );
	
	for
	(
		; n.b < pos
		; n = alu_bit_inc( n ), v = alu_bit_inc( v )
	)
	{
		/* TODO: Do branchless version of this */
		if ( !(*(v.S) & v.B) )
			*(n.S) &= ~(n.B);
	}
	
	while ( n.b < num.upto )
	{
		*(n.S) &= ~(n.B);
		n = alu_bit_inc(n);
	}
	
	return 0;
}

int_t alu_and( alu_t *alu, uint_t num, uint_t val )
{
	alu_reg_t _num, _val;
	
	alu_reg_init( alu, _num, num, 0 );
	alu_reg_init( alu, _val, val, 0 );
	
	return alu_reg_and( alu, _num, _val );
}

int_t alu_reg__or( alu_t *alu, alu_reg_t num, alu_reg_t val )
{
	alu_bit_t n, v;
	size_t pos = 0;
	void *part;
	
	num.node %= alu_used( alu );
	val.node %= alu_used( alu );
	
	pos = alu_lowest_upto( num, val );
	
	part = alu_reg_data( alu, num );
	n = alu_bit_set_bit( part, num.from );
	
	part = alu_reg_data( alu, val );
	v = alu_bit_set_bit( part, val.from );
	
	for
	(
		; n.b < pos
		; n = alu_bit_inc( n ), v = alu_bit_inc( v )
	)
	{
		*(n.S) |= (*(v.S) & v.B) ? n.B : SIZE_T_C(0);
	}
	
	return 0;
}

int_t alu__or( alu_t *alu, uint_t num, uint_t val )
{
	alu_reg_t _num, _val;
	
	alu_reg_init( alu, _num, num, 0 );
	alu_reg_init( alu, _val, val, 0 );
	
	return alu_reg__or( alu, _num, _val );
}

int_t alu_reg_xor( alu_t *alu, alu_reg_t num, alu_reg_t val )
{
	alu_bit_t n, v;
	size_t pos = 0;
	void *part;
	
	num.node %= alu_used( alu );
	val.node %= alu_used( alu );
	
	pos = alu_lowest_upto( num, val );
	
	part = alu_reg_data( alu, num );
	n = alu_bit_set_bit( part, num.from );
	part = alu_reg_data( alu, val );
	v = alu_bit_set_bit( part, val.from );
	
	for
	(
		; n.b < pos
		; n = alu_bit_inc( n ), v = alu_bit_inc( v )
	)
	{
		*(n.S) ^= (*(v.S) & v.B) ? n.B : SIZE_T_C(0);
	}
	
	return 0;
}

int_t alu_xor( alu_t *alu, uint_t num, uint_t val )
{
	alu_reg_t _num, _val;
	
	alu_reg_init( alu, _num, num, 0 );
	alu_reg_init( alu, _val, val, 0 );
	
	return alu_reg_xor( alu, _num, _val );
}
