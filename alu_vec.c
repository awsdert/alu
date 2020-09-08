#include "alu.h"
#include <string.h>

int alu_vec( alu_vec_t *vec, uint_t want, size_t perN, int dir )
{
	int ret = 0, used = vec->qty.used;
	void *dst, *src, *block;
	size_t diff = 0;
	
	if ( perN > vec->perN )
		goto expand;
		
	if ( perN < vec->perN )
		goto shrink;
	
	expand:
	ret = alu_block( &(vec->mem), want * perN, dir );
	
	if ( ret == 0 )
	{
		diff = perN - (vec->perN);
		
		if ( !diff )
			goto done;
			
		/* Align nodes and initialise extra bytes */
		block = vec->mem.block;
		while ( used )
		{
			--used;
			dst = block + (used * perN) + diff;
			src = block + (used * vec->perN);
			memmove( dst, src, vec->perN );
			memset( dst - diff, 0, diff );
		}
		goto done;
	}
	goto fail;
	
	shrink:
	diff = (vec->perN) - perN;
	/* Crop nodes */
	block = vec->mem.block;
	while ( used )
	{
		--used;
		dst = block + (used * perN);
		src = block + (used * vec->perN);
		memset( src, 0, diff );
		memmove( dst, src + diff, perN );
	}
	
	ret = alu_block( &(vec->mem), want * perN, dir );
	
	if ( ret != 0 )
		goto fail;
		
	done:
	vec->perN = perN;
	vec->qty.upto = want;
	vec->qty.last = want ? want - 1 : 0;
	
	if ( vec->qty.used > want )
		vec->qty.used = want;
	
	fail:
	return ret;
}
