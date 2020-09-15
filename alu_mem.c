#include "alu.h"
#include <stdlib.h>
#include <string.h>

int_t alu_block( struct alu_block *mem, size_t want, int_t dir )
{
	int ret = EINVAL;
	uchar_t *block = NULL;
	size_t size;
	
	if ( mem )
	{	
		if ( mem->block )
		{
			want = SET2IF( dir > 0, HIGHEST( mem->bytes.upto, want ), want );
			want = SET2IF( dir < 0, LOWEST( mem->bytes.upto, want ), want );
			
			if ( want )
			{
				want += 4 * sizeof(size_t);
				
				size = want % sizeof(size_t);
				want /= SET2IF( size, sizeof(size_t), 1 );
				want *= SET2IF( size, sizeof(size_t), 1 );
				want += SET1IF( size, sizeof(size_t) );
				
				if ( want != mem->bytes.upto )
				{				
					errno = 0;
					block = realloc( mem->block, want );
					ret = errno;
				}
				else
				{
					return 0;
				}
			}
			else
			{
				free( mem->block );
				(void)memset( mem, 0, sizeof(alu_block_t) );
				return 0;
			}
		}
		else
		{
			want = SET1IF( dir >= 0, want );
			
			memset( mem, 0, sizeof(alu_block_t) );
			
			if ( want )
			{
				want += 4 * sizeof(size_t);
				
				size = want % sizeof(size_t);
				want /= SET2IF( size, sizeof(size_t), 1 );
				want *= SET2IF( size, sizeof(size_t), 1 );
				want += SET1IF( size, sizeof(size_t) );
				
				errno = 0;
				block = malloc( want );
				ret = errno;
			}
			else
			{
				return 0;
			}
		}
		
		if ( block )
		{
			want -= 4 * sizeof(size_t);
			
			mem->block = block;
			mem->bytes.upto = want;
			mem->bytes.last = want - 1;
			
			for ( size = want; size > mem->bytes.used; block[--size] = 0 );
			
			mem->bytes.used = want;
			return 0;
		}
		
		return ret;
	}
	return alu_err_null_ptr( "mem" );
}
