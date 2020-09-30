#include "alu.h"
#include <string.h>

int_t alu_vec( alu_vec_t *vec, uint_t want, size_t Nsize, int dir )
{
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
				block = alu_block( &(vec->block), desire, dir );
				
				if ( block )
				{
					diff = Nsize - (vec->Nsize);
					
					if ( !diff )
						goto done;
						
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
				
				block = alu_block( &(vec->block), desire, dir );
			}
			
			if ( block )
			{
				done:
				vec->Nsize = Nsize;
				vec->given = want;
				vec->taken = LOWEST( vec->taken, want );
				vec->block.taken = vec->taken * vec->Nsize;
				
				return 0;
			}
			
			alu_error(vec->block.fault);
			return vec->block.fault;
		}
		
		alu_block_release( &(vec->block) );
		(void)memset( vec, 0, sizeof(alu_vec_t) );
		return 0;
	}
	
	return alu_err_null_ptr( "vec" );
}
