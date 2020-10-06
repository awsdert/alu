#include "alu.h"
#include <stdlib.h>
#include <string.h>
#ifdef UNIC_SYS_WIN32
#else
#include <unistd.h>
#include <sys/mman.h>
#endif

void* alu_block( alu_block_t *mem, size_t want, int_t dir )
{
	int ret = EINVAL;
	uchar_t *block = NULL;
	size_t size, pagesize;
#ifdef UNIC_SYS_WIN32
	SYSTEM_INFO si;
#endif
	
	if ( mem )
	{
		mem->fault = 0;
		
		if ( want )
		{
#ifdef UNIC_SYS_WIN32
			GetSystemInfo(&si);
			pagesize = si.dwPageSize
#else
			pagesize = sysconf(_SC_PAGESIZE);
#endif
			
			want = SET2IF( dir > 0, HIGHEST( mem->given, want ), want );
			want = SET2IF( dir < 0, LOWEST( mem->given, want ), want );
			
			size = want % pagesize;
			want /= pagesize;
			want += !!size;
			want *= pagesize;
			
			if ( want != mem->given )
			{					
				errno = 0;
				block = mmap
				(
					NULL
					, want
					, PROT_READ | PROT_WRITE
					, MAP_PRIVATE | MAP_ANON
					, -1
					, 0
				);
				
				ret = errno;
				
				if ( block )
				{
					if ( mem->block )
					{
						size = LOWEST( want, mem->given );
						memcpy( block, mem->block, size );
						munmap( mem->block, mem->given );
					}
					
					mem->given = want;
					mem->taken = LOWEST( want, mem->taken );
					return (mem->block = block);
				}
				
				mem->fault = ret;
				return NULL;
			}
			
			return mem->block;
		}
		
		if ( mem->block )
		{
			munmap( mem->block, mem->given );
		}
		
		(void)memset( mem, 0, sizeof(alu_block_t) );
		return NULL;
	}
	
	(void)alu_err_null_ptr( "mem" );
	return NULL;
}
