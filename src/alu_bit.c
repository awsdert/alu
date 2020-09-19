#include "alu.h"
#include <string.h>

struct alu_bit alu_bit_set_bit
(
	uintmax_t *init,
	size_t bit
)
{
	alu_bit_t pos = {0};
	pos.bit = bit;
	pos.pos = bit % bitsof(uintmax_t);
	pos.seg = bit / bitsof(uintmax_t);
	pos.ptr = init + (pos.seg);
	pos.mask = SIZE_T_C(1) << pos.pos;
	return pos;
}

struct alu_bit alu_bit_set_byte
(
	size_t *init,
	size_t byte
)
{
	alu_bit_t pos = {0};
	pos.bit = byte * CHAR_BIT;
	pos.seg = byte / sizeof(size_t);
	pos.ptr = init + pos.seg;
	pos.mask = SIZE_T_C(1);
	return pos;
}

struct alu_bit alu_bit_inc( struct alu_bit pos )
{
	uintptr_t ptr = (uintptr_t)(pos.ptr);
	
	pos.bit++;
	pos.mask <<= 1;
	pos.pos = SET2IF( pos.mask, pos.pos + 1, 0 );
	pos.seg = SET2IF( pos.mask, pos.seg, pos.seg + 1 );
	pos.ptr = (size_t*)SET2IF( pos.mask, ptr, ptr + sizeof(size_t) );
	pos.mask = SET2IF( pos.mask, pos.mask, SIZE_T_C(1) );
	return pos;
}

struct alu_bit alu_bit_dec( struct alu_bit pos )
{
	uintptr_t ptr = (uintptr_t)(pos.ptr);
	
	pos.bit--;
	pos.mask >>= 1;
	pos.pos = SET2IF( pos.mask, pos.pos - 1, bitsof(size_t) - 1 );
	pos.seg = SET2IF( pos.mask, pos.seg, pos.seg - 1 );
	pos.ptr = (size_t*)SET2IF( pos.mask, ptr, ptr - sizeof(size_t) );
	pos.mask = SET2IF( pos.mask, pos.mask, SIZE_END_BIT );
	return pos;
}

void alu_print_bit( char *pfx, struct alu_bit pos, bool dereference4value )
{
	char value = dereference4value ? ('0' + !!(*(pos.ptr) & pos.mask)) : '?';
	if ( !pfx ) pfx = "?";
	
	alu_printf(
		"%s.s = %zu, %s.b = %zu, %s.p = %zu",
		pfx, pos.seg, pfx, pos.bit, pfx, pos.pos
	);
	
	alu_printf(
		"Value = %c, %s.S = %p, %s.B = %016zX",
		value, pfx, (void*)(pos.ptr), pfx, pos.mask
	);
}
