

/*
* Purpose: DMA driver/interface
*
* Author: Tristan Yang
*
* Date: 2007/09/21
*
* Copyright Generalplus Corp. ALL RIGHTS RESERVED.
*
* Version : 1.11
*/
#include "driver_l1.h"
#include "drv_l1_sfr.h"
#include "drv_l1_dma.h"

#if (							defined _DRV_L1_DMA) && (_DRV_L1_DMA == 1)

#define C_DMA_CHANNEL_NUM		8
#define C_DMA_CH0				0
#define C_DMA_CH1				1
#define C_DMA_CH2				2
#define C_DMA_CH3				3
#define C_DMA_CH4				4
#define C_DMA_CH5				5
#define C_DMA_CH6				6
#define C_DMA_CH7				7

#define C_DMA_STATUS_WAITING	0
#define C_DMA_STATUS_DONE		1
#define C_DMA_STATUS_TIMEOUT	-1

#define C_DMA_IO_ADDR_START 	0xC0000000
#define C_DMA_IO_ADDR_END		0xDFFFFFFF

// DMA Control register
#define C_DMA_CTRL_ENABLE		0x00000001
#define C_DMA_CTRL_BUSY 		0x00000002
#define C_DMA_CTRL_SOFTWARE 	0x00000000
#define C_DMA_CTRL_EXTERNAL 	0x00000004
#define C_DMA_CTRL_NORMAL_INT	0x00000008
#define C_DMA_CTRL_DEST_INCREASE 0x00000000
#define C_DMA_CTRL_DEST_DECREASE 0x00000010
#define C_DMA_CTRL_SRC_INCREASE 0x00000000
#define C_DMA_CTRL_SRC_DECREASE 0x00000020
#define C_DMA_CTRL_DEST_FIX 	0x00000040
#define C_DMA_CTRL_SRC_FIX		0x00000080
#define C_DMA_CTRL_INT			0x00000100
#define C_DMA_CTRL_RESET		0x00000200
#define C_DMA_CTRL_M2M			0x00000000
#define C_DMA_CTRL_M2IO 		0x00000400
#define C_DMA_CTRL_IO2M 		0x00000800
#define C_DMA_CTRL_8BIT 		0x00000000
#define C_DMA_CTRL_16BIT		0x00001000
#define C_DMA_CTRL_32BIT		0x00002000
#define C_DMA_CTRL_SINGLE_TRANS 0x00000000
#define C_DMA_CTRL_DEMAND_TRANS 0x00004000
#define C_DMA_CTRL_DBF			0x00008000
#define C_DMA_CTRL_SINGLE_ACCESS 0x00000000
#define C_DMA_CTRL_BURST4_ACCESS 0x00010000
#define C_DMA_CTRL_BURST8_ACCESS 0x00020000

// DMA Transmit Count register
#define C_DMA_COUNT_MAX 		0x00FFFFFF

// DMA Sprite register
#define C_DMA_SPRITE_MAX		0x000003FF

// DMA Transparent register
#define C_DMA_TRANSPARENT_MAX	0x0000FFFF

// DMA Line register
#define C_DMA_LINE_MAX			0x000007FF

// DMA Miscellaneuous register
#define C_DMA_MISC_STATE_MASK	0x00000007
#define C_DMA_MISC_TIMEOUT_SHIFT 4
#define C_DMA_MISC_TIMEOUT_MAX	255
#define C_DMA_MISC_TIMEOUT_MASK 0x00000FF0
#define C_DMA_MISC_TRANSPARENT_EN 0x00001000
#define C_DMA_MISC_ERROR_WRITE	0x00004000
#define C_DMA_MISC_DMA_REQUEST	0x00008000

// DMA External IO register
#define C_DMA0_IO_SHIFT 		0
#define C_DMA1_IO_SHIFT 		4
#define C_DMA2_IO_SHIFT 		8
#define C_DMA3_IO_SHIFT 		12
#define C_DMA4_IO_SHIFT 		16
#define C_DMA5_IO_SHIFT 		20
#define C_DMA6_IO_SHIFT 		24
#define C_DMA7_IO_SHIFT 		28
#define C_DMA_IO_MASK			0xF

#define C_DMA_IO_CFC			0x00
#define C_DMA_IO_DAC_CHA		0x01		// Left channel
#define C_DMA_IO_UART_TX		0x02
#define C_DMA_IO_UART_RX		0x03
#define C_DMA_IO_SDC0			0x04
#define C_DMA_IO_SDC1			0x05
#define C_DMA_IO_6				0x06
#define C_DMA_IO_DAC_CHB		0x07		// Right channel
#define C_DMA_IO_ADC			0x08
#define C_DMA_IO_SPI0_TX		0x09
#define C_DMA_IO_SPI0_RX		0x0A
#define C_DMA_IO_SPI1_TX		0x0B
#define C_DMA_IO_SPI1_RX		0x0C
#define C_DMA_IO_13 			0x0D
#define C_DMA_IO_I2S_TX 		0x0E
#define C_DMA_IO_I2S_RX 		0x0F
#define C_DMA_IO_MAX			0x0F
#define C_DMA_MEMORY			0xFF

// DMA channel enable register
#define C_DMA_CE_DONT_RESET 	0x00000001

// DMA INT register
#define C_DMA0_INT_PEND 		0x00000001
#define C_DMA1_INT_PEND 		0x00000002
#define C_DMA2_INT_PEND 		0x00000004
#define C_DMA3_INT_PEND 		0x00000008
#define C_DMA0_TIMEOUT			0x00000010
#define C_DMA1_TIMEOUT			0x00000020
#define C_DMA2_TIMEOUT			0x00000040
#define C_DMA3_TIMEOUT			0x00000080
#define C_DMA0_BUSY 			0x00000100
#define C_DMA1_BUSY 			0x00000200
#define C_DMA2_BUSY 			0x00000400
#define C_DMA3_BUSY 			0x00000800
#define C_DMA4_INT_PEND 		0x00001000
#define C_DMA5_INT_PEND 		0x00002000
#define C_DMA6_INT_PEND 		0x00004000
#define C_DMA7_INT_PEND 		0x00008000
#define C_DMA4_TIMEOUT			0x00010000
#define C_DMA5_TIMEOUT			0x00020000
#define C_DMA6_TIMEOUT			0x00040000
#define C_DMA7_TIMEOUT			0x00080000
#define C_DMA4_BUSY 			0x00100000
#define C_DMA5_BUSY 			0x00200000
#define C_DMA6_BUSY 			0x00400000
#define C_DMA7_BUSY 			0x00800000

///////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////
#define C_DMA_Q_BUF_SIZE		1
#define C_DMA_Q_NUM 			C_DMA_CHANNEL_NUM

#define C_DMA_NOT_UESED 		0
#define C_DMA_NORMAL_USED		1
#define C_DMA_OCCUPIED			2

static INT8U dma_init_done = FALSE;
static INT8U dma_usage[C_DMA_CHANNEL_NUM];
static INT32U dma_notify_variable[C_DMA_CHANNEL_NUM];

#if _OPERATING_SYSTEM			!= _OS_NONE
static INT32U dma_notify_queue[C_DMA_CHANNEL_NUM];

static void * q_buffer[C_DMA_Q_NUM][C_DMA_Q_BUF_SIZE];
static OS_EVENT * dma_driver_queue[C_DMA_Q_NUM] =
{
	NULL
};


static INT8U dma_q_usage[C_DMA_Q_NUM];
#endif

static INT32U dma_get_channel(INT8U usage);
static void dma_free_channel(INT32U channel);

#if _OPERATING_SYSTEM			!= _OS_NONE
static INT32U dma_get_queue(void);
static void dma_free_queue(INT32U q_index);
#endif

static void dma_set_notify(INT32U channel, INT32U notify, INT32U os_q);
static INT32S dma_device_protect(void);
static void dma_device_unprotect(INT32S mask);
static void dma_isr(void);

static void dma_set_control(INT32U channel, INT32U ctrl);
static void dma_set_source(INT32U channel, INT32U addr);
static void dma_set_target(INT32U channel, INT32U addr);
static INT32S dma_set_tx_count(INT32U channel, INT32U count);
static INT32S dma_set_device(INT32U channel, INT16U device);
static INT32S dma_set_timeout(INT32U channel, INT32U timeout);
static INT32S dma_set_line_length(INT32U length);
static INT32S dma_set_sprite_size(INT32U channel, INT32U size);

