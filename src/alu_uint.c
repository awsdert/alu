#include "alu.h"

int_t alu_uint_set_raw( alu_t *alu, alu_uint_t num, uintmax_t val )
{
	alu_reg_t NUM;
	
	alu_reg_init( alu, NUM, num, 0 );
	
	return alu_reg_set_raw( alu, NUM, &val, sizeof(uintmax_t), 0 );
}

int_t alu_uint_get_raw( alu_t *alu, alu_uint_t num, uintmax_t *val )
{
	alu_reg_t NUM;
	
	alu_reg_init( alu, NUM, num, 0 );
	
	return alu_reg_get_raw( alu, NUM, val, sizeof(uintmax_t), 0 );
}

int_t alu__uint_op1
(
	alu_t *alu
	, alu_uint_t num
	, func_alu_reg_op1_t op1
)
{
	alu_reg_t _num;
	
	alu_reg_init( alu, _num, num, 0 );
	
	return op1( alu, _num );
}

int_t alu__uint_op2
(
	alu_t *alu
	, alu_uint_t num
	, alu_uint_t val
	, func_alu_reg_op2_t op2
)
{
	alu_reg_t _num, _val;
	
	alu_reg_init( alu, _num, num, 0 );
	alu_reg_init( alu, _val, val, 0 );
	
	return op2( alu, _num, _val );
}

int_t alu__uint_op3
(
	alu_t *alu
	, alu_uint_t num
	, alu_uint_t val
	, alu_uint_t reg
	, func_alu_reg_op3_t op3
)
{
	alu_reg_t _num, _val, _reg;
	
	alu_reg_init( alu, _num, num, 0 );
	alu_reg_init( alu, _val, val, 0 );
	alu_reg_init( alu, _reg, reg, 0 );
	
	return op3( alu, _num, _val, _reg );
}

int_t alu__uint_shift
(
	alu_t *alu
	, alu_uint_t num
	, alu_uint_t val
	, func_alu_reg__shift_t _shift
	, func_alu_reg_shift_t shift
)
{
	alu_reg_t _num, _val;
	
	alu_reg_init( alu, _num, num, 0 );
	alu_reg_init( alu, _val, val, 0 );
	
	return shift( alu, _num, _val, _shift );
}

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
