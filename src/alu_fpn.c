#include "alu.h"

int_t alu_fpn_set_raw( alu_t *alu, alu_fpn_t num, long double val )
{
	alur_t NUM;
	
	alur_init_floating( alu, NUM, num );
	
	return alur_set_raw( alu, NUM, &val, bitsof(long double), ALU_INFO_FLOAT );
}

int_t alu_fpn_get_raw( alu_t *alu, alu_fpn_t num, long double *val )
{
	alur_t NUM;
	
	alur_init_floating( alu, NUM, num );
	
	return alur_get_raw( alu, NUM, &val, bitsof(long double), ALU_INFO_FLOAT );
}
