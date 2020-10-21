#ifndef INC_ALUB_H
# define INC_ALUB_H

# include "_.h"

/** @brief Holds various values for bit position in a number
 * @param seg Index number of size_t sized data segments
 * @param bit Overall bit from 0 to alur.upto
 * @param pos Position of bit from 0 to bitsof(size_t) - 1
 * @param ptr Segment that is bit is in
 * @param mask Mask needed to reference specific bit
**/
typedef struct alub {
	uintmax_t seg, bit, pos, *ptr, mask;
} alub_t;

void alub_print( char *pfx, alub_t pos, bool getbit );

/** @brief sets bit position by bit count
 * @param ptr Base pointer that bit 0 starts from
 * @param pos Bit index to calculate position from
 * **/
alub_t alub( uintmax_t *ptr, size_t bit );
bool alub_get( uintmax_t *ptr, size_t bit );
void alub_set( uintmax_t *ptr, size_t bit, bool val );
/** @brief sets bit position by byte count
 * @param ptr Base pointer that byte 0 starts from
 * @param pos Byte index to calculate position from
 * **/
#define alub_by_byte( ptr, byte ) alub( ptr, byte * CHAR_BIT )

/** @brief increments bit position
 * @param pos object holding various values needed for bit position
 * **/
void alub_inc( alub_t *pos );
/** @brief decrements bit position
 * @param pos object holding various values needed for bit position
 * **/
void alub_dec( alub_t *pos );

#endif
