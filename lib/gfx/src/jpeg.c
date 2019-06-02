/*
 * jpeg.c
 *
 *  Created on: May 30, 2019
 *      Author: Oliver Meier
 */

#include <stdint.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/jpeg.h>
#include <libopencm3/stm32/dma.h>
#include <drivers/dma2d_helper_functions.h>

#include <gfx/jpeg.h>
#include <gfx/jpeg_tables.h>

typedef enum {
	DMAEF_TCIF  = DMA_TCIF,  // 1<<5
	DMAEF_HTIF  = DMA_HTIF,  // 1<<4
	DMAEF_TEIF  = DMA_TEIF,  // 1<<3
	DMAEF_DMEIF = DMA_DMEIF, // 1<<2
	DMAEF_FEIF  = DMA_FEIF,  // 1<<0

	DMAEF_TRX    = 1<<1,
	DMAEF_NOFLAG = 1<<1, // TX or RX is also set
	DMAEF_RX     = 1<<6,
	DMAEF_TX     = 1<<7
} dma_isr_flags_t;
static inline void dma_disable_stream_wait(uint32_t dma, uint8_t stream)
{
	DMA_SCR(dma, stream) &= ~DMA_SxCR_EN;
	while (DMA_SCR(dma, stream) & DMA_SxCR_EN);
}
#define dma_get_interrupt_flags(dma, stream, interrupt) ( \
		(stream<4) ? \
				((DMA_LISR(dma) >> DMA_ISR_OFFSET(stream)) & (interrupt)) \
			: \
			  	((DMA_HISR(dma) >> DMA_ISR_OFFSET(stream)) & (interrupt)) \
		)


static inline void jpeg_reset_fifo_dma(void);

static void jpeg_init(void);
void jpeg_init() {
//	/*Note : these intermediate variables are used to avoid MISRA warning
//	regarding rule 11.5 */
//	uint32_t acLum_huffmanTableAddr = (uint32_t)(&JPEG_ACLUM_HuffTable);
//	uint32_t dcLum_huffmanTableAddr = (uint32_t)(&JPEG_DCLUM_HuffTable);
//	uint32_t acChrom_huffmanTableAddr = (uint32_t)(&JPEG_ACCHROM_HuffTable);
//	uint32_t dcChrom_huffmanTableAddr = (uint32_t)(&JPEG_DCCHROM_HuffTable);

	/* Reset  */
	jpeg_reset_fifo_dma();
	rcc_periph_reset_pulse(RST_JPEG);

	/* Enable JPEG codec core */
	JPEG_CR |= JPEG_CR_JCEN;

//	hjpeg->QuantTable0 = (uint8_t *)JPEG_LUM_QuantTable;
//	hjpeg->QuantTable1 = (uint8_t *)JPEG_CHROM_QuantTable;
//	hjpeg->QuantTable2 = NULL;
//	hjpeg->QuantTable3 = NULL;
//
//	/* init the default Huffman tables*/
//	if(JPEG_Set_HuffEnc_Mem(hjpeg, (JPEG_ACHuffTableTypeDef *)acLum_huffmanTableAddr, (JPEG_DCHuffTableTypeDef *)dcLum_huffmanTableAddr, (JPEG_ACHuffTableTypeDef *)acChrom_huffmanTableAddr, (JPEG_DCHuffTableTypeDef *)dcChrom_huffmanTableAddr) != HAL_OK)
//	{
//	hjpeg->ErrorCode = HAL_JPEG_ERROR_HUFF_TABLE;
//
//	return HAL_ERROR;
//	}
//
	/* Enable header parsing */
	JPEG_CONFR1 |= JPEG_CONFR1_DE | JPEG_CONFR1_HDR;
//
//	/* Reset JpegInCount and JpegOutCount */
//	hjpeg->JpegInCount = 0;
//	hjpeg->JpegOutCount = 0;
//
//	/* Change the JPEG state */
//	hjpeg->State = HAL_JPEG_STATE_READY;
//
//	/* Reset the JPEG ErrorCode */
//	hjpeg->ErrorCode = HAL_JPEG_ERROR_NONE;
//
//	/*Clear the context filelds*/
//	hjpeg->Context = 0;
}


