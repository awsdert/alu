#include "alu.h"
#include <string.h>

struct alu_bit alu_bit_set_bit
(
	size_t *init,
	size_t bit
)
{
	alu_bit_t pos = {0};
	pos.b = bit;
	pos.p = bit % bitsof(size_t);
	pos.s = bit / bitsof(size_t);
	pos.S = init + pos.s;
	pos.B = SIZE_T_C(1) << pos.p;
	return pos;
}

struct alu_bit alu_bit_set_byte
(
	size_t *init,
	size_t byte
)
{
	alu_bit_t pos = {0};
	pos.b = byte * CHAR_BIT;
	pos.s = byte / sizeof(size_t);
	pos.S = init + pos.s;
	pos.B = SIZE_T_C(1);
	return pos;
}

struct alu_bit alu_bit_inc( struct alu_bit pos )
{
	pos.b++;
	pos.p++;
	pos.B <<= 1;
	if ( pos.B )
		return pos;
	pos.p = 0;
	pos.B = SIZE_T_C(1);
	pos.s++;
	pos.S++;
	return pos;
}

struct alu_bit alu_bit_dec( struct alu_bit pos )
{
	pos.b--;
	pos.p--;
	pos.B >>= 1;
	if ( pos.B )
		return pos;
	pos.p = bitsof(size_t) - 1;
	pos.B = SIZE_END_BIT;
	pos.s--;
	pos.S--;
	return pos;
}

void alu_print_bit( char *pfx, struct alu_bit pos, bool dereference4value )
{
	char value = dereference4value ? ('0' + !!(*(pos.S) & pos.B)) : '?';
	if ( !pfx ) pfx = "?";
	
	alu_printf(
		"%s.s = %zu, %s.b = %zu, %s.p = %zu",
		pfx, pos.s, pfx, pos.b, pfx, pos.p
	);
	
	alu_printf(
		"Value = %c, %s.S = %p, %s.B = %016zX",
		value, pfx, (void*)(pos.S), pfx, pos.B
	);
}