#if 0
static INT32S dma_set_transparent_enable(INT32U channel);
static INT32S dma_set_transparent_disable(INT32U channel);
static INT32S dma_set_transparent_pattern(INT32U channel, INT16U pattern);
#endif

static INT32S dma_transfer_extend(DMA_STRUCT * dma_struct, INT8U usage, INT32U os_q);

INT32U target_addr;

#if _OPERATING_SYSTEM			!= _OS_NONE	
static OS_EVENT * sw_dma_sem = NULL;

void sw_dma_lock(void)
{
	INT8U err;

	OSSemPend(sw_dma_sem, 0, &err);
}


void sw_dma_unlock(void)
{
	OSSemPost(sw_dma_sem);
}


#endif


// Return 0~(C_DMA_CHANNEL_NUM-1) when one of DMA channel is available, C_DMA_CHANNEL_NUM if none of them is free
static INT32U dma_get_channel(INT8U usage)
{
	int i;
	INT32S mask;

#if _OPERATING_SYSTEM				!= _OS_NONE				// Soft Protect for critical section
	sw_dma_lock();
#endif

	mask				= dma_device_protect(); 	// Device Protect for critical section

	if (target_addr > 0xF8000000)
	{
		for (i = C_DMA_CHANNEL_NUM - 1; i >= 0; i--)
		{

			// 若 AES 功能 disable，CH2 不能動（所以乾脆不用）
			if (i == 2)
				continue;

			if (dma_usage[i] == C_DMA_NOT_UESED)
			{
				dma_usage[i]		= usage;

				dma_device_unprotect(mask);

#if _OPERATING_SYSTEM							!= _OS_NONE
				sw_dma_unlock();
#endif

				// Reset DMA controller
				dma_set_control(i, C_DMA_CTRL_RESET);

				//DBG_PRINT("DMA i(%d)\r\n",i);		  
				return i;
			}
		}
	}
	else 
	{
		for (i = 0; i < C_DMA_CHANNEL_NUM; i++)
		{

			// 若 AES 功能 disable，CH2 不能動（所以乾脆不用）
			if (i == 2)
			{
				continue;
			}

			if (dma_usage[i] == C_DMA_NOT_UESED)
			{
				dma_usage[i]		= usage;

				dma_device_unprotect(mask);

#if _OPERATING_SYSTEM							!= _OS_NONE
				sw_dma_unlock();
#endif

				// Reset DMA controller
				dma_set_control(i, C_DMA_CTRL_RESET);

				//DBG_PRINT("DMA s(%d)\r\n",i);
				return i;
			}
		}
	}

	dma_device_unprotect(mask);

#if _OPERATING_SYSTEM				!= _OS_NONE
	sw_dma_unlock();
#endif

	//DBG_PRINT("DMA channel full (i)\r\n",i);
	return i;
}


static void dma_free_channel(INT32U channel)
{
	if (channel >= C_DMA_CHANNEL_NUM)
	{
		return;
	}

	// Reset DMA controller
	dma_set_control(channel, C_DMA_CTRL_RESET);

	// Clear pending bit
	if (channel == C_DMA_CH0)
	{
		R_DMA_INT			= C_DMA0_INT_PEND;
	}
	else if (channel == C_DMA_CH1)
	{
		R_DMA_INT			= C_DMA1_INT_PEND;
	}
	else if (channel == C_DMA_CH2)
	{
		R_DMA_INT			= C_DMA2_INT_PEND;
	}
	else if (channel == C_DMA_CH3)
	{
		R_DMA_INT			= C_DMA3_INT_PEND;
	}
	else if (channel == C_DMA_CH4)
	{
		R_DMA_INT			= C_DMA4_INT_PEND;
	}
	else if (channel == C_DMA_CH5)
	{
		R_DMA_INT			= C_DMA5_INT_PEND;
	}
	else if (channel == C_DMA_CH6)
	{
		R_DMA_INT			= C_DMA6_INT_PEND;
	}
	else if (channel == C_DMA_CH7)
	{
		R_DMA_INT			= C_DMA7_INT_PEND;
	}

	dma_notify_variable[channel] = (INT32U)
	NULL;

#if _OPERATING_SYSTEM				!= _OS_NONE
	dma_notify_queue[channel] = (INT32U)
	NULL;
#endif

	dma_usage[channel]	= (INT8U)
	C_DMA_NOT_UESED;
}


#if _OPERATING_SYSTEM			!= _OS_NONE
static INT32U dma_get_queue(void)
{
	INT32U i;

	sw_dma_lock();

	for (i = 0; i < C_DMA_Q_NUM; i++)
	{
		if (!dma_q_usage[i])
		{
			dma_q_usage[i]		= (INT8U)
			TRUE;

			sw_dma_unlock();

			if (dma_driver_queue[i])
			{
				OSQFlush(dma_driver_queue[i]);
			}
			else 
			{
				dma_driver_queue[i] = OSQCreate(&q_buffer[i][0], C_DMA_Q_BUF_SIZE);
			}

			return i;
		}
	}

	sw_dma_unlock();

	return i;
}


static void dma_free_queue(INT32U q_index)
{
	if (q_index >= C_DMA_Q_NUM)
	{
		return;
	}

	dma_q_usage[q_index] = (INT8U)
	FALSE;
}


#endif

static void dma_set_notify(INT32U channel, INT32U notify, INT32U os_q)
{
	dma_notify_variable[channel] = notify;

#if _OPERATING_SYSTEM				!= _OS_NONE
	dma_notify_queue[channel] = os_q;

#else

	os_q				= os_q; 					// Prevent compiler warning
#endif
}


static INT32S dma_device_protect(void)
{
	return vic_irq_disable(VIC_DMA);
}


static void dma_device_unprotect(INT32S mask)
{
	if (mask == 0)
	{ // Clear device interrupt mask
		vic_irq_enable(VIC_DMA);
	}
	else if (mask == 1)
	{
		vic_irq_disable(VIC_DMA);
	}
	else 
	{ // Something is wrong, do nothing
		return;
	}
}