#define JPEG_IN_DMA_STREAM   DMA_STREAM0
#define JPEG_IN_DMA_CHANNEL  ((uint8_t)DMA_SxCR_CHSEL_9)
#define JPEG_IN_DMA_ISR      dma2_stream0_isr
#define JPEG_IN_DMA_IRQ      NVIC_DMA2_STREAM0_IRQ
//#define JPEG_IN_DMA_STREAM   DMA_STREAM3
//#define JPEG_IN_DMA_ISR      dma2_stream3_isr
//#define JPEG_IN_DMA_IRQ      NVIC_DMA2_STREAM0_IRQ

#define JPEG_OUT_DMA_STREAM  DMA_STREAM1
#define JPEG_OUT_DMA_CHANNEL ((uint8_t)DMA_SxCR_CHSEL_9)
#define JPEG_OUT_DMA_ISR     dma2_stream1_isr
#define JPEG_OUT_DMA_IRQ     NVIC_DMA2_STREAM1_IRQ
//#define JPEG_OUT_DMA_STREAM  DMA_STREAM4
//#define JPEG_OUT_DMA_ISR     dma2_stream1_isr
//#define JPEG_OUT_DMA_IRQ     NVIC_DMA2_STREAM4_IRQ

static inline
void jpeg_reset_fifo_dma(void) {
	/* Disable jpeg conversion */
	JPEG_CONFR0 &= ~JPEG_CONFR0_START;
	/* Reset DMA */
	dma_disable_stream_wait(DMA2, JPEG_IN_DMA_STREAM);
	dma_disable_stream_wait(DMA2, JPEG_OUT_DMA_STREAM);
	dma_stream_reset(DMA2, JPEG_IN_DMA_STREAM);
	dma_stream_reset(DMA2, JPEG_OUT_DMA_STREAM);
	/* Disable dma */
	JPEG_CR &= ~(JPEG_CR_IDMAEN | JPEG_CR_ODMAEN);
	/* Flush FIFOs */
	JPEG_CR |= JPEG_CR_IFF | JPEG_CR_OFF;
}
//static inline
//void jpeg_reset_output_fifo_dma_size_unter_anderem(void) {
////	bool ibkp = cb_is_interrupts_enabled();
////	cb_disable_interrupts();
//
//	/* disabled stream (should already be disabled) */
////	dma_disable_stream_wait(DMA2, JPEG_IN_DMA_STREAM);
////	dma_disable_stream_wait(DMA2, JPEG_OUT_DMA_STREAM);
//
//	/* not needed here */
////	dma_set_memory_address(DMA2, ...);
//
//	/* reset transmission size */
////	dma_set_number_of_data(DMA2, JPEG_IN_DMA_STREAM,  16); // oder input_size
////	dma_set_number_of_data(DMA2, JPEG_OUT_DMA_STREAM, 16); // oder ((w+7)/8)*(((h+7)/8)*64
//
////	if (ibkp) cb_enable_interrupts();
//}
static inline
void jpeg_start_fifo_dma(void) {
	/* enabled stream */
	dma_enable_stream(DMA2, JPEG_IN_DMA_STREAM);
	dma_enable_stream(DMA2, JPEG_OUT_DMA_STREAM);
}
static inline
void jpeg_config_input_fifo_dma(void *in, uint32_t in_size) {
	dma_channel_select(DMA2, JPEG_IN_DMA_STREAM, JPEG_IN_DMA_CHANNEL);
	dma_set_priority(DMA2, JPEG_IN_DMA_STREAM, DMA_SxCR_PL_LOW);

	dma_set_memory_size(DMA2, JPEG_IN_DMA_STREAM, DMA_SxCR_MSIZE_32BIT);
	dma_set_peripheral_size(DMA2, JPEG_IN_DMA_STREAM, DMA_SxCR_PSIZE_32BIT);

	dma_disable_peripheral_increment_mode(DMA2, JPEG_IN_DMA_STREAM);
	dma_enable_memory_increment_mode(DMA2, JPEG_IN_DMA_STREAM);

	dma_set_transfer_mode(DMA2, JPEG_IN_DMA_STREAM, DMA_SxCR_DIR_MEM_TO_PERIPHERAL);
//	dma_set_dma_flow_control(DMA2, JPEG_IN_DMA_STREAM);
	dma_set_peripheral_flow_control(DMA2, JPEG_IN_DMA_STREAM);

	dma_enable_direct_mode(DMA2, JPEG_IN_DMA_STREAM);
	//dma_set_fifo_threshold(DMA2, JPEG_IN_DMA_STREAM, DMA_SxFCR_FTH_2_4_FULL);
	dma_set_memory_burst(DMA2, JPEG_IN_DMA_STREAM, DMA_SxCR_MBURST_INCR4);
	dma_set_peripheral_burst(DMA2, JPEG_IN_DMA_STREAM, DMA_SxCR_PBURST_INCR4);

	dma_disable_double_buffer_mode(DMA2, JPEG_IN_DMA_STREAM);

	dma_set_memory_address(DMA2, JPEG_IN_DMA_STREAM, (uint32_t) in);
	dma_set_peripheral_address(DMA2, JPEG_IN_DMA_STREAM, (uint32_t) &JPEG_DIR);

	dma_set_number_of_data(DMA2, JPEG_IN_DMA_STREAM, (in_size+3)/4);
}

