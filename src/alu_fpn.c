#include "alu.h"

int_t alu_fpn_set_raw( alu_t *alu, alu_fpn_t num, long double val )
{
	alu_reg_t NUM;
	
	alu_reg_init_floating( alu, NUM, num );
	
	return alu_reg_set_raw( alu, NUM, &val, sizeof(long double), ALU_INFO_FLOAT );
}

int_t alu_fpn_get_raw( alu_t *alu, alu_fpn_t num, long double *val )
{
	alu_reg_t NUM;
	
	alu_reg_init_floating( alu, NUM, num );
	
	return alu_reg_set_raw( alu, NUM, &val, sizeof(long double), ALU_INFO_FLOAT );
}