static void dma_isr(void) // Device ISR
{
	INT32U pending;

	pending 			= R_DMA_INT;

	// DMA0
	if (pending & C_DMA0_INT_PEND)
	{ // DMA0 interrupt is pending

		if (dma_usage[C_DMA_CH0] != C_DMA_NOT_UESED)
		{
			if (pending & C_DMA0_TIMEOUT)
			{ // DMA transmit timeout

				if (dma_notify_variable[C_DMA_CH0])
				{
					* ((INT8S *) dma_notify_variable[C_DMA_CH0]) = (INT8S)C_DMA_STATUS_TIMEOUT;
				}

#if _OPERATING_SYSTEM							!= _OS_NONE

				if (dma_notify_queue[C_DMA_CH0])
				{
					OSQPost((OS_EVENT *) dma_notify_queue[C_DMA_CH0], (void *) C_DMA_STATUS_TIMEOUT);
				}

#endif
			}
			else 
			{
				if (dma_notify_variable[C_DMA_CH0])
				{
					* ((INT8S *) dma_notify_variable[C_DMA_CH0]) = (INT8S)
					C_DMA_STATUS_DONE;
				}

#if _OPERATING_SYSTEM							!= _OS_NONE

				if (dma_notify_queue[C_DMA_CH0])
				{
					OSQPost((OS_EVENT *) dma_notify_queue[C_DMA_CH0], (void *) C_DMA_STATUS_DONE);
				}

#endif
			}

			if (dma_usage[C_DMA_CH0] != C_DMA_OCCUPIED)
			{
				dma_free_channel(C_DMA_CH0);
			}
		}

		R_DMA_INT			= C_DMA0_INT_PEND;		// Clear pending bit
	}

	// DMA1
	if (pending & C_DMA1_INT_PEND)
	{ // DMA1 interrupt is pending

		if (dma_usage[C_DMA_CH1] != C_DMA_NOT_UESED)
		{
			if (pending & C_DMA1_TIMEOUT)
			{ // DMA transmit timeout

				if (dma_notify_variable[C_DMA_CH1])
				{
					* ((INT8S *) dma_notify_variable[C_DMA_CH1]) = (INT8S)
					C_DMA_STATUS_TIMEOUT;
				}

#if _OPERATING_SYSTEM							!= _OS_NONE

				if (dma_notify_queue[C_DMA_CH1])
				{
					OSQPost((OS_EVENT *) dma_notify_queue[C_DMA_CH1], (void *) C_DMA_STATUS_TIMEOUT);
				}

#endif
			}
			else 
			{
				if (dma_notify_variable[C_DMA_CH1])
				{
					* ((INT8S *) dma_notify_variable[C_DMA_CH1]) = (INT8S)
					C_DMA_STATUS_DONE;
				}

#if _OPERATING_SYSTEM							!= _OS_NONE

				if (dma_notify_queue[C_DMA_CH1])
				{
					OSQPost((OS_EVENT *) dma_notify_queue[C_DMA_CH1], (void *) C_DMA_STATUS_DONE);
				}

#endif
			}

			if (dma_usage[C_DMA_CH1] != C_DMA_OCCUPIED)
			{
				dma_free_channel(C_DMA_CH1);
			}
		}

		R_DMA_INT			= C_DMA1_INT_PEND;		// Clear pending bit
	}

	// DMA2
	if (pending & C_DMA2_INT_PEND)
	{ // DMA2 interrupt is pending

		if (dma_usage[C_DMA_CH2] != C_DMA_NOT_UESED)
		{
			if (pending & C_DMA2_TIMEOUT)
			{ // DMA transmit timeout

				if (dma_notify_variable[C_DMA_CH2])
				{
					* ((INT8S *) dma_notify_variable[C_DMA_CH2]) = (INT8S)
					C_DMA_STATUS_TIMEOUT;
				}

#if _OPERATING_SYSTEM							!= _OS_NONE

				if (dma_notify_queue[C_DMA_CH2])
				{
					OSQPost((OS_EVENT *) dma_notify_queue[C_DMA_CH2], (void *) C_DMA_STATUS_TIMEOUT);
				}

#endif
			}
			else 
			{
				if (dma_notify_variable[C_DMA_CH2])
				{
					* ((INT8S *) dma_notify_variable[C_DMA_CH2]) = (INT8S)
					C_DMA_STATUS_DONE;
				}

#if _OPERATING_SYSTEM							!= _OS_NONE

				if (dma_notify_queue[C_DMA_CH2])
				{
					OSQPost((OS_EVENT *) dma_notify_queue[C_DMA_CH2], (void *) C_DMA_STATUS_DONE);
				}

#endif
			}

			if (dma_usage[C_DMA_CH2] != C_DMA_OCCUPIED)
			{
				dma_free_channel(C_DMA_CH2);
			}
		}

		R_DMA_INT			= C_DMA2_INT_PEND;		// Clear pending bit
	}

	// DMA3
	if (pending & C_DMA3_INT_PEND)
	{ // DMA3 interrupt is pending

		if (dma_usage[C_DMA_CH3] != C_DMA_NOT_UESED)
		{
			if (pending & C_DMA3_TIMEOUT)
			{ // DMA transmit timeout

				if (dma_notify_variable[C_DMA_CH3])
				{
					* ((INT8S *) dma_notify_variable[C_DMA_CH3]) = (INT8S)
					C_DMA_STATUS_TIMEOUT;
				}

#if _OPERATING_SYSTEM							!= _OS_NONE

				if (dma_notify_queue[C_DMA_CH3])
				{
					OSQPost((OS_EVENT *) dma_notify_queue[C_DMA_CH3], (void *) C_DMA_STATUS_TIMEOUT);
				}

#endif
			}
			else 
			{
				if (dma_notify_variable[C_DMA_CH3])
				{
					* ((INT8S *) dma_notify_variable[C_DMA_CH3]) = (INT8S)
					C_DMA_STATUS_DONE;
				}

#if _OPERATING_SYSTEM							!= _OS_NONE

				if (dma_notify_queue[C_DMA_CH3])
				{
					OSQPost((OS_EVENT *) dma_notify_queue[C_DMA_CH3], (void *) C_DMA_STATUS_DONE);
				}

#endif
			}

			if (dma_usage[C_DMA_CH3] != C_DMA_OCCUPIED)
			{
				dma_free_channel(C_DMA_CH3);
			}
		}

		R_DMA_INT			= C_DMA3_INT_PEND;		// Clear pending bit
	}

	// DMA4
	if (pending & C_DMA4_INT_PEND)
	{ // DMA4 interrupt is pending

		if (dma_usage[C_DMA_CH4] != C_DMA_NOT_UESED)
		{
			if (pending & C_DMA4_TIMEOUT)
			{ // DMA transmit timeout

				if (dma_notify_variable[C_DMA_CH4])
				{
					* ((INT8S *) dma_notify_variable[C_DMA_CH4]) = (INT8S)
					C_DMA_STATUS_TIMEOUT;
				}

#if _OPERATING_SYSTEM							!= _OS_NONE

				if (dma_notify_queue[C_DMA_CH4])
				{
					OSQPost((OS_EVENT *) dma_notify_queue[C_DMA_CH4], (void *) C_DMA_STATUS_TIMEOUT);
				}

#endif
			}
			else 
			{
				if (dma_notify_variable[C_DMA_CH4])
				{
					* ((INT8S *) dma_notify_variable[C_DMA_CH4]) = (INT8S)
					C_DMA_STATUS_DONE;
				}

#if _OPERATING_SYSTEM							!= _OS_NONE

				if (dma_notify_queue[C_DMA_CH4])
				{
					OSQPost((OS_EVENT *) dma_notify_queue[C_DMA_CH4], (void *) C_DMA_STATUS_DONE);
				}

#endif
			}

			if (dma_usage[C_DMA_CH4] != C_DMA_OCCUPIED)
			{
				dma_free_channel(C_DMA_CH4);
			}
		}

		R_DMA_INT			= C_DMA4_INT_PEND;		// Clear pending bit
	}

	// DMA5
	if (pending & C_DMA5_INT_PEND)
	{ // DMA5 interrupt is pending

		if (dma_usage[C_DMA_CH5] != C_DMA_NOT_UESED)
		{
			if (pending & C_DMA5_TIMEOUT)
			{ // DMA transmit timeout

				if (dma_notify_variable[C_DMA_CH5])
				{
					* ((INT8S *) dma_notify_variable[C_DMA_CH5]) = (INT8S)
					C_DMA_STATUS_TIMEOUT;
				}

#if _OPERATING_SYSTEM							!= _OS_NONE

				if (dma_notify_queue[C_DMA_CH5])
				{
					OSQPost((OS_EVENT *) dma_notify_queue[C_DMA_CH5], (void *) C_DMA_STATUS_TIMEOUT);
				}

#endif
			}
			else 
			{
				if (dma_notify_variable[C_DMA_CH5])
				{
					* ((INT8S *) dma_notify_variable[C_DMA_CH5]) = (INT8S)
					C_DMA_STATUS_DONE;
				}

#if _OPERATING_SYSTEM							!= _OS_NONE

				if (dma_notify_queue[C_DMA_CH5])
				{
					OSQPost((OS_EVENT *) dma_notify_queue[C_DMA_CH5], (void *) C_DMA_STATUS_DONE);
				}

#endif
			}

			if (dma_usage[C_DMA_CH5] != C_DMA_OCCUPIED)
			{
				dma_free_channel(C_DMA_CH5);
			}
		}

		R_DMA_INT			= C_DMA5_INT_PEND;		// Clear pending bit
	}

	// DMA6
	if (pending & C_DMA6_INT_PEND)
	{ // DMA6 interrupt is pending

		if (dma_usage[C_DMA_CH6] != C_DMA_NOT_UESED)
		{
			if (pending & C_DMA6_TIMEOUT)
			{ // DMA transmit timeout

				if (dma_notify_variable[C_DMA_CH6])
				{
					* ((INT8S *) dma_notify_variable[C_DMA_CH6]) = (INT8S)
					C_DMA_STATUS_TIMEOUT;
				}

#if _OPERATING_SYSTEM							!= _OS_NONE

				if (dma_notify_queue[C_DMA_CH6])
				{
					OSQPost((OS_EVENT *) dma_notify_queue[C_DMA_CH6], (void *) C_DMA_STATUS_TIMEOUT);
				}

#endif
			}
			else 
			{
				if (dma_notify_variable[C_DMA_CH6])
				{
					* ((INT8S *) dma_notify_variable[C_DMA_CH6]) = (INT8S)
					C_DMA_STATUS_DONE;
				}

#if _OPERATING_SYSTEM							!= _OS_NONE

				if (dma_notify_queue[C_DMA_CH6])
				{
					OSQPost((OS_EVENT *) dma_notify_queue[C_DMA_CH6], (void *) C_DMA_STATUS_DONE);
				}

#endif
			}

			if (dma_usage[C_DMA_CH6] != C_DMA_OCCUPIED)
			{
				dma_free_channel(C_DMA_CH6);
			}
		}

		R_DMA_INT			= C_DMA6_INT_PEND;		// Clear pending bit
	}

	// DMA7
	if (pending & C_DMA7_INT_PEND)
	{ // DMA7 interrupt is pending

		if (dma_usage[C_DMA_CH7] != C_DMA_NOT_UESED)
		{
			if (pending & C_DMA7_TIMEOUT)
			{ // DMA transmit timeout

				if (dma_notify_variable[C_DMA_CH7])
				{
					* ((INT8S *) dma_notify_variable[C_DMA_CH7]) = (INT8S)
					C_DMA_STATUS_TIMEOUT;
				}

#if _OPERATING_SYSTEM							!= _OS_NONE

				if (dma_notify_queue[C_DMA_CH7])
				{
					OSQPost((OS_EVENT *) dma_notify_queue[C_DMA_CH7], (void *) C_DMA_STATUS_TIMEOUT);
				}

#endif
			}
			else 
			{
				if (dma_notify_variable[C_DMA_CH7])
				{
					* ((INT8S *) dma_notify_variable[C_DMA_CH7]) = (INT8S)
					C_DMA_STATUS_DONE;
				}

#if _OPERATING_SYSTEM							!= _OS_NONE

				if (dma_notify_queue[C_DMA_CH7])
				{
					OSQPost((OS_EVENT *) dma_notify_queue[C_DMA_CH7], (void *) C_DMA_STATUS_DONE);
				}

#endif
			}

			if (dma_usage[C_DMA_CH7] != C_DMA_OCCUPIED)
			{
				dma_free_channel(C_DMA_CH7);
			}
		}

		R_DMA_INT			= C_DMA7_INT_PEND;		// Clear pending bit
	}

}


