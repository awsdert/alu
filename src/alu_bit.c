#include "alu.h"
#include <string.h>

alu_bit_t alu_bit_set_bit
(
	uintmax_t *init,
	size_t bit
)
{
	alu_bit_t alu_bit = {0};
	alu_bit.bit = bit;
	alu_bit.pos = bit % bitsof(uintmax_t);
	alu_bit.seg = bit / bitsof(uintmax_t);
	alu_bit.ptr = init + (alu_bit.seg);
	alu_bit.mask = UNIC_SIZE_C(1) << alu_bit.pos;
	return alu_bit;
}

alu_bit_t alu_bit_set_byte
(
	uintmax_t *init,
	size_t byte
)
{
	alu_bit_t alu_bit = {0};
	alu_bit.bit = byte * CHAR_BIT;
	alu_bit.seg = byte / sizeof(size_t);
	alu_bit.ptr = init + alu_bit.seg;
	alu_bit.mask = UNIC_SIZE_C(1);
	return alu_bit;
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
		alu_bit->mask += !(alu_bit->mask);
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
		alu_bit->mask = SET2IF
		(
			alu_bit->mask
			, alu_bit->mask
			, INTMAX_MIN
		);
	}
}

void alu_print_bit( char *pfx, alu_bit_t alu_bit, bool dereference4value )
{
	char value = dereference4value ? ('0' + !!(*(alu_bit.ptr) & alu_bit.mask)) : '?';
	if ( !pfx ) pfx = "?";
	
	alu_printf(
		"%s.s = %zu, %s.b = %zu, %s.p = %zu",
		pfx, alu_bit.seg, pfx, alu_bit.bit, pfx, alu_bit.pos
	);
	
	alu_printf(
		"Value = %c, %s.S = %p, %s.B = %016jX",
		value, pfx, (void*)(alu_bit.ptr), pfx, alu_bit.mask
	);
}
