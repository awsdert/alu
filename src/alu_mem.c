#include "alu.h"
#include <stdlib.h>
#include <string.h>

void* alu_block( alu_block_t *mem, size_t want, int_t dir )
{
	int ret = EINVAL;
	uchar_t *block = NULL;
	size_t size;
	
	if ( mem )
	{
		mem->fault = 0;
		
		if ( mem->block )
		{
			want = SET2IF( dir > 0, HIGHEST( mem->given, want ), want );
			want = SET2IF( dir < 0, LOWEST( mem->given, want ), want );
			
			if ( want )
			{	
				size = want % sizeof(size_t);
				want /= SET2IF( size, sizeof(size_t), 1 );
				want *= SET2IF( size, sizeof(size_t), 1 );
				want += SET1IF( size, sizeof(size_t) );
				
				if ( want != mem->given )
				{	
					size = want + (4 * sizeof(size_t));
					alu_printf
					(
						"want = %zu, size = %zu, given = %zu"
						, want
						, size
						, mem->given
					);
					
					errno = 0;
					block = realloc( mem->block, size );
					ret = errno;
				}
				else
				{
					return mem->block;
				}
			}
			else
			{
				free( mem->block );
				(void)memset( mem, 0, sizeof(alu_block_t) );
				return NULL;
			}
		}
		else
		{
			want = SET1IF( dir >= 0, want );
			
			memset( mem, 0, sizeof(alu_block_t) );
			
			if ( want )
			{				
				size = want % sizeof(size_t);
				want /= SET2IF( size, sizeof(size_t), 1 );
				want *= SET2IF( size, sizeof(size_t), 1 );
				want += SET1IF( size, sizeof(size_t) );
				
				size = want + (4 * sizeof(size_t));
				alu_printf
				(
					"want = %zu, size = %zu, given = %zu"
					, want
					, size
					, mem->given
				);
				
				errno = 0;
				block = malloc( size );
				ret = errno;
			}
			else
			{
				return mem->block;
			}
		}
		
		if ( block )
		{			
			mem->block = block;
			mem->given = want;
			
			for ( size = want; size > mem->taken; block[--size] = 0 );
			
			return block;
		}
		
		mem->fault = ret;
		return NULL;
	}
	
	(void)alu_err_null_ptr( "mem" );
	return NULL;
}