void dma_init(void)
{
	INT32U i;

	R_DMA0_CTRL 		= C_DMA_CTRL_RESET; 		// Software reset, this bit will auto clear after reset complete
	R_DMA0_TX_COUNT 	= 0x0;
	R_DMA0_SPRITE_SIZE	= 0x0;
	R_DMA0_TRANSPARENT	= 0x0;
	R_DMA0_MISC 		= 0x0;

	R_DMA1_CTRL 		= C_DMA_CTRL_RESET;
	R_DMA1_TX_COUNT 	= 0x0;
	R_DMA1_SPRITE_SIZE	= 0x0;
	R_DMA1_TRANSPARENT	= 0x0;
	R_DMA1_MISC 		= 0x0;

	R_DMA2_CTRL 		= C_DMA_CTRL_RESET;
	R_DMA2_TX_COUNT 	= 0x0;
	R_DMA2_SPRITE_SIZE	= 0x0;
	R_DMA2_TRANSPARENT	= 0x0;
	R_DMA2_MISC 		= 0x0;

	R_DMA3_CTRL 		= C_DMA_CTRL_RESET;
	R_DMA3_TX_COUNT 	= 0x0;
	R_DMA3_SPRITE_SIZE	= 0x0;
	R_DMA3_TRANSPARENT	= 0x0;
	R_DMA3_MISC 		= 0x0;

	R_DMA4_CTRL 		= C_DMA_CTRL_RESET;
	R_DMA4_TX_COUNT 	= 0x0;
	R_DMA4_SPRITE_SIZE	= 0x0;
	R_DMA4_TRANSPARENT	= 0x0;
	R_DMA4_MISC 		= 0x0;

	R_DMA5_CTRL 		= C_DMA_CTRL_RESET;
	R_DMA5_TX_COUNT 	= 0x0;
	R_DMA5_SPRITE_SIZE	= 0x0;
	R_DMA5_TRANSPARENT	= 0x0;
	R_DMA5_MISC 		= 0x0;

	R_DMA6_CTRL 		= C_DMA_CTRL_RESET;
	R_DMA6_TX_COUNT 	= 0x0;
	R_DMA6_SPRITE_SIZE	= 0x0;
	R_DMA6_TRANSPARENT	= 0x0;
	R_DMA6_MISC 		= 0x0;

	R_DMA7_CTRL 		= C_DMA_CTRL_RESET;
	R_DMA7_TX_COUNT 	= 0x0;
	R_DMA7_SPRITE_SIZE	= 0x0;
	R_DMA7_TRANSPARENT	= 0x0;
	R_DMA7_MISC 		= 0x0;

	R_DMA_LINE_LEN		= 0x0;
	R_DMA_DEVICE		=
		 (C_DMA_IO_UART_RX << C_DMA3_IO_SHIFT) | (C_DMA_IO_UART_TX << C_DMA2_IO_SHIFT) | (C_DMA_IO_DAC_CHA << C_DMA1_IO_SHIFT) | (C_DMA_IO_CFC << C_DMA0_IO_SHIFT);
													// 0x3210
	R_DMA_CEMODE		= C_DMA_CE_DONT_RESET;
	R_DMA_INT			= 0xFFFFFFFF;				// Clear all pending bits

	for (i = 0; i < C_DMA_CHANNEL_NUM; i++)
	{
		dma_usage[i]		= (INT8U)C_DMA_NOT_UESED;
		dma_notify_variable[i] = (INT32U)NULL;

#if _OPERATING_SYSTEM					!= _OS_NONE
		dma_notify_queue[i] = (INT32U)NULL;
#endif
	}

#if _OPERATING_SYSTEM				!= _OS_NONE

	for (i = 0; i < C_DMA_Q_NUM; i++)
	{
		dma_q_usage[i]		= (INT8U)FALSE;

		if (dma_driver_queue[i])
		{
			OSQFlush(dma_driver_queue[i]);
		}
		else 
		{
			dma_driver_queue[i] = OSQCreate(&q_buffer[i][0], C_DMA_Q_BUF_SIZE);
		}
	}

#endif

#if _OPERATING_SYSTEM				!= _OS_NONE

	if (sw_dma_sem == NULL)
	{
		sw_dma_sem			= OSSemCreate(1);
	}

#endif

	vic_irq_register(VIC_DMA, dma_isr);
	vic_irq_enable(VIC_DMA);

	dma_init_done		= TRUE;
}


static void dma_set_control(INT32U channel, INT32U ctrl)
{
	switch (channel)
	{
		case C_DMA_CH0:
			R_DMA0_CTRL = ctrl;
			break;

		case C_DMA_CH1:
			R_DMA1_CTRL = ctrl;
			break;

		case C_DMA_CH2:
			R_DMA2_CTRL = ctrl;
			break;

		case C_DMA_CH3:
			R_DMA3_CTRL = ctrl;
			break;

		case C_DMA_CH4:
			R_DMA4_CTRL = ctrl;
			break;

		case C_DMA_CH5:
			R_DMA5_CTRL = ctrl;
			break;

		case C_DMA_CH6:
			R_DMA6_CTRL = ctrl;
			break;

		case C_DMA_CH7:
			R_DMA7_CTRL = ctrl;
			break;

		default:
			break;
	}
}


static void dma_set_source(INT32U channel, INT32U addr)
{
	switch (channel)
	{
		case C_DMA_CH0:
			R_DMA0_SRC_ADDR = addr;
			break;

		case C_DMA_CH1:
			R_DMA1_SRC_ADDR = addr;
			break;

		case C_DMA_CH2:
			R_DMA2_SRC_ADDR = addr;
			break;

		case C_DMA_CH3:
			R_DMA3_SRC_ADDR = addr;
			break;

		case C_DMA_CH4:
			R_DMA4_SRC_ADDR = addr;
			break;

		case C_DMA_CH5:
			R_DMA5_SRC_ADDR = addr;
			break;

		case C_DMA_CH6:
			R_DMA6_SRC_ADDR = addr;
			break;

		case C_DMA_CH7:
			R_DMA7_SRC_ADDR = addr;
			break;

		default:
			break;
	}
}


