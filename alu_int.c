#include "alu.h"
int_t alu__int_op1
(
	alu_t *alu
	, alu_int_t num
	, func_alu_reg_op1_t op1
)
{
	alu_reg_t _num;
	
	alu_reg_init( alu, _num, num, ALU_INFO__SIGN );
	
	return op1( alu, _num );
}

int_t alu__int_op2
(
	alu_t *alu
	, alu_int_t num
	, alu_int_t val
	, func_alu_reg_op2_t op2
)
{
	alu_reg_t _num, _val;
	
	alu_reg_init( alu, _num, num, ALU_INFO__SIGN );
	alu_reg_init( alu, _val, val, ALU_INFO__SIGN );
	
	return op2( alu, _num, _val );
}

int_t alu__int_op3
(
	alu_t *alu
	, alu_int_t num
	, alu_int_t val
	, alu_int_t reg
	, func_alu_reg_op3_t op3
)
{
	alu_reg_t _num, _val, _reg;
	
	alu_reg_init( alu, _num, num, ALU_INFO__SIGN );
	alu_reg_init( alu, _val, val, ALU_INFO__SIGN );
	alu_reg_init( alu, _reg, reg, ALU_INFO__SIGN );
	
	return op3( alu, _num, _val, _reg );
}

int_t alu__int_shift
(
	alu_t *alu
	, alu_int_t num
	, alu_int_t val
	, bool left
	, func_alu_reg_shift_t shift
)
{
	alu_reg_t _num, _val;
	
	alu_reg_init( alu, _num, num, ALU_INFO__SIGN );
	alu_reg_init( alu, _val, val, ALU_INFO__SIGN );
	
	return shift( alu, _num, _val, left );
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
