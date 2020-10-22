#include "alu/alum.h"
#include <stdlib.h>
#include <string.h>

#ifdef UNIC_SYS_WIN32
#else
#include <unistd.h>
#include <sys/mman.h>
#endif

void* alum( alum_t *mem, size_t want, int_t dir )
{	
	if ( mem )
	{
		mem->fault = 0;
		
		want = EITHER( dir > 0, HIGHEST( mem->given, want ), want );
		want = EITHER( dir < 0, LOWEST( mem->given, want ), want );
		
		if ( want )
		{
			size_t size, pagesize;
#ifdef UNIC_SYS_WIN32
			SYSTEM_INFO si;
			GetSystemInfo(&si);
			pagesize = si.dwPageSize
#else
			pagesize = sysconf(_SC_PAGESIZE);
#endif
			
			size = want % pagesize;
			want /= pagesize;
			want += !!size;
			want *= pagesize;
			
			if ( want != mem->given )
			{
				int_t ret;
				void *block;
				errno = 0;
				block = calloc( want, 1 );
				
				ret = errno;
				
				if ( block )
				{
					if ( mem->block )
					{
						size = LOWEST( want, mem->given );
						memcpy( block, mem->block, size );
						free( mem->block );
					}
					
					mem->given = want;
					mem->block = block;
					mem->taken = LOWEST( want, mem->taken );
					return block;
				}
				
				mem->fault = ret;
				return NULL;
			}
			
			return mem->block;
		}
		
		if ( mem->block )
		{
			free( mem->block );
		}
		
		(void)memset( mem, 0, sizeof(alum_t) );
		return NULL;
	}
	
	(void)alu_err_null_ptr( "mem" );
	return NULL;
}
