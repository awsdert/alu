#include "alu.h"

int_t alu_str2uint( alu_t *alu, alu_src_t src, alu_uint_t dst, alu_base_t base )
{
	alu_reg_t _dst;
	
	alu_reg_init( alu, _dst, dst, 0 );
	
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
	alu_reg_t _src;
	
	alu_reg_init( alu, _src, src, 0 );
	
	return alu_reg2str( alu, dst, _src, base );
}
