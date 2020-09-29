#include "alu.h"
#include <string.h>

int_t alu_vec( alu_vec_t *vec, uint_t want, size_t Nsize, int dir )
{
	int ret = 0;
	uchar_t *dst, *src, *block;
	size_t diff, desire;
	uint_t used;
	
	if ( vec )
	{
		desire = Nsize * want;
		
		if ( desire )
		{
			used = vec->taken;
			
			if ( Nsize >= vec->Nsize )
			{
				ret = alu_block( &(vec->block), desire, dir );
				
				if ( ret == 0 )
				{
					diff = Nsize - (vec->Nsize);
					
					if ( !diff )
						goto done;
						
					/* Align nodes and initialise extra bytes */
					block = vec->block.block;
					while ( used )
					{
						--used;
						dst = block + (used * Nsize) + diff;
						src = block + (used * vec->Nsize);
						(void)memmove( dst, src, vec->Nsize );
						(void)memset( dst - diff, 0, diff );
					}
					goto done;
				}
			}
			else
			{
				diff = (vec->Nsize) - Nsize;
				
				/* Crop nodes */
				block = vec->block.block;
				while ( used )
				{
					--used;
					dst = block + (used * Nsize);
					src = block + (used * vec->Nsize);
					(void)memset( src, 0, diff );
					(void)memmove( dst, src + diff, Nsize );
				}
				
				ret = alu_block( &(vec->block), desire, dir );
			}
			
			if ( ret == 0 )
			{
				done:
				vec->Nsize = Nsize;
				vec->given = want;
				vec->taken = LOWEST( vec->taken, want );
				vec->block.taken = vec->taken * vec->Nsize;
				
				return 0;
			}
			
			alu_error(ret);
			return ret;
		}
		
		alu_block_release( &(vec->block) );
		(void)memset( vec, 0, sizeof(alu_vec_t) );
		return 0;
	}
	
	return alu_err_null_ptr( "vec" );
}
