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

int_t alu_int___shift
(
	alu_t *alu
	, alu_int_t num
	, alu_int_t tmp
	, size_t bits
	, func_alu_reg__shift_t _shift
)
{
	alu_reg_t NUM, TMP;
	
	alu_reg_init( alu, NUM, num, ALU_INFO__SIGN );
	alu_reg_init( alu, TMP, tmp, 0 );
	
	return _shift( alu, NUM, TMP, bits );
}

int_t alu_int__shift
(
	alu_t *alu
	, alu_int_t num
	, alu_int_t val
	, alu_int_t tmp
	, func_alu_reg__shift_t _shift
	, func_alu_reg_shift_t shift
)
{
	alu_reg_t NUM, VAL, TMP;
	
	alu_reg_init( alu, NUM, num, ALU_INFO__SIGN );
	alu_reg_init( alu, VAL, val, ALU_INFO__SIGN );
	alu_reg_init( alu, TMP, tmp, 0 );
	
	return shift( alu, NUM, VAL, TMP, _shift );
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
		alu_error( ret );
	
	return ret;
}