static void dma_set_target(INT32U channel, INT32U addr)
{
	switch (channel)
	{
		case C_DMA_CH0:
			R_DMA0_TAR_ADDR = addr;
			break;

		case C_DMA_CH1:
			R_DMA1_TAR_ADDR = addr;
			break;

		case C_DMA_CH2:
			R_DMA2_TAR_ADDR = addr;
			break;

		case C_DMA_CH3:
			R_DMA3_TAR_ADDR = addr;
			break;

		case C_DMA_CH4:
			R_DMA4_TAR_ADDR = addr;
			break;

		case C_DMA_CH5:
			R_DMA5_TAR_ADDR = addr;
			break;

		case C_DMA_CH6:
			R_DMA6_TAR_ADDR = addr;
			break;

		case C_DMA_CH7:
			R_DMA7_TAR_ADDR = addr;
			break;

		default:
			break;
	}
}


static INT32S dma_set_tx_count(INT32U channel, INT32U count)
{
	if (count > C_DMA_COUNT_MAX)
	{
		return - 1;
	}

	switch (channel)
	{
		case C_DMA_CH0:
			R_DMA0_TX_COUNT = count;
			break;

		case C_DMA_CH1:
			R_DMA1_TX_COUNT = count;
			break;

		case C_DMA_CH2:
			R_DMA2_TX_COUNT = count;
			break;

		case C_DMA_CH3:
			R_DMA3_TX_COUNT = count;
			break;

		case C_DMA_CH4:
			R_DMA4_TX_COUNT = count;
			break;

		case C_DMA_CH5:
			R_DMA5_TX_COUNT = count;
			break;

		case C_DMA_CH6:
			R_DMA6_TX_COUNT = count;
			break;

		case C_DMA_CH7:
			R_DMA7_TX_COUNT = count;
			break;

		default:
			return - 1;
	}

	return 0;
}


static INT32S dma_set_device(INT32U channel, INT16U device)
{
	INT32U shift		= 0;

	if ((channel >= C_DMA_CHANNEL_NUM) || (device > C_DMA_IO_MAX))
	{
		return - 1;
	}

	if (channel == C_DMA_CH0)
	{
		shift				= C_DMA0_IO_SHIFT;
	}
	else if (channel == C_DMA_CH1)
	{
		shift				= C_DMA1_IO_SHIFT;
	}
	else if (channel == C_DMA_CH2)
	{
		shift				= C_DMA2_IO_SHIFT;
	}
	else if (channel == C_DMA_CH3)
	{
		shift				= C_DMA3_IO_SHIFT;
	}
	else if (channel == C_DMA_CH4)
	{
		shift				= C_DMA4_IO_SHIFT;
	}
	else if (channel == C_DMA_CH5)
	{
		shift				= C_DMA5_IO_SHIFT;
	}
	else if (channel == C_DMA_CH6)
	{
		shift				= C_DMA6_IO_SHIFT;
	}
	else if (channel == C_DMA_CH7)
	{
		shift				= C_DMA7_IO_SHIFT;
	}

	R_DMA_DEVICE		&= ~((C_DMA_IO_MASK) << shift);
	R_DMA_DEVICE		|= device << shift;

	return 0;
}


static INT32S dma_set_timeout(INT32U channel, INT32U timeout)
{
	if ((channel >= C_DMA_CHANNEL_NUM) || (timeout > C_DMA_MISC_TIMEOUT_MAX))
	{
		return - 1;
	}

	switch (channel)
	{
		case C_DMA_CH0:
			R_DMA0_MISC &= ~(C_DMA_MISC_TIMEOUT_MASK);
			R_DMA0_MISC |= timeout << C_DMA_MISC_TIMEOUT_SHIFT;
			break;

		case C_DMA_CH1:
			R_DMA1_MISC &= ~(C_DMA_MISC_TIMEOUT_MASK);
			R_DMA1_MISC |= timeout << C_DMA_MISC_TIMEOUT_SHIFT;
			break;

		case C_DMA_CH2:
			R_DMA2_MISC &= ~(C_DMA_MISC_TIMEOUT_MASK);
			R_DMA2_MISC |= timeout << C_DMA_MISC_TIMEOUT_SHIFT;
			break;

		case C_DMA_CH3:
			R_DMA3_MISC &= ~(C_DMA_MISC_TIMEOUT_MASK);
			R_DMA3_MISC |= timeout << C_DMA_MISC_TIMEOUT_SHIFT;
			break;

		case C_DMA_CH4:
			R_DMA4_MISC &= ~(C_DMA_MISC_TIMEOUT_MASK);
			R_DMA4_MISC |= timeout << C_DMA_MISC_TIMEOUT_SHIFT;
			break;

		case C_DMA_CH5:
			R_DMA5_MISC &= ~(C_DMA_MISC_TIMEOUT_MASK);
			R_DMA5_MISC |= timeout << C_DMA_MISC_TIMEOUT_SHIFT;
			break;

		case C_DMA_CH6:
			R_DMA6_MISC &= ~(C_DMA_MISC_TIMEOUT_MASK);
			R_DMA6_MISC |= timeout << C_DMA_MISC_TIMEOUT_SHIFT;
			break;

		case C_DMA_CH7:
			R_DMA7_MISC &= ~(C_DMA_MISC_TIMEOUT_MASK);
			R_DMA7_MISC |= timeout << C_DMA_MISC_TIMEOUT_SHIFT;
			break;

		default:
			break;
	}

	return 0;
}


static INT32S dma_set_line_length(INT32U length)
{
	if (length > C_DMA_LINE_MAX)
	{
		return - 1;
	}

	R_DMA_LINE_LEN		= length;

	return 0;
}


static INT32S dma_set_sprite_size(INT32U channel, INT32U size)
{
	if (size > C_DMA_SPRITE_MAX)
	{
		return - 1;
	}

	switch (channel)
	{
		case C_DMA_CH0:
			R_DMA0_SPRITE_SIZE = size;
			break;

		case C_DMA_CH1:
			R_DMA1_SPRITE_SIZE = size;
			break;

		case C_DMA_CH2:
			R_DMA2_SPRITE_SIZE = size;
			break;

		case C_DMA_CH3:
			R_DMA3_SPRITE_SIZE = size;
			break;

		case C_DMA_CH4:
			R_DMA4_SPRITE_SIZE = size;
			break;

		case C_DMA_CH5:
			R_DMA5_SPRITE_SIZE = size;
			break;

		case C_DMA_CH6:
			R_DMA6_SPRITE_SIZE = size;
			break;

		case C_DMA_CH7:
			R_DMA7_SPRITE_SIZE = size;
			break;

		default:
			return - 1;
	}

	return 0;
}


