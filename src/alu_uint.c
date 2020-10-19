#include "alu.h"

int_t alu_str2uint( alu_t *alu, alu_src_t src, alu_uint_t dst, alu_base_t base )
{
	alur_t _dst;
	
	alur_init_unsigned( alu, _dst, dst );
	
	return alu_str2reg( alu, src, _dst, base );
}

int_t alu_uint2str
(
	alu_t *alu
	, alu_dst_t dst
	, alu_uint_t src
	, alu_base_t base
)
{	
	alur_t _src;
	
	alur_init_unsigned( alu, _src, src );
	
	return alur2str( alu, dst, _src, base );
}
