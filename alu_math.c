#include "alu.h"

void alu_reg_fill( alu_t alu, alu_register_t num, bool fillwith )
{
	void *part;
	alu_bit_t n;
	
	/* Branching takes longer to complete so assume intended node is
	 * remainder of division, prevents segfault */
	 
	num.node %= ALU_USED( alu );
	part = ALU_PART( alu, num.node );
	
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
}

void alu_fill( alu_t alu, uint_t num, bool fillwith )
{
	num %= ALU_USED( alu );
	alu_reg_fill( alu, alu.regv[num], fillwith );
}

alu_bit_t alu_reg_get_raw(
	alu_t alu,
	alu_register_t num,
	size_t *raw
)
{
	void *part;
	alu_bit_t n = {0}, l;
	size_t b;
	bool neg;
	
	num.node %= ALU_USED( alu );
	
	part = ALU_PART( alu, num.node );
	n = alu_bit_set_bit( part, num.from );
	
	if ( raw )
	{
		*raw = 0;
		
		l = alu_bit_set_bit( part, num.upto - 1 );
		
		neg = !!(num.info & ALU_INFO__SIGN) & !!(*(l.S) & l.B);
		
		for (
			b = 1
			; b && n.b < num.upto
			; b <<= 1, n = alu_bit_inc(n)
		)
		{
			*raw |= (!!(*(n.S) & n.B) * b);
		}
		
		for ( ; b ; b <<= 1 )
		{
			*raw |= neg * b;
		}
	}
		
	return n;
}

alu_bit_t alu_get_raw( alu_t *alu, uint_t num, size_t *raw )
{
	alu_bit_t n = {0};
	
	if ( alu )
	{
		num %= ALU_USED( *alu );
		return alu_reg_get_raw( *alu, alu->regv[num], raw );
	}
	
	return n;
}

void alu_reg_set_raw(
	alu_t alu,
	alu_register_t num,
	size_t raw,
	uint_t info
)
{
	void *part;
	alu_register_t *NUM;
	alu_bit_t n = {0};
	size_t b;
	bool neg;
	
	num.node %= ALU_USED( alu );
	
	neg = !!(info & ALU_INFO__SIGN) & !!(raw & SIZE_END_BIT);
	
	NUM = alu.regv + num.node;
	NUM->info = ALU_INFO_VALID | info;
	
	part = ALU_PART( alu, num.node );
	n = alu_bit_set_bit( part, num.from );
	
	for (
		b = 1
		; b && n.b < num.upto
		; b <<= 1, n = alu_bit_inc(n)
	)
	{
		*(n.S) &= ~(n.B);
		*(n.S) |= (!!(raw & b) * n.B);
	}
	
	for
	(
		; n.b < num.upto
		; n = alu_bit_inc(n)
	)
	{
		*(n.S) &= ~(n.B);
		*(n.S) |= neg * n.B;
	}
}

void alu_set_raw( alu_t alu, uint_t num, size_t raw, uint_t info )
{
	num %= ALU_USED( alu );
	alu_reg_set_raw( alu, alu.regv[num], raw, info );
}

alu_bit_t alu_reg_copy(
	alu_t alu,
	alu_register_t num,
	alu_register_t val
)
{
	void *num_part, *val_part;
	alu_bit_t n = {0}, v, e;
	
	if ( num.node < ALU_USED( alu ) )
	{
		num_part = ALU_PART( alu, num.node );
		n = alu_bit_set_bit( num_part, num.from );
		
		if ( val.node < ALU_USED( alu ) )
		{	
			val_part = alu.buff.mem.block + (val.node * alu.buff.perN);
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
		}
	}
	
	return n;
}

void alu_copy( alu_t alu, uint_t num, uint_t val )
{	
	num %= ALU_USED( alu );
	val %= ALU_USED( alu );
	
	alu_reg_copy( alu, alu.regv[num], alu.regv[val] );
}

alu_bit_t alu_end_bit( alu_t alu, alu_register_t num )
{
	alu_bit_t n;
	size_t b;
	
	num.node %= ALU_USED( alu );
	n = alu_bit_set_bit( ALU_PART( alu, num.node ), num.upto - 1 );
	b = n.b * !( *(n.S) & n.B );
	
	while ( b > num.from )
	{
		n = alu_bit_dec( n );
		b = n.b * !( *(n.S) & n.B );
	}
	
	return n;
}