static INT32S dma_transfer_extend(DMA_STRUCT * dma_struct, INT8U usage, INT32U os_q)
{
	INT32U s_addr		= dma_struct->s_addr;
	INT32U t_addr		= dma_struct->t_addr;
	INT32U count		= dma_struct->count;
	INT32U channel, ctrl;
	INT16U src_type, target_type;

#if (								defined _DRV_L1_CACHE) && (_DRV_L1_CACHE == 1)

	// Invalid target memory from cache
	INT32U len;
#endif


	if (!count)
	{
		if (dma_struct->notify)
		{
			* (dma_struct->notify) = C_DMA_STATUS_DONE;
		}

		return 0;
	}

	if (dma_struct->width == DMA_DATA_WIDTH_1BYTE)
	{
		ctrl				= C_DMA_CTRL_8BIT | C_DMA_CTRL_INT | C_DMA_CTRL_NORMAL_INT | C_DMA_CTRL_ENABLE;

#if (									defined _DRV_L1_CACHE) && (_DRV_L1_CACHE == 1)

		// Invalid target memory from cache
		len 				= count;
#endif
	}
	else if (dma_struct->width == DMA_DATA_WIDTH_2BYTE)
	{
		if ((s_addr & 0x1) || (t_addr & 0x1))
		{
			return - 1; // Both source and target address must be 2-byte alignment
		}

		ctrl				= C_DMA_CTRL_16BIT | C_DMA_CTRL_INT | C_DMA_CTRL_NORMAL_INT | C_DMA_CTRL_ENABLE;

#if (									defined _DRV_L1_CACHE) && (_DRV_L1_CACHE == 1)

		// Invalid target memory from cache		
		len 				= count << 1;
#endif
	}
	else if (dma_struct->width == DMA_DATA_WIDTH_4BYTE)
	{
		if ((s_addr & 0x3) || (t_addr & 0x3))
		{
			return - 1; // Both source and target address must be 4-byte alignment
		}

		ctrl				= C_DMA_CTRL_32BIT | C_DMA_CTRL_INT | C_DMA_CTRL_NORMAL_INT | C_DMA_CTRL_ENABLE;

#if (									defined _DRV_L1_CACHE) && (_DRV_L1_CACHE == 1)

		// Invalid target memory from cache		
		len 				= count << 2;
#endif
	}
	else 
	{
		return - 1;
	}

	if ((s_addr <C_DMA_IO_ADDR_START) || (s_addr> C_DMA_IO_ADDR_END))
	{
		src_type			= C_DMA_MEMORY;
	}
	else 
	{
		if (s_addr == (INT32U)
		P_UART0_BASE)
		{
			src_type			= C_DMA_IO_UART_RX;
		}
		else if (s_addr == (INT32U)
		P_SDC0_DATA_RX)
		{
			src_type			= C_DMA_IO_SDC0;
		}
		else if (s_addr == (INT32U)
		P_SDC1_DATA_RX)
		{
			src_type			= C_DMA_IO_SDC1;
		}
		else if (s_addr == (INT32U)
		P_SPI0_RX_DATA)
		{
			src_type			= C_DMA_IO_SPI0_RX;
		}
		else if (s_addr == (INT32U)
		P_SPI1_RX_DATA)
		{
			src_type			= C_DMA_IO_SPI1_RX;
		}
		else if (s_addr == (INT32U)
		P_CFC_DMADATA)
		{
			src_type			= C_DMA_IO_CFC;
		}
		else if (s_addr == (INT32U)
		P_I2SRX_DATA)
		{
			src_type			= C_DMA_IO_I2S_RX;
		}
		else if (s_addr == (INT32U)
		P_ADC_ASADC_DATA)
		{
			src_type			= C_DMA_IO_ADC;
		}
		else if (s_addr == (INT32U)
		P_MIC_ASADC_DATA)
		{
			src_type			= C_DMA_IO_6;
		}
		else 
		{
			return - 4; // Unknow IO
		}
	}

	if ((t_addr <C_DMA_IO_ADDR_START) || (t_addr> C_DMA_IO_ADDR_END))
	{
		target_type 		= C_DMA_MEMORY;
	}
	else 
	{
		if (src_type != C_DMA_MEMORY)
		{
			return - 1; // IO to IO is not supported
		}

		if (t_addr == (INT32U)
		P_UART0_BASE)
		{
			target_type 		= C_DMA_IO_UART_TX;
		}
		else if (t_addr == (INT32U)
		P_SDC0_DATA_TX)
		{
			target_type 		= C_DMA_IO_SDC0;
		}
		else if (t_addr == (INT32U)
		P_SDC1_DATA_TX)
		{
			target_type 		= C_DMA_IO_SDC1;
		}
		else if (t_addr == (INT32U)
		P_SPI0_TX_DATA)
		{
			target_type 		= C_DMA_IO_SPI0_TX;
		}
		else if (t_addr == (INT32U)
		P_SPI1_TX_DATA)
		{
			target_type 		= C_DMA_IO_SPI1_TX;
		}
		else if (t_addr == (INT32U)
		P_DAC_CHA_DATA)
		{
			target_type 		= C_DMA_IO_DAC_CHA;
		}
		else if (t_addr == (INT32U)
		P_DAC_CHB_DATA)
		{
			target_type 		= C_DMA_IO_DAC_CHB;
		}
		else if (t_addr == (INT32U)
		P_CFC_DMADATA)
		{
			target_type 		= C_DMA_IO_CFC;
		}
		else if (t_addr == (INT32U)
		P_I2STX_DATA)
		{
			target_type 		= C_DMA_IO_I2S_TX;
		}
		else 
		{
			return - 5; // Unknow IO
		}
	}

	if (!dma_init_done)
	{
		dma_init();
	}

	target_addr 		= t_addr;
	channel 			= dma_get_channel(usage);
	target_addr 		= 0;

	if (channel >= C_DMA_CHANNEL_NUM)
	{
		return - 1; // No free DMA channel is available
	}

	dma_set_notify(channel, (INT32U) dma_struct->notify, os_q);

	// Set source address
	dma_set_source(channel, s_addr);

	// Set target address
	dma_set_target(channel, t_addr);

	// Set transmit counter
	if (dma_set_tx_count(channel, count))
	{
		dma_free_channel(channel);
		return - 1;
	}

	if (dma_set_timeout(channel, dma_struct->timeout))
	{
		dma_free_channel(channel);
		return - 1;
	}

	if (dma_set_sprite_size(channel, 0))
	{
		dma_free_channel(channel);
		return - 1;
	}

	// Prepare control register
	if (src_type != C_DMA_MEMORY)
	{ // IO to memory
		ctrl				|= C_DMA_CTRL_DEMAND_TRANS | C_DMA_CTRL_IO2M | C_DMA_CTRL_SRC_FIX | C_DMA_CTRL_EXTERNAL;

		if (!dma_struct->timeout)
		{
			dma_set_timeout(channel, C_DMA_MISC_TIMEOUT_MAX);
		}

		if (dma_set_device(channel, src_type))
		{
			dma_free_channel(channel);
			return - 1;
		}

#if (									defined _DRV_L1_CACHE) && (_DRV_L1_CACHE == 1)

		// Invalid target memory from cache
		cache_invalid_range(t_addr, len);
#endif

	}
	else if (target_type != C_DMA_MEMORY)
	{ // Memory to IO
		ctrl				|= C_DMA_CTRL_DEMAND_TRANS | C_DMA_CTRL_M2IO | C_DMA_CTRL_DEST_FIX | C_DMA_CTRL_EXTERNAL;

		if (!dma_struct->timeout)
		{
			dma_set_timeout(channel, C_DMA_MISC_TIMEOUT_MAX);
		}

		if (dma_set_device(channel, target_type))
		{
			dma_free_channel(channel);
			return - 1;
		}

#if (									defined _DRV_L1_CACHE) && (_DRV_L1_CACHE == 1)

		// Drain source memory from cache
		cache_drain_range(s_addr, len);
#endif

	}
	else 
	{ // Memory to memory
		ctrl				|=
			 C_DMA_CTRL_SINGLE_TRANS | C_DMA_CTRL_SRC_INCREASE | C_DMA_CTRL_DEST_INCREASE | C_DMA_CTRL_SOFTWARE;

#if (									defined _DRV_L1_CACHE) && (_DRV_L1_CACHE == 1)

		// Drain source memory and invalid target memory from cache
		cache_drain_range(s_addr, len);
		cache_invalid_range(t_addr, len);
#endif
	}

	// Check whether burst mode can be used
	if (usage == C_DMA_OCCUPIED)
	{
		ctrl				|= C_DMA_CTRL_SINGLE_ACCESS;
	}
	else if (! (count & 0x7))
	{
		ctrl				|= C_DMA_CTRL_BURST8_ACCESS;
	}
	else if (! (count & 0x3))
	{
		ctrl				|= C_DMA_CTRL_BURST4_ACCESS;
	}
	else 
	{
		ctrl				|= C_DMA_CTRL_SINGLE_ACCESS;
	}

	// Start DMA now
	dma_set_control(channel, ctrl);

	dma_struct->channel = channel;

	return 0;
}


INT32S dma_transfer(DMA_STRUCT * dma_struct)
{
	if (!dma_struct)
	{
		return - 1;
	}

	return dma_transfer_extend(dma_struct, C_DMA_NORMAL_USED, NULL);
}


