#include "alu.h"
#include <stdlib.h>
#include <string.h>

int alu_block( struct alu_block *mem, size_t want, int_t dir )
{
	int ret = EINVAL;
	uchar_t *block = NULL;
	
	if ( mem->block )
	{
		if ( dir > 0 && mem->bytes.upto >= want )
			return 0;
		
		if ( dir < 0 && want > mem->bytes.upto )
			return ret;
			
		if ( !want )
		{
			free( mem->block );
			(void)memset( mem, 0, sizeof(alu_block_t) );
			return 0;
		}
		
		errno = 0;
		block = realloc( mem->block, want );
		ret = errno;
	}
	else
	{
		memset( mem, 0, sizeof(alu_block_t) );
		
		if ( !want )
			return 0;
		
		if ( dir < 0 )
			return ret;
		
		errno = 0;
		block = malloc( want );
		ret = errno;
	}
	
	if ( block )
	{
		mem->block = block;
		mem->bytes.upto = want;
		mem->bytes.last = want - 1;
		memset( &(block[mem->bytes.used]), 0, want - mem->bytes.used );
		return 0;
	}
	
	return ret;
}
