#include "alu.h"

int_t alu_int_set_raw( alu_t *alu, alu_int_t num, intmax_t val )
{
	alu_reg_t NUM;
	
	alu_reg_init( alu, NUM, num, ALU_INFO__SIGN );
	
	return alu_reg_set_raw( alu, NUM, &val, sizeof(intmax_t), ALU_INFO__SIGN );
}

int_t alu_int_get_raw( alu_t *alu, alu_int_t num, intmax_t *val )
{
	alu_reg_t NUM;
	
	alu_reg_init( alu, NUM, num, ALU_INFO__SIGN );
	
	return alu_reg_get_raw( alu, NUM, val, sizeof(intmax_t) );
}

int_t alu__int_shift
(
	alu_t *alu
	, alu_int_t num
	, alu_int_t val
	, func_alu_reg__shift_t _shift
	, func_alu_reg_shift_t shift
)
{
	alu_reg_t _num, _val;
	
	alu_reg_init( alu, _num, num, ALU_INFO__SIGN );
	alu_reg_init( alu, _val, val, ALU_INFO__SIGN );
	
	return shift( alu, _num, _val, _shift );
}

int_t alu_str2int( alu_t *alu, alu_src_t src, alu_int_t dst, alu_base_t base )
{
	alu_reg_t tmp = {0};
	
	alu_reg_init( alu, tmp, dst, ALU_INFO__SIGN );
	
	return alu_str2reg( alu, src, tmp, base );
}

int_t alu_int2str( alu_t *alu, alu_dst_t dst, alu_int_t src, alu_base_t base )
{
	int ret = 0;
	alu_reg_t tmp = {0};
	
	alu_reg_init( alu, tmp, src, ALU_INFO__SIGN );
	
	ret = alu_reg2str( alu, dst, tmp, base );
	
	if ( ret != 0 )
		alu_error(ret);
	
	return ret;
}