static inline
void jpeg_config_output_fifo_dma(void *out, uint32_t out_size) {
	dma_channel_select(DMA2, JPEG_OUT_DMA_STREAM, JPEG_OUT_DMA_CHANNEL);
	dma_set_priority(DMA2, JPEG_OUT_DMA_STREAM, DMA_SxCR_PL_LOW);

	dma_set_memory_size(DMA2, JPEG_OUT_DMA_STREAM, DMA_SxCR_MSIZE_32BIT);
	dma_set_peripheral_size(DMA2, JPEG_OUT_DMA_STREAM, DMA_SxCR_PSIZE_32BIT);

	dma_disable_peripheral_increment_mode(DMA2, JPEG_OUT_DMA_STREAM);
	dma_enable_memory_increment_mode(DMA2, JPEG_OUT_DMA_STREAM);

	dma_set_transfer_mode(DMA2, JPEG_OUT_DMA_STREAM, DMA_SxCR_DIR_PERIPHERAL_TO_MEM);
//	dma_set_dma_flow_control(DMA2, JPEG_OUT_DMA_STREAM);
	dma_set_peripheral_flow_control(DMA2, JPEG_OUT_DMA_STREAM);

	dma_enable_direct_mode(DMA2, JPEG_OUT_DMA_STREAM);
	//dma_set_fifo_threshold(DMA2, JPEG_OUT_DMA_STREAM, DMA_SxFCR_FTH_2_4_FULL);
	dma_set_memory_burst(DMA2, JPEG_OUT_DMA_STREAM, DMA_SxCR_MBURST_INCR4);
	dma_set_peripheral_burst(DMA2, JPEG_OUT_DMA_STREAM, DMA_SxCR_PBURST_INCR4);

	dma_disable_double_buffer_mode(DMA2, JPEG_OUT_DMA_STREAM);

	dma_set_peripheral_address(DMA2, JPEG_OUT_DMA_STREAM, (uint32_t) &JPEG_DIR);
	dma_set_memory_address(DMA2, JPEG_OUT_DMA_STREAM, (uint32_t) out);

	dma_set_number_of_data(DMA2, JPEG_OUT_DMA_STREAM, (out_size+3)/4);
}

