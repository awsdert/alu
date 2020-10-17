#include "alu.h"
#include <string.h>

alu_bit_t alu_bit( uintmax_t *ptr, size_t bit )
{
	alu_bit_t alu_bit = {0};
	alu_bit.bit = bit;
	alu_bit.pos = bit % bitsof(uintmax_t);
	alu_bit.seg = bit / bitsof(uintmax_t);
	alu_bit.ptr = ptr + (alu_bit.seg);
	alu_bit.mask = UNIC_SIZE_C(1) << alu_bit.pos;
	return alu_bit;
}

bool alu_get_bit( uintmax_t *ptr, size_t bit )
{
	alu_bit_t b = alu_bit( ptr, bit );
	return !!(*(b.ptr) & b.mask);
}

void alu_set_bit( uintmax_t *ptr, size_t bit, bool val )
{
	alu_bit_t b = alu_bit( ptr, bit );
	*(b.ptr) &= ~(b.mask);
	*(b.ptr) |= IFTRUE( val, b.mask );
}

void alu_bit_inc( alu_bit_t *alu_bit )
{
	size_t i;
	
	if ( alu_bit )
	{
		alu_bit->bit++;
		alu_bit->mask <<= 1;
		alu_bit->pos = alu_bit->bit % bitsof(uintmax_t);
		i = alu_bit->bit / bitsof(uintmax_t);
		alu_bit->ptr += (i - alu_bit->seg);
		alu_bit->seg = i;
		alu_bit->mask |= !(alu_bit->mask);
	}
}

void alu_bit_dec( alu_bit_t *alu_bit )
{
	size_t i;
	
	if ( alu_bit )
	{
		alu_bit->bit--;
		alu_bit->mask >>= 1;
		alu_bit->pos = alu_bit->bit % bitsof(uintmax_t);
		i = alu_bit->bit / bitsof(uintmax_t);
		alu_bit->ptr -= (alu_bit->seg - i);
		alu_bit->seg = i;
		alu_bit->mask |= IFTRUE( !(alu_bit->mask), INTMAX_MIN );
	}
}

void alu_print_bit( char *pfx, alu_bit_t alu_bit, bool dereference4value )
{
	char value = dereference4value ? ('0' + !!(*(alu_bit.ptr) & alu_bit.mask)) : '?';
	if ( !pfx ) pfx = "?";
	
	fprintf( stderr,
		"%s = %c, ptr = %p, mask = %016jX, pos = %zu, seg = %zu, bit = %zu\n"
		, pfx
		, value
		, (void*)(alu_bit.ptr)
		, alu_bit.mask
		, alu_bit.pos
		, alu_bit.seg
		, alu_bit.bit
	);
}
