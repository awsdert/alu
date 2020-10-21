#include "alu.h"
#include <string.h>

alub_t alub( uintmax_t *ptr, size_t bit )
{
	alub_t alub = {0};
	alub.bit = bit;
	alub.pos = bit % bitsof(uintmax_t);
	alub.seg = bit / bitsof(uintmax_t);
	alub.ptr = ptr + (alub.seg);
	alub.mask = UNIC_SIZE_C(1) << alub.pos;
	return alub;
}

bool alub_get( uintmax_t *ptr, size_t bit )
{
	alub_t b = alub( ptr, bit );
	return !!(*(b.ptr) & b.mask);
}

void alub_set( uintmax_t *ptr, size_t bit, bool val )
{
	alub_t b = alub( ptr, bit );
	*(b.ptr) &= ~(b.mask);
	*(b.ptr) |= IFTRUE( val, b.mask );
}

void alub_inc( alub_t *alub )
{
	
	if ( alub )
	{
		size_t i;
		alub->bit++;
		alub->mask <<= 1;
		alub->pos = alub->bit % bitsof(uintmax_t);
		i = alub->bit / bitsof(uintmax_t);
		alub->ptr += (i - alub->seg);
		alub->seg = i;
		alub->mask |= IFTRUE( !(alub->mask), 1 );
	}
}

void alub_dec( alub_t *alub )
{	
	if ( alub )
	{
		size_t i;
		alub->bit--;
		alub->mask >>= 1;
		alub->pos = alub->bit % bitsof(uintmax_t);
		i = alub->bit / bitsof(uintmax_t);
		alub->ptr -= (alub->seg - i);
		alub->seg = i;
		alub->mask |= IFTRUE( !(alub->mask), INTMAX_MIN );
	}
}

void alub_print( char *pfx, alub_t alub, bool dereference4value )
{
	char value = dereference4value ? ('0' + !!(*(alub.ptr) & alub.mask)) : '?';
	if ( !pfx ) pfx = "?";
	
	fprintf( stderr,
		"%s = %c, ptr = %p, mask = %016jX, pos = %zu, seg = %zu, bit = %zu\n"
		, pfx
		, value
		, (void*)(alub.ptr)
		, alub.mask
		, alub.pos
		, alub.seg
		, alub.bit
	);
}
