#include "alu.h"
#include <string.h>

int_t alu_vec( alu_vec_t *vec, uint_t want, size_t perN, int dir )
{
	int ret = 0, used;
	uchar_t *dst, *src, *block;
	size_t diff = 0;
	
	if ( vec )
	{
		used = vec->qty.used;
		
		if ( perN >= vec->perN )
		{
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
					(void)memmove( dst, src, vec->perN );
					(void)memset( dst - diff, 0, diff );
				}
				goto done;
			}
		}
		else
		{
			diff = (vec->perN) - perN;
			
			/* Crop nodes */
			block = vec->mem.block;
			while ( used )
			{
				--used;
				dst = block + (used * perN);
				src = block + (used * vec->perN);
				(void)memset( src, 0, diff );
				(void)memmove( dst, src + diff, perN );
			}
			
			ret = alu_block( &(vec->mem), want * perN, dir );
		}
		
		if ( ret == 0 )
		{
			done:
			vec->perN = perN;
			vec->qty.upto = want;
			vec->qty.last = !!want * (want - 1);
			vec->qty.used = LOWEST( vec->qty.used, want );
			
			return 0;
		}
		
		alu_error(ret);
		return ret;
	}
	
	return alu_err_null_ptr( "vec" );
}