INT32S dma_transfer_wait_ready(DMA_STRUCT * dma_struct)
{
	INT8S notify;

#if _OPERATING_SYSTEM				!= _OS_NONE
	INT32U q_index;
	INT32S mask;
	INT32S result;
	INT8U error;
#endif

	if (!dma_struct)
	{
		return - 1;
	}

	if (! (dma_struct->notify))
	{
		dma_struct->notify	= &notify;
	}

	* (dma_struct->notify) = C_DMA_STATUS_WAITING;

#if _OPERATING_SYSTEM				!= _OS_NONE
	q_index 			= dma_get_queue();

	if (q_index >= C_DMA_Q_NUM)
	{
		return - 1;
	}

	if (dma_transfer_extend(dma_struct, C_DMA_NORMAL_USED, (INT32U) dma_driver_queue[q_index]))
	{
		if (dma_struct->notify == &notify)
		{ // Restore dma_struct->notify
			dma_struct->notify	= NULL;
		}

		dma_free_queue(q_index);

		return - 1;
	}

	result				= (INT32S)
	OSQPend(dma_driver_queue[q_index], 0, &error);
	dma_free_queue(q_index);

	if (error == OS_NO_ERR)
	{
		if (dma_struct->notify == &notify)
		{ // Restore dma_struct->notify
			dma_struct->notify	= NULL;
		}

		if (result == C_DMA_STATUS_DONE)
		{
			return 0;
		}

		return - 1; // DMA timeout
	}

	// If we don't receive response from DMA, we have to reset DMA controller and free the channel by ourselves
	mask				= dma_device_protect();
	dma_free_channel((INT32U) dma_struct->channel);
	dma_device_unprotect(mask);

#else

	if (dma_transfer_extend(dma_struct, C_DMA_NORMAL_USED, NULL))
	{
		return - 1;
	}

#endif

	while (* ((volatile INT8S *) dma_struct->notify) == C_DMA_STATUS_WAITING)
		;

	if (* (dma_struct->notify) == C_DMA_STATUS_DONE)
	{
		if (dma_struct->notify == &notify)
		{ // Restore dma_struct->notify
			dma_struct->notify	= NULL;
		}

		return 0;
	}

	if (dma_struct->notify == &notify)
	{ // Restore dma_struct->notify
		dma_struct->notify	= NULL;
	}

	return - 1; // DMA timeout
}


#if _OPERATING_SYSTEM			!= _OS_NONE
INT32S dma_transfer_with_queue(DMA_STRUCT * dma_struct, OS_EVENT * os_q)
{
	if (!dma_struct || !os_q)
	{
		return - 1;
	}

	return dma_transfer_extend(dma_struct, C_DMA_NORMAL_USED, (INT32U) os_q);
}


INT32S dma_transfer_with_double_buf(DMA_STRUCT * dma_struct, OS_EVENT * os_q)
{
	if (!dma_struct || !os_q)
	{
		return - 1;
	}

	return dma_transfer_extend(dma_struct, C_DMA_OCCUPIED, (INT32U) os_q);
}


INT32S dma_transfer_double_buf_set(DMA_STRUCT * dma_struct)
{
	INT32S mask;

	if (!dma_struct || dma_struct->channel >= C_DMA_Q_NUM)
	{
		return - 1;
	}

	if (dma_usage[dma_struct->channel] != C_DMA_OCCUPIED)
	{
		return - 1;
	}

#if (								defined _DRV_L1_CACHE) && (_DRV_L1_CACHE == 1)
	cache_drain_range(dma_struct->s_addr, dma_struct->count * 2);
#endif

	dma_set_source((INT32U) dma_struct->channel, dma_struct->s_addr);
	dma_set_target((INT32U) dma_struct->channel, dma_struct->t_addr);

	if (dma_set_tx_count((INT32U) dma_struct->channel, dma_struct->count))
	{
		mask				= dma_device_protect();
		dma_free_channel((INT32U) dma_struct->channel);
		dma_device_unprotect(mask);

		return - 1;
	}

	return 0;
}


INT32S dma_dbf_status_get(INT8U channel)
{
	INT32U ctrl;

	if (channel == C_DMA_CH0)
	{
		ctrl				= R_DMA0_CTRL;
	}
	else if (channel == C_DMA_CH1)
	{
		ctrl				= R_DMA1_CTRL;
	}
	else if (channel == C_DMA_CH2)
	{
		ctrl				= R_DMA2_CTRL;
	}
	else if (channel == C_DMA_CH3)
	{
		ctrl				= R_DMA3_CTRL;
	}
	else if (channel == C_DMA_CH4)
	{
		ctrl				= R_DMA4_CTRL;
	}
	else if (channel == C_DMA_CH5)
	{
		ctrl				= R_DMA5_CTRL;
	}
	else if (channel == C_DMA_CH6)
	{
		ctrl				= R_DMA6_CTRL;
	}
	else if (channel == C_DMA_CH7)
	{
		ctrl				= R_DMA7_CTRL;
	}
	else 
	{
		ctrl				= 0;
	}

	if (ctrl & C_DMA_CTRL_DBF)
	{
		return 1;
	}

	return 0;
}


INT32S dma_status_get(INT8U channel)
{
	INT32U ctrl;

	if (channel == C_DMA_CH0)
	{
		ctrl				= R_DMA0_CTRL;
	}
	else if (channel == C_DMA_CH1)
	{
		ctrl				= R_DMA1_CTRL;
	}
	else if (channel == C_DMA_CH2)
	{
		ctrl				= R_DMA2_CTRL;
	}
	else if (channel == C_DMA_CH3)
	{
		ctrl				= R_DMA3_CTRL;
	}
	else if (channel == C_DMA_CH4)
	{
		ctrl				= R_DMA4_CTRL;
	}
	else if (channel == C_DMA_CH5)
	{
		ctrl				= R_DMA5_CTRL;
	}
	else if (channel == C_DMA_CH6)
	{
		ctrl				= R_DMA6_CTRL;
	}
	else if (channel == C_DMA_CH7)
	{
		ctrl				= R_DMA7_CTRL;
	}
	else 
	{
		ctrl				= 0;
	}

	if (ctrl & C_DMA_CTRL_BUSY)
	{
		return 1;
	}

	return 0;
}


INT32S dma_transfer_double_buf_free(DMA_STRUCT * dma_struct)
{
	INT32S mask;

	if (!dma_struct)
	{
		return - 1;
	}

	if (dma_usage[dma_struct->channel] != C_DMA_OCCUPIED)
	{
		return - 1;
	}

	mask				= dma_device_protect();
	dma_free_channel((INT32U) dma_struct->channel);
	dma_device_unprotect(mask);

	return 0;
}


#endif

