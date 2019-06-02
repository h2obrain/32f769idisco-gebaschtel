/*
 * jpeg_tables.h
 *
 *      Author: MCD Application Team
 */

#ifndef LIB_GFX_INCLUDE_GFX_JPEG_TABLES_H_
#define LIB_GFX_INCLUDE_GFX_JPEG_TABLES_H_

#include <stdint.h>

#define JPEG_QUANT_TABLE_SIZE   (64U)
#define JPEG_AC_HUFF_TABLE_SIZE (162U)
#define JPEG_DC_HUFF_TABLE_SIZE (12U)

/*
 JPEG Huffman Table Structure definition :
 This implementation of Huffman table structure is compliant with ISO/IEC 10918-1 standard , Annex C Huffman Table specification
 */
typedef struct {
  /* These two fields directly represent the contents of a JPEG DHT marker */
  uint8_t Bits[16];        /*!< bits[k] = # of symbols with codes of length k bits, this parameter corresponds to BITS list in the Annex C */
  uint8_t HuffVal[JPEG_AC_HUFF_TABLE_SIZE];    /*!< The symbols, in order of incremented code length, this parameter corresponds to HUFFVAL list in the Annex C */
} jpeg_ac_huff_table_t;

typedef struct {
  /* These two fields directly represent the contents of a JPEG DHT marker */
  uint8_t Bits[16];        /*!< bits[k] = # of symbols with codes of length k bits, this parameter corresponds to BITS list in the Annex C */
  uint8_t HuffVal[JPEG_DC_HUFF_TABLE_SIZE];     /*!< The symbols, in order of incremented code length, this parameter corresponds to HUFFVAL list in the Annex C */
} jpeg_dc_huff_table_t;

typedef struct {
  uint8_t CodeLength[JPEG_AC_HUFF_TABLE_SIZE];      /*!< Code length  */
  uint32_t HuffmanCode[JPEG_AC_HUFF_TABLE_SIZE];    /*!< HuffmanCode */
} jpeg_ac_huff_code_table_t;

typedef struct {
  uint8_t CodeLength[JPEG_DC_HUFF_TABLE_SIZE];        /*!< Code length  */
  uint32_t HuffmanCode[JPEG_DC_HUFF_TABLE_SIZE];    /*!< HuffmanCode */
} jpeg_dc_huff_code_table_t;


#endif /* LIB_GFX_INCLUDE_GFX_JPEG_TABLES_H_ */