static inline
void jpeg_enable_fifo_dma(void) {
	dma_clear_interrupt_flags(DMA2, JPEG_IN_DMA_STREAM, DMA_ISR_FLAGS);
	dma_clear_interrupt_flags(DMA2, JPEG_OUT_DMA_CHANNEL, DMA_ISR_FLAGS);

	dma_enable_transfer_complete_interrupt(DMA2, JPEG_IN_DMA_STREAM);
	dma_enable_transfer_error_interrupt(DMA2, JPEG_IN_DMA_STREAM);
	dma_enable_fifo_error_interrupt(DMA2, JPEG_IN_DMA_STREAM);

	dma_enable_transfer_complete_interrupt(DMA2, JPEG_OUT_DMA_CHANNEL);
	dma_enable_transfer_error_interrupt(DMA2, JPEG_OUT_DMA_CHANNEL);
	dma_enable_fifo_error_interrupt(DMA2, JPEG_OUT_DMA_CHANNEL);

//	jpeg_reset_output_fifo_dma_size_unter_anderem();

	jpeg_start_fifo_dma();

	/* enable dma */
	JPEG_CR |= JPEG_CR_IDMAEN | JPEG_CR_ODMAEN;
}
void JPEG_IN_DMA_ISR() {
	dma_isr_flags_t flags = dma_get_interrupt_flags(DMA2, JPEG_IN_DMA_STREAM, DMA_ISR_FLAGS);
	if (!flags) return;
	/* check for errors */
	dma_clear_interrupt_flags(DMA2, JPEG_IN_DMA_STREAM, flags);
	/* check for end of decode? */
	//jpeg_enable_fifo_dma(); // let someone else stop dma
}
void JPEG_OUT_DMA_ISR() {
	dma_isr_flags_t flags = dma_get_interrupt_flags(DMA2, JPEG_OUT_DMA_STREAM, DMA_ISR_FLAGS);
	if (!flags) return;
	/* check for errors */
	dma_clear_interrupt_flags(DMA2, JPEG_OUT_DMA_STREAM, flags);
//	/* check for end of decode? */
//	jpeg_reset_output_fifo_dma_size_unter_anderem(); // let someone else stop dma
}

//TBD
//static inline
//void jpeg_config_input_fifo_isr(void) {
//	/* disable dma */
//	JPEG_CR &= ~JPEG_CR_IDMAEN;
//}

/**
 *
 * @param in
 * @param in_size
 * @param out
 * @param out_size out buffer size
 */
void jpeg_decode(void *in, uint32_t in_size, void *out, uint32_t out_size) {
//	/* or disable.. */
//	JPEG_CONFR0 &= ~JPEG_CONFR0_START;

	/* enable jpeg core */
	if (!(JPEG_CR & JPEG_CR_JCEN)) {
		jpeg_init();
	}

	/* reset fifos */
	jpeg_reset_fifo_dma();

	/* decode */
	JPEG_CONFR1 |= JPEG_CONFR1_DE;

	/* start decoding operation */
	JPEG_CONFR0 |= JPEG_CONFR0_START;

	/* config dma */
	jpeg_config_input_fifo_dma(in, in_size);
	jpeg_config_output_fifo_dma(out, out_size);

	/* enable dma */
	jpeg_enable_fifo_dma();

	/* enable interrupts */
	JPEG_CR |= JPEG_CR_HPDIE
			 | JPEG_CR_EOCIE
			 | JPEG_CR_OFNEIE
			 | JPEG_CR_OFTIE
			 | JPEG_CR_IFNFIE
			 | JPEG_CR_IFTIE;
	nvic_enable_irq(NVIC_JPEG_IRQ);
	nvic_enable_irq(JPEG_IN_DMA_IRQ);
	nvic_enable_irq(JPEG_OUT_DMA_IRQ);
}
void jpeg_isr() {
	jpeg_sr_flags_t status = JPEG_SR;
	JPEG_CFR |= status & (JPEG_CFR_CEOCF | JPEG_CFR_CHPDF);
}
