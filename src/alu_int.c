#include "alu.h"

int_t alu_int_set_raw( alu_t *alu, alu_int_t num, intmax_t val )
{
	alur_t NUM;
	
	alur_init___signed( alu, NUM, num );
	
	return alur_set_raw( alu, NUM, &val, sizeof(intmax_t), ALU_INFO__SIGN );
}

int_t alu_int_get_raw( alu_t *alu, alu_int_t num, intmax_t *val )
{
	alur_t NUM;
	
	alur_init___signed( alu, NUM, num );
	
	return alur_get_raw( alu, NUM, val, sizeof(intmax_t), ALU_INFO__SIGN );
}

int_t alu_int___shift
(
	alu_t *alu
	, alu_int_t num
	, alu_int_t tmp
	, size_t bits
	, func_alur__shift_t _shift
)
{
	alur_t NUM;
	
	alur_init___signed( alu, NUM, num );
	
	return _shift( alu, NUM, tmp, bits );
}

int_t alu_int__shift
(
	alu_t *alu
	, alu_int_t num
	, alu_int_t val
	, alu_int_t tmp
	, func_alur__shift_t _shift
	, func_alur_shift_t shift
)
{
	alur_t NUM, VAL;
	
	alur_init___signed( alu, NUM, num );
	alur_init___signed( alu, VAL, val );
	
	return shift( alu, NUM, VAL, tmp, _shift );
}

int_t alu_str2int( alu_t *alu, alu_src_t src, alu_int_t dst, alu_base_t base )
{
	alur_t tmp = {0};
	
	alur_init___signed( alu, tmp, dst );
	
	return alu_str2reg( alu, src, tmp, base );
}

int_t alu_int2str( alu_t *alu, alu_dst_t dst, alu_int_t src, alu_base_t base )
{
	int ret = 0;
	alur_t tmp = {0};
	
	alur_init___signed( alu, tmp, src );
	
	ret = alur2str( alu, dst, tmp, base );
	
	if ( ret != 0 )
		alu_error( ret );
	
	return ret;
}