INT32S dma_memory_fill(INT32U t_addr, INT8U value, INT32U byte_count)
{
	INT8U * p8;
	INT32U * p32;
	INT32U ctrl, channel;
	INT32U src_value;
	INT32S ret;

#if _OPERATING_SYSTEM				!= _OS_NONE
	INT32U q_index;
	INT32S result;
	INT32S mask;
	INT8U error;
#endif

	volatile INT8S notify;

	if (!dma_init_done)
	{
		dma_init();
	}

	// If length is less than 16 bytes, uses CPU to set the memory directly
	if (byte_count < 16)
	{
		p8					= (INT8U *)
		t_addr;

		while (byte_count--)
		{
			*p8++				= value;
		}

		return 0;
	}

	// Make sure start address is 4-byte alignment
	while (t_addr & 0x3)
	{
		* ((INT8U *) t_addr++) = value;
		byte_count--;
	}

	// Make sure end address is on 4-byte boundry
	while (byte_count & 0x3)
	{
		* ((INT8U *) t_addr + byte_count - 1) = value;
		byte_count--;
	}

	src_value			= (value << 24) | (value << 16) | (value << 8) | value;

	// If left length is less than 128 bytes, uses CPU to set the memory directly
	if (byte_count < 128)
	{
		p32 				= (INT32U *)
		t_addr;

		while (byte_count)
		{
			*p32++				= src_value;
			byte_count			-= 4;
		}

		return 0;
	}

	// Create a queue to receive DMA result
#if _OPERATING_SYSTEM				!= _OS_NONE
	q_index 			= dma_get_queue();

	if (q_index >= C_DMA_Q_NUM)
	{
		return - 1;
	}

#endif

	// Get a free DMA channel
	channel 			= dma_get_channel(C_DMA_NORMAL_USED);

	if (channel >= C_DMA_CHANNEL_NUM)
	{
#if _OPERATING_SYSTEM					!= _OS_NONE
		dma_free_queue(q_index);
#endif

		return - 1; // No free DMA channel is available
	}

	// Set transmit counter
	ret 				= dma_set_tx_count(channel, byte_count >> 2);

	// Disable timeout function
	if (dma_set_timeout(channel, 0))
	{
		ret 				= -1;
	}

	// Disable skip function
	if (dma_set_sprite_size(channel, 0))
	{
		ret 				= -1;
	}

	// Return if any error occurs
	if (ret)
	{
#if _OPERATING_SYSTEM					!= _OS_NONE
		dma_free_queue(q_index);
#endif

		dma_free_channel(channel);

		return - 1;
	}

	// Set DMA status to waiting transmit
	notify				= C_DMA_STATUS_WAITING;

#if _OPERATING_SYSTEM				!= _OS_NONE
	dma_set_notify(channel, (INT32U) &notify, (INT32U) dma_driver_queue[q_index]);

#else

	dma_set_notify(channel, (INT32U) &notify, NULL);
#endif

	// Set source address
	dma_set_source(channel, (INT32U) &src_value);

	// Set target address
	dma_set_target(channel, t_addr);

#if (								defined _DRV_L1_CACHE) && (_DRV_L1_CACHE == 1)

	// Drain source memory and invalid target memory from cache
	cache_drain_range((INT32U) &src_value, 4);
	cache_invalid_range(t_addr, byte_count);
#endif

	// Prepare control register
	if (! ((byte_count >> 2) & 0x7))
	{
		ctrl				=
			 C_DMA_CTRL_BURST8_ACCESS | C_DMA_CTRL_SINGLE_TRANS | C_DMA_CTRL_32BIT | C_DMA_CTRL_M2M | C_DMA_CTRL_INT | C_DMA_CTRL_SRC_FIX | C_DMA_CTRL_DEST_INCREASE | C_DMA_CTRL_NORMAL_INT | C_DMA_CTRL_SOFTWARE | C_DMA_CTRL_ENABLE;
	}
	else if (! ((byte_count >> 2) & 0x3))
	{
		ctrl				=
			 C_DMA_CTRL_BURST4_ACCESS | C_DMA_CTRL_SINGLE_TRANS | C_DMA_CTRL_32BIT | C_DMA_CTRL_M2M | C_DMA_CTRL_INT | C_DMA_CTRL_SRC_FIX | C_DMA_CTRL_DEST_INCREASE | C_DMA_CTRL_NORMAL_INT | C_DMA_CTRL_SOFTWARE | C_DMA_CTRL_ENABLE;
	}
	else 
	{
		ctrl				=
			 C_DMA_CTRL_SINGLE_ACCESS | C_DMA_CTRL_SINGLE_TRANS | C_DMA_CTRL_32BIT | C_DMA_CTRL_M2M | C_DMA_CTRL_INT | C_DMA_CTRL_SRC_FIX | C_DMA_CTRL_DEST_INCREASE | C_DMA_CTRL_NORMAL_INT | C_DMA_CTRL_SOFTWARE | C_DMA_CTRL_ENABLE;
	}

	// Start DMA now
	dma_set_control(channel, ctrl);

	// Wait until DMA finish transmitting or timeout
#if _OPERATING_SYSTEM				!= _OS_NONE
	result				= (INT32S)
	OSQPend(dma_driver_queue[q_index], 10 * OS_TICKS_PER_SEC, &error);
	dma_free_queue(q_index);

	if (error == OS_NO_ERR)
	{
		return 0; // DMA timeout function is not enabled, DMA transfer must be ok to reach here
	}

	// If we don't receive response from DMA, we have to reset DMA controller and free the channel by ourselves
	mask				= dma_device_protect();
	dma_free_channel(channel);
	dma_device_unprotect(mask);

	if (notify == C_DMA_STATUS_DONE)
	{
		return 0;
	}

	return - 1;

#else

	while (notify == C_DMA_STATUS_WAITING)
		;

	return 0; // DMA timeout function is not enabled, DMA transfer must be ok to reach here

#endif
}


INT32S dma_buffer_copy(INT32U s_addr, INT32U t_addr, INT32U byte_count, INT32U s_width, INT32U t_width)
{
	INT32U ctrl, channel;
	INT32S ret;

#if _OPERATING_SYSTEM				!= _OS_NONE
	INT32U q_index;
	INT32S result;
	INT32S mask;
	INT8U error;
#endif

	volatile INT8S notify;

	if (!dma_init_done)
	{
		dma_init();
	}

	// Make sure address and size are 2-byte alignment
	if ((s_addr & 0x1) || (t_addr & 0x1) || (byte_count & 0x1) || (s_width & 0x1) || (t_width & 0x1) ||
		 (s_width > t_width))
	{
		return - 1;
	}

	// Create a queue to receive DMA result
#if _OPERATING_SYSTEM				!= _OS_NONE
	q_index 			= dma_get_queue();

	if (q_index >= C_DMA_Q_NUM)
	{
		return - 1;
	}

#endif

	// Get a free DMA channel
	channel 			= dma_get_channel(C_DMA_NORMAL_USED);

	if (channel >= C_DMA_CHANNEL_NUM)
	{
#if _OPERATING_SYSTEM					!= _OS_NONE
		dma_free_queue(q_index);
#endif

		return - 1; // No free DMA channel is available
	}

	// Set transmit counter
	ret 				= dma_set_tx_count(channel, byte_count >> 1);

	// Disable timeout function
	if (dma_set_timeout(channel, 0))
	{
		ret 				= -1;
	}

	// Set destination buffer width (1 means 2 bytes)
	if (dma_set_line_length(t_width >> 1))
	{
		ret 				= -1;
	}

	// Set source buffer width (1 means 2 bytes)
	if (dma_set_sprite_size(channel, s_width >> 1))
	{
		ret 				= -1;
	}

	// Return if any error occurs
	if (ret)
	{
#if _OPERATING_SYSTEM					!= _OS_NONE
		dma_free_queue(q_index);
#endif

		dma_free_channel(channel);

		return - 1;
	}

	// Set DMA status to waiting transmit
	notify				= C_DMA_STATUS_WAITING;

#if _OPERATING_SYSTEM				!= _OS_NONE
	dma_set_notify(channel, (INT32U) &notify, (INT32U) dma_driver_queue[q_index]);

#else

	dma_set_notify(channel, (INT32U) &notify, NULL);
#endif

	// Set source address
	dma_set_source(channel, s_addr);

	// Set target address
	dma_set_target(channel, t_addr);

#if (								defined _DRV_L1_CACHE) && (_DRV_L1_CACHE == 1)

	// Drain source memory and invalid target memory from cache
	cache_drain_range(s_addr, byte_count);
	cache_invalid_range(t_addr, byte_count);
#endif

	// Prepare control register, burst is not supported in sprite copy mode
	ctrl				=
		 C_DMA_CTRL_SINGLE_ACCESS | C_DMA_CTRL_SINGLE_TRANS | C_DMA_CTRL_16BIT | C_DMA_CTRL_M2M | C_DMA_CTRL_INT | C_DMA_CTRL_SRC_INCREASE | C_DMA_CTRL_DEST_INCREASE | C_DMA_CTRL_NORMAL_INT | C_DMA_CTRL_SOFTWARE | C_DMA_CTRL_ENABLE;

	// Start DMA now
	dma_set_control(channel, ctrl);

	// Wait until DMA finish transmitting or timeout
#if _OPERATING_SYSTEM				!= _OS_NONE
	result				= (INT32S)
	OSQPend(dma_driver_queue[q_index], 5 * OS_TICKS_PER_SEC, &error);
	dma_free_queue(q_index);

	if (error == OS_NO_ERR)
	{
		return 0; // DMA timeout function is not enabled, DMA transfer must be ok to reach here
	}

	// If we don't receive response from DMA, we have to reset DMA controller and free the channel by ourselves
	mask				= dma_device_protect();
	dma_free_channel(channel);
	dma_device_unprotect(mask);

	if (notify == C_DMA_STATUS_DONE)
	{
		return 0;
	}

	return - 1;

#else

	while (notify == C_DMA_STATUS_WAITING)
		;

	return 0; // DMA timeout function is not enabled, DMA transfer must be ok to reach here

#endif
}


#endif