int alu_reg_cmp(
	alu_t alu
	, alu_register_t num
	, alu_register_t val
	, size_t *bit
)
{
	int a, b, c = 0;
	alu_bit_t n = {0}, v = {0};
	size_t ndiff = 0, vdiff = 0;
	bool nNeg, vNeg;
	
	n = alu_end_bit( alu, num );
	v = alu_end_bit( alu, val );
	
	nNeg = !!(num.info & ALU_INFO__SIGN) & (n.b == num.upto - 1);
	vNeg = !!(val.info & ALU_INFO__SIGN) & (v.b == val.upto - 1);
	
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

int alu_cmp( alu_t alu, uint_t num, uint_t val, size_t *bit )
{
	num %= ALU_USED( alu );
	val %= ALU_USED( alu );
	
	return alu_reg_cmp( alu, alu.regv[num], alu.regv[val], bit );
}

void alu_size2reg( alu_t alu, uint_t num, size_t val )
{
	num %= ALU_USED( alu );
	alu_reg_set_raw( alu, alu.regv[num], val, 0 );
}

alu_bit_t alu_reg2size( alu_t *alu, uint_t num, size_t *val )
{
	alu_bit_t n = {0};
	
	if ( alu )
	{
		num %= ALU_USED( *alu );
		return alu_reg_get_raw( *alu, alu->regv[num], val );
	}
	
	return n;
}

void alu_reg_not(
	alu_t alu,
	alu_register_t num
)
{
	void *part;
	alu_bit_t n = {0};
	
	if ( num.node < ALU_USED( alu ) )
	{
		part = ALU_PART( alu, num.node );
		n = alu_bit_set_bit( part, num.from );
		
		for ( ; n.b < num.upto; n = alu_bit_inc( n ) )
		{
			*(n.S) ^= n.B;
		}
	}
}


void alu_not( alu_t alu, uint_t num )
{
	num %= ALU_USED( alu );
	alu_reg_not( alu, alu.regv[num] );
}

int alu_reg_inc( alu_t alu, alu_register_t num )
{
	bool carry = 0;
	alu_bit_t n;
	
	num.node %= ALU_USED( alu );
	n = alu_bit_set_bit( ALU_PART( alu, num.node ), num.from );
	
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

int alu_inc( alu_t alu, uint_t num )
{
	num %= ALU_USED( alu );
	return alu_reg_inc( alu, alu.regv[num] );
}

int alu_reg_dec( alu_t alu, alu_register_t num )
{
	bool carry = 0;
	alu_bit_t n;
	
	num.node %= ALU_USED( alu );
	
	n = alu_bit_set_bit( ALU_PART( alu, num.node ), num.from );
	
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

int alu_dec( alu_t alu, uint_t num )
{
	num %= ALU_USED( alu );
	return alu_reg_dec( alu, alu.regv[num] );
}

int alu_reg_add( alu_t alu, alu_register_t num, alu_register_t val )
{
	bool carry = false, changed = false;
	alu_bit_t n, v = {0};
	size_t pos = 0;
	
	num.node %= ALU_USED( alu );
	val.node %= ALU_USED( alu );
	
	pos = alu_lowest_upto( num, val );
	
	n = alu_bit_set_bit( ALU_PART( alu, num.node ), num.from );
	v = alu_bit_set_bit( ALU_PART( alu, val.node ), val.from );
	n = alu_bit_set_bit( ALU_PART( alu, num.node ), num.from );
	
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

int alu_add( alu_t alu, uint_t num, uint_t val )
{
	num %= ALU_USED( alu );
	val %= ALU_USED( alu );
	return alu_reg_add( alu, alu.regv[num], alu.regv[val] );
}

int alu_reg_sub( alu_t alu, alu_register_t num, alu_register_t val )
{
	bool carry = false, changed = false;
	alu_bit_t n, v = {0};
	size_t pos = 0;
	
	num.node %= ALU_USED( alu );
	val.node %= ALU_USED( alu );
	
	pos = alu_lowest_upto( num, val );
	
	n = alu_bit_set_bit( ALU_PART( alu, num.node ), num.from );
	v = alu_bit_set_bit( ALU_PART( alu, val.node ), val.from );
	n = alu_bit_set_bit( ALU_PART( alu, num.node ), num.from );
	
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

int alu_sub( alu_t alu, uint_t num, uint_t val )
{
	num %= ALU_USED( alu );
	val %= ALU_USED( alu );
	return alu_reg_sub( alu, alu.regv[num], alu.regv[val] );
}

void alu_reg__shl( alu_t alu, alu_register_t num, size_t by )
{
	alu_bit_t n = {0}, v;
	void *part;
	
	if ( by )
	{
		num.node %= ALU_USED( alu );
		
		part = ALU_PART( alu, num.node );
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
}

void alu__shl( alu_t alu, uint_t num, size_t by )
{
	num %= ALU_USED( alu );
	alu_reg__shl( alu, alu.regv[num], by );
}

void alu_reg__shr( alu_t alu, alu_register_t num, size_t by )
{
	alu_bit_t n, v, l, e;
	void *part;
	bool neg;
	
	if ( by )
	{
	
		num.node %= ALU_USED( alu );
		
		part = ALU_PART( alu, num.node );
		
		n = alu_bit_set_bit( part, num.from );
		e = v = alu_bit_set_bit( part, num.upto );
		l = alu_bit_dec( e );
		
		if ( by < (e.b - n.b) )
			v = alu_bit_set_bit( part, n.b + by );
		
		neg = !!(num.info & ALU_INFO__SIGN) & !!(*(l.S) & l.B);
		
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
}

void alu__shr( alu_t alu, uint_t num, size_t by )
{
	num %= ALU_USED( alu );
	alu_reg__shr( alu, alu.regv[num], by );
}

void alu_reg__shift
(
	alu_t alu,
	alu_register_t num,
	alu_register_t val,
	alu_register_t tmp,
	bool left
)
{
	int cmp;
	size_t by = 0, ndiff = num.upto - num.from;
	void (*shift)( alu_t, alu_register_t, size_t )
		= (void (*)( alu_t, alu_register_t, size_t ))
		(((uintptr_t)alu_reg__shl * left) | ((uintptr_t)alu_reg__shr * !left));
		
	num.node %= ALU_USED( alu );
	val.node %= ALU_USED( alu );
	tmp.node %= ALU_USED( alu );
	
	alu_reg_set_raw( alu, tmp, ndiff, 0 );
	cmp = alu_reg_cmp( alu, val, tmp, NULL );
	
	if ( cmp >= 0 )
	{
		shift( alu, num, -1 );
		return;
	}
	
	alu_reg_get_raw( alu, val, &by );
	shift( alu, num, by );
}

void alu__shift( alu_t alu, uint_t num, uint_t val, uint_t tmp, bool left )
{
	num %= ALU_USED( alu );
	val %= ALU_USED( alu );
	tmp %= ALU_USED( alu );
	alu_reg__shift( alu, alu.regv[num], alu.regv[val], alu.regv[tmp], left );
}

int alu_reg_mul
(
	alu_t alu
	, alu_register_t num
	, alu_register_t val
	, alu_register_t tmp
)
{
	int ret = 0;
	bool carry = 0;
	alu_bit_t p, v;
	
	num.node %= ALU_USED( alu );
	val.node %= ALU_USED( alu );
	tmp.node %= ALU_USED( alu );
	
	p = v = alu_bit_set_bit( ALU_PART( alu, val.node ), val.from );
	
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

int alu_mul( alu_t alu, uint_t num, uint_t val, uint_t tmp )
{
	num %= ALU_USED( alu );
	val %= ALU_USED( alu );
	tmp %= ALU_USED( alu );
	
	return alu_reg_mul( alu, alu.regv[num], alu.regv[val], alu.regv[tmp] );
}

void alu_reg_neg( alu_t alu, alu_register_t num )
{
	alu_reg_not( alu, num );
	alu_reg_inc( alu, num );
}

void alu_neg( alu_t alu, uint_t num )
{
	num %= ALU_USED( alu );
	alu_reg_neg( alu, alu.regv[num] );
}

int alu_reg_divide
(
	alu_t alu
	, alu_register_t num
	, alu_register_t val
	, alu_register_t rem
)
{
	int ret = 0;
	alu_register_t tmp;
	alu_bit_t n;
	size_t bits = 0;
	bool nNeg, vNeg;
	void *num_part, *val_part;
	
	num.node %= ALU_USED( alu );
	val.node %= ALU_USED( alu );
	rem.node %= ALU_USED( alu );
	
	num_part = ALU_PART( alu, num.node );
	val_part = ALU_PART( alu, val.node );
	
	tmp = rem;
	
	n = alu_bit_set_bit( num_part, num.upto - 1 );
	nNeg = !!(num.info & ALU_INFO__SIGN) & !!(*(n.S) & n.B);
	
	n = alu_bit_set_bit( val_part, val.upto - 1 );
	vNeg = !!(val.info & ALU_INFO__SIGN) & !!(*(n.S) & n.B);
	
	if ( nNeg )
		alu_reg_neg( alu, num );
	
	if ( vNeg )
		alu_reg_neg( alu, val );
	
	(void)alu_reg_copy( alu, rem, num );
	(void)alu_reg_set_nil( alu, num );

	n = alu_end_bit( alu, rem );
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

int alu_divide( alu_t alu, uint_t num, uint_t val, uint_t rem )
{
	num %= ALU_USED( alu );
	val %= ALU_USED( alu );
	rem %= ALU_USED( alu );
	return alu_reg_divide( alu, alu.regv[num], alu.regv[val], alu.regv[rem] );
}

int alu_reg_div
(
	alu_t alu
	, alu_register_t num
	, alu_register_t val
	, alu_register_t tmp
)
{
	return alu_reg_divide( alu, num, val, tmp );
}

int alu_div( alu_t alu, uint_t num, uint_t val, uint_t tmp )
{
	num %= ALU_USED( alu );
	val %= ALU_USED( alu );
	tmp %= ALU_USED( alu );
	return alu_reg_divide( alu, alu.regv[num], alu.regv[val], alu.regv[tmp] );
}

int alu_reg_rem
(
	alu_t alu
	, alu_register_t num
	, alu_register_t val
	, alu_register_t tmp
)
{
	int ret = alu_reg_divide( alu, num, val, tmp );
	
	(void)alu_reg_copy( alu, num, tmp );
	
	return ret;
}

int alu_rem( alu_t alu, uint_t num, uint_t val, uint_t tmp )
{	
	num %= ALU_USED( alu );
	val %= ALU_USED( alu );
	tmp %= ALU_USED( alu );
	
	return alu_reg_div( alu, alu.regv[num], alu.regv[val], alu.regv[tmp] );
}

void alu_reg__rol( alu_t alu, alu_register_t num, alu_register_t tmp, size_t by )
{
	alu_bit_t n = {0}, v;
	void *num_part, *tmp_part;
	
	if ( !by )
		return;
	
	by %= (num.upto - num.from);
		
	num.node %= ALU_USED( alu );
	tmp.node %= ALU_USED( alu );
	
	tmp.info = num.info;
	tmp.from = num.from;
	tmp.upto = num.upto;
	
	num_part = ALU_PART( alu, num.node );
	tmp_part = ALU_PART( alu, tmp.node );
	
	(void)memcpy( tmp_part, num_part, alu.buff.perN );
	(void)memset( num_part, 0, alu.buff.perN );
	
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
}

void alu__rol( alu_t alu, uint_t num, uint_t tmp, size_t by )
{
	num %= ALU_USED( alu );
	tmp %= ALU_USED( alu );
	alu_reg__rol( alu, alu.regv[num], alu.regv[tmp], by );
}

void alu_reg__ror( alu_t alu, alu_register_t num, alu_register_t tmp, size_t by )
{
	alu_bit_t n, v;
	void *num_part, *tmp_part;
	
	if ( !by )
		return;
		
	by %= (num.upto - num.from);
		
	num.node %= ALU_USED( alu );
	tmp.node %= ALU_USED( alu );
	
	tmp.info = num.info;
	tmp.from = num.from;
	tmp.upto = num.upto;
	
	num_part = ALU_PART( alu, num.node );
	tmp_part = ALU_PART( alu, tmp.node );
	
	(void)memcpy( tmp_part, num_part, alu.buff.perN );
	(void)memset( num_part, 0, alu.buff.perN );
	
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
}

void alu__ror( alu_t alu, uint_t num, uint_t tmp, size_t by )
{
	num %= ALU_USED( alu );
	tmp %= ALU_USED( alu );
	alu_reg__ror( alu, alu.regv[num], alu.regv[tmp], by );
}

void alu_reg__rotate
(
	alu_t alu
	, alu_register_t num
	, alu_register_t val
	, alu_register_t tmp
	, bool left
)
{
	int cmp;
	size_t by = 0, ndiff = num.upto - num.from;
	void (*rotate)( alu_t, alu_register_t, alu_register_t, size_t )
		= (void (*)( alu_t, alu_register_t, alu_register_t, size_t ))
		(((uintptr_t)alu_reg__rol * left) | ((uintptr_t)alu_reg__ror * !left));
	
	alu_reg_set_raw( alu, tmp, ndiff, 0 );
	cmp = alu_reg_cmp( alu, val, tmp, NULL );
	
	if ( cmp >= 0 )
	{
		rotate( alu, num, tmp, -1 );
		return;
	}
	
	alu_reg_get_raw( alu, val, &by );
	rotate( alu, num, tmp, by );
}

size_t alu_lowest_upto( alu_register_t num, alu_register_t val )
{
	size_t ndiff, vdiff;
	
	ndiff = (num.upto - num.from);
	vdiff = (val.upto - val.from);
		
	return num.from + LOWEST(ndiff,vdiff);
}

void alu_reg_and( alu_t alu, alu_register_t num, alu_register_t val )
{
	alu_bit_t n, v;
	size_t pos = 0;
	
	num.node %= ALU_USED( alu );
	val.node %= ALU_USED( alu );
	
	pos = alu_lowest_upto( num, val );
	
	n = alu_bit_set_bit( ALU_PART( alu, num.node ), num.from );
	v = alu_bit_set_bit( ALU_PART( alu, val.node ), val.from );
	
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
}

void alu_and( alu_t alu, uint_t num, uint_t val )
{
	num %= ALU_USED( alu );
	val %= ALU_USED( alu );
	
	alu_reg_and( alu, alu.regv[num], alu.regv[val] );
}

void alu_reg__or( alu_t alu, alu_register_t num, alu_register_t val )
{
	alu_bit_t n, v;
	size_t pos = 0;
	
	num.node %= ALU_USED( alu );
	val.node %= ALU_USED( alu );
	
	pos = alu_lowest_upto( num, val );
	
	n = alu_bit_set_bit( ALU_PART( alu, num.node ), num.from );
	v = alu_bit_set_bit( ALU_PART( alu, val.node ), val.from );
	
	for
	(
		; n.b < pos
		; n = alu_bit_inc( n ), v = alu_bit_inc( v )
	)
	{
		*(n.S) |= (*(v.S) & v.B) ? n.B : SIZE_T_C(0);
	}
}

void alu__or( alu_t alu, uint_t num, uint_t val )
{
	num %= ALU_USED( alu );
	val %= ALU_USED( alu );
	
	alu_reg__or( alu, alu.regv[num], alu.regv[val] );
}

void alu_reg_xor( alu_t alu, alu_register_t num, alu_register_t val )
{
	alu_bit_t n, v;
	size_t pos = 0;
	
	num.node %= ALU_USED( alu );
	val.node %= ALU_USED( alu );
	
	pos = alu_lowest_upto( num, val );
	
	n = alu_bit_set_bit( ALU_PART( alu, num.node ), num.from );
	v = alu_bit_set_bit( ALU_PART( alu, val.node ), val.from );
	
	for
	(
		; n.b < pos
		; n = alu_bit_inc( n ), v = alu_bit_inc( v )
	)
	{
		*(n.S) ^= (*(v.S) & v.B) ? n.B : SIZE_T_C(0);
	}
}

void alu_xor( alu_t alu, uint_t num, uint_t val )
{
	num %= ALU_USED( alu );
	val %= ALU_USED( alu );
	
	alu_reg_xor( alu, alu.regv[num], alu.regv[val] );
}
