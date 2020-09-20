#include "alu.h"
#include <string.h>

struct alu_bit alu_bit_set_bit
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

struct alu_bit alu_bit_set_byte
(
	size_t *init,
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
	uintptr_t ptr;
	
	if ( alu_bit )
	{
		ptr = (uintptr_t)(alu_bit->ptr);
	
		alu_bit->bit++;
		alu_bit->mask <<= 1;
		alu_bit->pos = SET2IF
		(
			alu_bit->mask
			, alu_bit->pos + 1
			, 0
		);
		alu_bit->seg = SET2IF
		(
			alu_bit->mask
			, alu_bit->seg
			, alu_bit->seg + 1
		);
		alu_bit->ptr = (size_t*)SET2IF
		(
			alu_bit->mask
			, ptr
			, ptr + sizeof(size_t)
		);
		alu_bit->mask = SET2IF
		(
			alu_bit->mask
			, alu_bit->mask
			, UNIC_SIZE_C(1)
		);
	}
}

void alu_bit_dec( alu_bit_t *alu_bit )
{
	uintptr_t ptr;
	
	if ( alu_bit )
	{
		ptr = (uintptr_t)(alu_bit->ptr);
	
		alu_bit->bit--;
		alu_bit->mask >>= 1;
		alu_bit->pos = SET2IF
		(
			alu_bit->mask
			, alu_bit->pos - 1
			, bitsof(size_t) - 1
		);
		alu_bit->seg = SET2IF
		(
			alu_bit->mask
			, alu_bit->seg
			, alu_bit->seg - 1
		);
		alu_bit->ptr = (size_t*)SET2IF
		(
			alu_bit->mask
			, ptr
			, ptr - sizeof(size_t)
		);
		
		alu_bit->mask = SET2IF
		(
			alu_bit->mask
			, alu_bit->mask
			, UNIC_SIZE_END_BIT
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
		"Value = %c, %s.S = %p, %s.B = %016zX",
		value, pfx, (void*)(alu_bit.ptr), pfx, alu_bit.mask
	);
}
