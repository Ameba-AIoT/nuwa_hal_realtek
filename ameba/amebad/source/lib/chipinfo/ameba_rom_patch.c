/*
 * Copyright (c) 2024 Realtek Semiconductor Corp.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ameba_soc.h"
#include "ameba_rom_patch.h"

char num_buf_temp[10];


#define get_num_va_args(_args, _lcount)				\
	(((_lcount) > 1)  ? va_arg(_args, long long int) :	\
	(((_lcount) == 1) ? va_arg(_args, long int) :		\
			    va_arg(_args, int)))

#define get_unum_va_args(_args, _lcount)				\
	(((_lcount) > 1)  ? va_arg(_args, unsigned long long int) :	\
	(((_lcount) == 1) ? va_arg(_args, unsigned long int) :		\
			    va_arg(_args, unsigned int)))

#define get_char_upper_lower(_upper, _rem)				\
	(((_rem) < 0xa)   ?  ('0' + _rem) : 					\
	(((_upper) == 1)   ?  ('A' + (_rem - 0xa)) : 			\
	('a' + (_rem - 0xa))))

#define is_digit(c) ((c >= '0') && (c <= '9'))

static int print_string(const char *str)
{
	int count = 0;

	//assert_param(str != NULL);
	if (str == NULL) {
		return -1;
	}
	for (; *str != '\0'; str++) {
		DiagPutChar(*str);
		count++;
	}
	return count;
}

static int print_unsigned_num(unsigned long long int unum, unsigned int radix,
							  char padc, int padn, int upper, int sign)
{
	/* Just need enough space to store 64 bit decimal integer */
	char num_buf[20];
	int i = 0, count = 0;
	unsigned int rem;
	/* 1. convert num to str*/
	do {
		rem = unum % radix;
		num_buf[i] = get_char_upper_lower(upper, rem);
		i++;
		unum /= radix;
	} while (unum > 0U);
	/* 2. If there is a sign bit, subtract the padn by 1. */
	if (sign) {
		count++;
		padn--;
	}

	/* 3. If fill 0, need to print the sign first, and then fill in 0 */
	if (sign && (padc == '0')) {
		DiagPutChar('-');
	}
	/* 4. Fill the character padc */
	if (padn > 0) {
		while (i < padn) {
			DiagPutChar(padc);
			count++;
			padn--;
		}
	}
	/* 5.If fill a space, need to wait for step 4 to complete before filling the sign. */
	if (sign && (padc == ' ' || padc == '\0')) {
		DiagPutChar('-');
	}
	/* 6. Print the numbers*/
	while (--i >= 0) {
		DiagPutChar(num_buf[i]);
		count++;
	}

	return count;
}

static inline int pad_char_control(int padn, char padc)
{
	int count = 0;
	while (padn > 0) {
		DiagPutChar(padc);
		padn--;
		count++;
	}
	return count;
}

static inline int pad_num_control(unsigned long long int unum, unsigned int radix, char padc,
								  int padn, int upper, int sign, int pad_on_right)
{
	int count = 0;
	/* 1. if '-' left align was set, pad character in the right of num*/
	if (pad_on_right) {
		int width = print_unsigned_num(unum, radix, padc, 0, upper, sign);
		count += width;
		//if padc is equal to '\space' and pad number is great than width of num, execute pad opration. Otherwise, no operation.
		if ((padn > width) && (padc  == ' ')) {
			count += pad_char_control(padn - width, padc);
		}
		/* 2. Normally, pad operation is executed on the left of num.*/
	} else {
		count += print_unsigned_num(unum, radix, padc, padn, upper, sign);
	}
	return count;
}
/*******************************************************************
 * Reduced format print for Trusted firmware.
 * The following type specifiers are supported by this print
 * %lx - hexadecimal format
 * %s - string format
 * %lu or %i - signed decimal format
 * %u - unsigned decimal format
 * %p - pointer format
 *
 * The following length specifiers are supported by this print
 * %l - long int (64-bit on AArch64)
 * %ll - long long int (64-bit on AArch64)
 * %z - size_t sized integer formats (64 bit on AArch64)
 *
 * The following padding specifiers are supported by this print
 * %0NN - Left-pad the number with 0s (NN is a decimal number)
 * %NN - Left-pad the number with spaces
 * %-NN - Right-pad the number with spaces
 *
 * The print exits on all other formats specifiers other than valid
 * combinations of the above specifiers.
 *******************************************************************/
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
int DiagVprintf(const char *fmt, va_list args)
{
	int l_count;
	long long int num;
	unsigned long long int unum;
	char *str;

	int count = 0; 		/* Number of printed characters */
	int upper = 0;

	// pad control
	char padc = '\0'; 	/* Padding character */
	int padn; 			/* Number of characters to pad */
	int pad_on_right;
	int sign = 0;

	char c = '\0';
	int percent_flag = 0;
	char *tempstr = NULL;
	int tempcount = 0;

	while (*fmt != '\0') {
		l_count = 0;
		percent_flag = pad_on_right = padn = sign = 0;
		padc = '\0';

		if (*fmt == '%') {
			percent_flag = 1;
			fmt++;
			/* Check the format specifier */
loop:
			switch (*fmt) {
			case 'i': /* Fall through to next one */
			case 'd':
				num = get_num_va_args(args, l_count);
				if (num < 0) {
					sign = 1;
					unum = (unsigned long long int) - num;
				} else {
					unum = (unsigned long long int)num;
				}
				count += pad_num_control(unum, 10, padc, padn, upper, sign, pad_on_right);
				break;
			case 'u':
				unum = get_unum_va_args(args, l_count);
				count += pad_num_control(unum, 10, padc, padn, upper, sign, pad_on_right);
				break;
			case 'B':
			case 'b':
				unum = get_unum_va_args(args, l_count);
				count += pad_num_control(unum, 2, padc, padn, upper, sign, pad_on_right);
				break;
			case 'O':
			case 'o':
				unum = get_unum_va_args(args, l_count);
				count += pad_num_control(unum, 8, padc, padn, upper, sign, pad_on_right);
				break;
			case 'X':
				upper = 1;
			case 'x':
				unum = get_unum_va_args(args, l_count);
				count += pad_num_control(unum, 16, padc, padn, upper, sign, pad_on_right);
				break;
			case 'p':
				upper = 1;
				unum = (uintptr_t)va_arg(args, void *);
				if (sizeof(uintptr_t) == 4U) {
					padc = '0';
					padn = 8;
				} else {
					padc = '0';
					padn = 16;
				}
				count += pad_num_control(unum, 16, padc, padn, upper, sign, pad_on_right);
				break;
			case 's':
				str = va_arg(args, char *);
				tempstr = str;
				tempcount = 0;
				while (*tempstr != '\0') {
					tempstr++;
					tempcount++;
				}
				// left pad
				if (!pad_on_right && (padn - tempcount > 0)) {
					count += pad_char_control(padn - tempcount, padc);
				}
				// print string
				count += print_string(str);
				// right pad
				if (pad_on_right && (padn - tempcount > 0)) {
					count += pad_char_control(padn - tempcount, padc);
				}
				break;
			case 'c':
			case 'C':
				c = va_arg(args, int);
				// left pad
				if (!pad_on_right && (padn > 1)) {
					count += pad_char_control(padn - 1, padc);
				}
				DiagPutChar(c);
				count++;
				//right pad
				if (pad_on_right && (padn > 1)) {
					count += pad_char_control(padn - 1, padc);
				}
				break;
			case 'z':
				if (sizeof(size_t) == 8U) {
					goto not_support;
				}
				fmt++;
				goto loop;
			case 'L':
			case 'l':
				l_count++;
				fmt++;
				goto loop;
			case '-':
				fmt++;
				pad_on_right++;
				goto loop;
			/*% is followed by a %. */
			case '%':
				if (percent_flag) {
					DiagPutChar('%');
				}
				count++;
				break;
			case '0':
				padc = '0';
				padn = 0;
				fmt++;
				goto pad_count;
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				padc = ' ';
				padn = 0;
				goto pad_count;
			/*% is followed by a spaces, so padc = \space. */
			case ' ':
				padc = ' ';
				padn = 0;
				fmt++;
pad_count:
				for (;;) {
					char ch = *fmt;
					if (ch == '\0') { // deal with case '% \0'.
						goto exit;
					} else if (!is_digit(ch)) {
						goto loop;
					}
					padn = (padn * 10) + (ch - '0');
					fmt++;
				}
			case '\0':
				break;
			default:
not_support:
				/* Exit on any other format specifier */
				return -1;
			}
			fmt++;
			continue;
		}
		DiagPutChar(*fmt);
		fmt++;
		count++;
	}
exit:
	return count;
}

static inline int print_decimal_num(unsigned int unum, int radix, char padc, int padn, int sign)
{
	/* Just need enough space to store 31 bit decimal integer */
	char *num_buf = num_buf_temp;
	int i = 0, count = 0;
	unsigned int rem;
	/* 1. convert num to str*/
	do {
		rem = unum % radix;
		num_buf[i] = get_char_upper_lower(0, rem);
		i++;
		unum /= radix;
	} while (unum > 0U);

	/* 2. Fill the character padc */
	if (padn > 0) {
		while (i < padn) {
			DiagPutChar(padc);
			count++;
			padn--;
		}
	}

	if (sign) {
		DiagPutChar('-');
	}

	/* 3. Print the numbers*/
	while (--i >= 0) {
		DiagPutChar(num_buf[i]);
		count++;
	}

	return count;
}

int DiagVprintfNano(const char *fmt, va_list args)
{
	char *str;

	int count = 0; 		/* Number of printed characters */
	int sign = 0, num = 0;
	// pad control
	char padc = '\0'; 	/* Padding character */
	int padn; 			/* Number of characters to pad */
	int pad_on_right;

	char c = '\0';
	int percent_flag = 0;
	char *tempstr = NULL;
	int tempcount = 0;

	unsigned int unum;
	while (*fmt != '\0') {
		percent_flag = pad_on_right = padn = 0;
		padc = '\0';
		sign = 0;
		if (*fmt == '%') {
			percent_flag = 1;
			fmt++;
loop:
			/* Check the format specifier */
			switch (*fmt) {
			case 'i':
			case 'd':
				num = get_num_va_args(args, 0);
				if (num < 0) {
					sign = 1;
					unum = (unsigned int) - num;
				} else {
					unum = (unsigned int)num;
				}
				count += print_decimal_num(unum, 10, padc, padn, sign);
				break;
			case 'u':
				unum = (unsigned int)get_unum_va_args(args, 0);
				count += print_decimal_num(unum, 10, padc, padn, 0);
				break;
			case 'x':
				unum = (unsigned int)get_unum_va_args(args, 0);
				count += print_decimal_num(unum, 16, padc, padn, 0);
				break;
			case 's':
				str = va_arg(args, char *);
				tempstr = str;
				tempcount = 0;
				while (*tempstr != '\0') {
					tempstr++;
					tempcount++;
				}
				// left pad
				if (!pad_on_right && (padn - tempcount > 0)) {
					count += pad_char_control(padn - tempcount, padc);
				}
				// print string
				count += print_string(str);
				// right pad
				if (pad_on_right && (padn - tempcount > 0)) {
					count += pad_char_control(padn - tempcount, padc);
				}
				break;
			case 'c':
			case 'C':
				c = va_arg(args, int);
				// left pad
				if (!pad_on_right && (padn > 1)) {
					count += pad_char_control(padn - 1, padc);
				}
				DiagPutChar(c);
				count++;
				//right pad
				if (pad_on_right && (padn > 1)) {
					count += pad_char_control(padn - 1, padc);
				}
				break;
			/*% is followed by a %. */
			case '%':
				if (percent_flag) {
					DiagPutChar('%');
				}
				count++;
				break;
			case '0':
				padc = '0';
				padn = 0;
				fmt++;
				goto pad_count;
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				padc = ' ';
				padn = 0;
				goto pad_count;
			/*% is followed by a spaces, so padc = \space. */
			case ' ':
				padc = ' ';
				padn = 0;
				fmt++;
pad_count:
				for (;;) {
					char ch = *fmt;
					if (ch == '\0') { // deal with case '% \0'.
						return count;
					} else if (!is_digit(ch)) {
						goto loop;
					}
					padn = (padn * 10) + (ch - '0');
					fmt++;
				}
			default:
				/* Exit on any other format specifier */
				return -1;
			}
			fmt++;
			continue;
		}
		DiagPutChar(*fmt);
		fmt++;
		count++;
	}

	return count;
}


#pragma GCC diagnostic pop

u32 DiagPrintf(const char *fmt, ...)
{
	int ret;
	va_list va;

	va_start(va, fmt);
	ret = DiagVprintf(fmt, va);
	va_end(va);
	return ret;
}

/* support %s %c */
/* and num %d %x, format like %08.. is supported too, num must within 32bits */
u32 DiagPrintfNano(const char *fmt, ...)
{
	int ret;
	va_list va;

#ifdef CONFIG_ARM_CORE_CA32
	u32 isr_status = spin_lock_irqsave(&print_lock);
#endif

	va_start(va, fmt);
	ret = DiagVprintfNano(fmt, va);
	va_end(va);

#ifdef CONFIG_ARM_CORE_CA32
	spin_unlock_irqrestore(&print_lock, isr_status);
#endif
	return ret;
}
/* support %s %d %x %c, %08x.*/
int DiagSnPrintf(char *buf, size_t size, const char *fmt, ...)
{
	va_list     ap;
	char *p, *s, *buf_end = NULL;
	char padc = ' ';
	int padn = 0;

	if (buf == NULL) {
		return 0;
	}

	va_start(ap, fmt);
	s = buf;
	buf_end = size ? (buf + size) : (char *)~0;
	for (; *fmt != '\0'; ++fmt) {
		padn = 0;
		padc = ' ';
		if (*fmt != '%') {
			*s++ = *fmt;

			if (s >= buf_end) {
				goto Exit;
			}

			continue;
		}
		if (*++fmt == 's') {
			p = va_arg(ap, char *);
			while (*p != '\0') {
				*s++ = *p++;
				if (s >= buf_end) {
					goto Exit;
				}
			}
		} else {	/* Length of item is bounded */

			char tmp[20], *q = tmp;
			int shift = 0;// = 12;

			int lpforchk = 0;
			if ((*fmt == 'p') || (*fmt == 'P')) {
				lpforchk = (uintptr_t)va_arg(ap, void *);
				if (sizeof(uintptr_t) == 4U) {
					padc = '0';
					padn = 8;
				} else {
					padc = '0';
					padn = 16;
				}
			} else {
				lpforchk = va_arg(ap, int);
			}
			if ((lpforchk) < 0x10) {
				shift = 0;
			} else if (((lpforchk) >= 0x10) && ((lpforchk) < 0x100)) {
				shift = 4;
			} else if (((lpforchk) >= 0x100) && ((lpforchk) < 0x1000)) {
				shift = 8;
			} else if (((lpforchk) >= 0x1000) && ((lpforchk) < 0x10000)) {
				shift = 12;
			} else if (((lpforchk) >= 0x10000) && ((lpforchk) < 0x100000)) {
				shift = 16;
			} else if (((lpforchk) >= 0x100000) && ((lpforchk) < 0x1000000)) {
				shift = 20;
			} else if (((lpforchk) >= 0x1000000) && ((lpforchk) < 0x10000000)) {
				shift = 24;
			} else if ((lpforchk) >= 0x10000000) {
				shift = 28;
			} else {
				shift = 28;
			}

			if (*fmt == '0') {
				fmt++;
				padc = '0';
			}

			if ((*fmt  > '0') && (*fmt  <= '9')) {
				int width;
				unsigned char fch = *fmt;
				for (width = 0; (fch >= '0') && (fch <= '9'); fch = *++fmt) {
					width = width * 10 + fch - '0';
				}
				padn = width;
			}
			/*
			 * Before each format q points to tmp buffer
			 * After each format q points past end of item
			 * Only %p and %x can be filled with digits 0 or spaces.
			 */
			if ((*fmt == 'x') || (*fmt == 'X') || (*fmt == 'p') || (*fmt == 'P')) {
				/* With x86 gcc, sizeof(long) == sizeof(int) */
				const int lp = lpforchk;
				int h = lp;
				int hex_count = 0;
				unsigned int h_back = h;
				int ncase = (*fmt & 0x20);

				if ((*fmt == 'p') || (*fmt == 'P')) {
					*q++ = '0';
					*q++ = 'X' | ncase;
				}

				while (h_back) {
					hex_count += 1;
					h_back  = h_back >> 4;
				}
				while (padn-- > hex_count) {
					*q++ = padc;
				}

				for (; shift >= 0; shift -= 4) {
					*q++ = "0123456789ABCDEF"[(h >> shift) & 0xF] | ncase;
				}
			} else if (*fmt == 'd') {
				int i = lpforchk;
				char *r;
				int digit_space = 0;

				if (i < 0) {
					*q++ = '-';
					i = -i;
					digit_space++;
				}

				p = q;		/* save beginning of digits */

				do {
					*q++ = '0' + (i % 10);
					i /= 10;
					digit_space++;
				} while (i);


				for (; shift >= 0; shift -= 4) {

					if (digit_space-- > 0) {
						; //do nothing
					} else {
						*q++ = '0';
					}
				}

				/* reverse digits, stop in middle */
				r = q;		/* don't alter q */

				while (--r > p) {
					i = *r;
					*r = *p;
					*p++ = i;
				}
			} else if (*fmt == 'c') {
				*q++ = lpforchk;
			} else {
				*q++ = *fmt;
			}

			/* now output the saved string */
			for (p = tmp; p < q; ++p) {
				*s++ = *p;

				if (s >= buf_end) {
					goto Exit;
				}
			}
		}
	}
Exit:

	if (buf) {
		*s = '\0';
	}
	va_end(ap);
	return (s - buf);
}

_LONG_CALL_ extern int _EFUSE_LogicalMap_Read(u8 *pbuf);
/**
  * @brief  read EFUSE logical map, patch return value for _EFUSE_LogicalMap_Read
  * @param  pbuf: EFUSE_MAP_LEN len buffer used for EFUSE Logical map
  * @retval status value:
  *          - RTK_SUCCESS: read ok
  *          - RTK_FAIL: read fail
  * @note merge from rtw_efuse_map_read
  */
int EFUSE_LogicalMap_Read(u8 *pbuf)
{
	u32 ret = _EFUSE_LogicalMap_Read(pbuf);

	if (ret == 1) {
		return RTK_SUCCESS;
	} else {
		return RTK_FAIL;
	}
}

_LONG_CALL_ extern int _EFUSE_LogicalMap_Write(u32 addr, u32 cnts, u8 *data);
/**
  * @brief  write EFUSE logical address, patch return value for _EFUSE_LogicalMap_Write
  * @param  addr: logical addr
  * @param  cnts: byte number, should be even
  * @param  data: data buffer to be write
  * @retval status value:
  *          - RTK_SUCCESS: write ok
  *          - RTK_FAIL: write fail
  * @note merge from rtw_efuse_map_write
  */
int EFUSE_LogicalMap_Write(u32 addr, u32 cnts, u8 *data)
{
	u32 ret = _EFUSE_LogicalMap_Write(addr, cnts, data);

	if (ret == 1) {
		return RTK_SUCCESS;
	} else {
		return RTK_FAIL;
	}
}

int OTP_LogicalMap_Read(u8 *pbuf, u32 addr, u32 len)
{
	int ret = EFUSE_LogicalMap_Read(pbuf);

	if (ret != RTK_SUCCESS) {
		return ret;
	}

	for (u32 i = 0; i < len && addr < OTP_LMAP_LEN; i++) {
		pbuf[i] = pbuf[addr++];
	}

	return RTK_SUCCESS;
}

int OTP_LogicalMap_Write(u32 addr, u32 cnts, u8 *data)
{
	return EFUSE_LogicalMap_Write(addr, cnts, data);
}

int OTP_Read8(u32 Addr, u8 *Data)
{
	u32 ret = EFUSERead8(0,  Addr, Data, L25EOUTVOLTAGE);
	if (ret == 1) {
		return RTK_SUCCESS;
	} else {
		return RTK_FAIL;
	}
}

int OTP_Write8(u32 Addr, u8 Data)
{
	u32 ret = EFUSEWrite8(0,  Addr, Data, L25EOUTVOLTAGE);
	if (ret == 1) {
		return RTK_SUCCESS;
	} else {
		return RTK_FAIL;
	}
}

int OTPSetCRC(void)
{
	RTK_LOGS(NOTAG, RTK_LOG_ERROR, "Not Support.\n");

	return RTK_SUCCESS;
}

u32 SYSCFG_OTP_RSIPEn(void)
{
	return HAL_READ32(SYSTEM_CTRL_BASE_LP, REG_SYS_EFUSE_SYSCFG3) & BIT_SYS_FLASH_ENCRYPT_EN;
}
/**
  * @brief  Reads the specified GPIO pins.
  * @param  GPIO_Port: Specifies port number
  *          This parameter can be one of the following values:
  *		 		@arg GPIO_PORT_A: port number is A
  *		 		@arg GPIO_PORT_B: port number is B
  * @param  GPIO_Mask: One bit one GPIO pin, PortA/B:0x00000000 ~0xFFFFFFFF.
  * @note   It can config multiple pins of one port by parameter GPIO_Mask
  * @retval The value of the specified port pins
  */
u32 GPIO_PortRead(u32 GPIO_Port, u32 GPIO_Mask)
{
	GPIO_TypeDef *GPIO = NULL;
	u32 RegValue;

	GPIO = PORT_AB[GPIO_Port];

	RegValue = GPIO->EXT_PORT[0];

	RegValue &= GPIO_Mask;

	return RegValue;
}


static GPIO_TypeDef *GPIO_PortAddrGet(u32 GPIO_Port)
{
	GPIO_TypeDef *GPIO = NULL;

	GPIO = PORT_AB[GPIO_Port];

	return GPIO;
}

u32 GPIO_INTStatusGet(u32 GPIO_Port)
{
	GPIO_TypeDef *GPIO = GPIO_PortAddrGet(GPIO_Port);

	return GPIO->INT_STATUS;
}

void GPIO_INTStatusClearEdge(u32 GPIO_Port)
{
	GPIO_TypeDef *GPIO = NULL;
	u32 IntStatus;

	GPIO = GPIO_PortAddrGet(GPIO_Port);

	IntStatus = GPIO->INT_STATUS;

	/* Clear pending edge interrupt */
	GPIO->PORTA_EOI = IntStatus;
}


void SYSCFG_CHIPType_Set(void)
{
#ifdef CONFIG_FPGA
	/* for simulation set ASIC */
	if (BKUP_Read(BKUP_REG0) & BIT_SW_SIM_RSVD) {
		IS_FPGA_VERIF = FALSE;
	} else {
		IS_FPGA_VERIF = TRUE;
	}
#endif
}

SRAMDRAM_ONLY_TEXT_SECTION
void FLASH_TxData(u32 StartAddr, u32 DataPhaseLen, u8 *pData)
{
	SPIC_TypeDef *spi_flash = SPIC;
	u32 temp, tx_num = 0;
	u32 ctrl0 = spi_flash->ctrlr0;
	u8 addrbyte[4];
	u32 *aligned_buf;

	assert_param(DataPhaseLen <= 256); /* max len is 12 */
	assert_param(((StartAddr & 0xFF) + DataPhaseLen) <= 0x100); /* addr should page(256) align */

	/* write enable cmd */
	FLASH_WriteEn();

	/* Disable SPI_FLASH User Mode */
	spi_flash->ssienr = 0;

	/* set ctrlr0: TX mode and one bit mode */
	spi_flash->ctrlr0 &= ~(BIT_TMOD(3) | BIT_CMD_CH(3) | BIT_ADDR_CH(3) | BIT_DATA_CH(3));
	spi_flash->addr_length = flash_init_para.FLASH_addr_phase_len;

	/* set flash_cmd: write cmd & address to fifo & addr is MSB */
	addrbyte[3] = (StartAddr & 0xFF000000) >> 24;
	addrbyte[2] = (StartAddr & 0xFF0000) >> 16;
	addrbyte[1] = (StartAddr & 0xFF00) >> 8;
	addrbyte[0] = StartAddr & 0xFF;

	if (flash_init_para.FLASH_addr_phase_len == ADDR_3_BYTE) {
		spi_flash->dr[0].word = FLASH_CMD_PP | (addrbyte[2] << 8) | (addrbyte[1] << 16) | (addrbyte[0] << 24);
	} else if (flash_init_para.FLASH_addr_phase_len == ADDR_4_BYTE) {
		spi_flash->dr[0].byte = FLASH_CMD_PP;
		spi_flash->dr[0].word = (addrbyte[3]) | (addrbyte[2] << 8) | (addrbyte[1] << 16) | (addrbyte[0] << 24);
	}

	FLASH_SW_CS_Control(0);
	/* Enable SPI_FLASH  User Mode */
	spi_flash->ssienr = 1;

	/* 1. SPIC will pull high SPI CS when FIFO empty, this will cause FLASH reset write Enable and cause write fail  */
	/* 2. so we use GPIO to control CS, and pull CS low when we write DR  */
	/* 3. we should set  ssienr = 1 before write DR, and SPIC will send data after we write */
	/* 4. SPIC will hold system bus when SPI Tx, So we need not check FIFO Full */
	tx_num = 0;

	/* write some address non-aligned data in byte */
	temp = UNALIGNED32(pData);
	if (temp) {
		while (tx_num < DataPhaseLen) {
			spi_flash->dr[0].byte = pData[tx_num];
			tx_num += 1;
			if (tx_num == U32BLOCKSIZE - temp) {
				break;
			}
		}
	}

	/* write the address aligned data in word */
	aligned_buf = (u32 *)&pData[tx_num];
	while (DataPhaseLen - tx_num >= U32BLOCKSIZE) {
		spi_flash->dr[0].word = *aligned_buf;
		aligned_buf++;
		tx_num += U32BLOCKSIZE;
	}

	while (tx_num < DataPhaseLen) {
		spi_flash->dr[0].byte = pData[tx_num];
		tx_num += 1;
	}

	FLASH_SW_CS_Control(1);

	/* wait spic busy done */
	FLASH_WaitBusy(WAIT_SPIC_BUSY);

	/* Disable SPI_FLASH User Mode */
	spi_flash->ssienr = 0;

	/* polling WEL and busy done */
	FLASH_WaitBusy(WAIT_WRITE_DONE);

	/* backup bitmode */
	spi_flash->ctrlr0 = ctrl0;
}

SRAMDRAM_ONLY_TEXT_SECTION
void FLASH_Addr4ByteEn(void)
{
	FLASH_WriteEn();
	FLASH_TxCmd(FLASH_CMD_ENT_ADDR4B, 0, NULL);
	FLASH_WaitBusy(WAIT_WRITE_DONE);
}

__NO_RETURN
void io_assert_failed(uint8_t *file, uint32_t line)
{
	RTK_LOGS(NOTAG, RTK_LOG_ERROR, "io driver parameters error! file_name: %s, line: %d\n", (char *)file, line);

	for (;;);
}

/**
* @brief  Check if there is space in loguart tx path fifo.
* @retval Status value:
*          - 1: tx path fifo is not full
*          - 0: tx path fifo is full
*/
u32 LOGUART_Writable(void)
{
	return UART_Writable(UART2_DEV);
}

u32 LOGUART_GetStatus(UART_TypeDef *UARTLOG)
{
	return UART_LineStatusGet(UARTLOG);
}

void LOGUART_INTClear(UART_TypeDef *UARTLOG, u32 UART_IT)
{
	UART_INT_Clear(UARTLOG, UART_IT);
}

void LOGUART_INTConfig(UART_TypeDef *UARTLOG, u32 UART_IT, u32 newState)
{
	UART_INTConfig(UARTLOG, UART_IT, newState);
}

/**
 * @brief Get the Line status register value.
 * @param UARTx UART device, where x can be 0~3.
 * @return The Line status register value.
 */
u32 UART_LineStatusGet(UART_TypeDef *UARTx)
{
	volatile u8 reg_iir;
	u8 IntId;
	u32 IntStatus = 0;
	u32 UartLSR = UARTx->LSR; // read only once

	if (UartLSR & RUART_LINE_STATUS_REG_DR) {
		IntStatus |= RUART_BIT_DRDY;
	}

	if (UartLSR & RUART_LINE_STATUS_REG_THRE) {
		IntStatus |= RUART_BIT_TX_NOT_FULL;
	}

	reg_iir = UART_IntStatus(UARTx);
	if ((reg_iir & RUART_IIR_INT_PEND) != 0) {
		// No pending IRQ
		return IntStatus;
	}

	IntId = (reg_iir & RUART_IIR_INT_ID) >> 1;

	switch (IntId) {
	case RUART_RECEIVE_LINE_STATUS:
		IntStatus |= (UartLSR & RUART_LINE_STATUS_ERR);
		break;
	case RUART_RECEIVER_DATA_AVAILABLE:
		IntStatus |= RUART_BIT_RXFIFO_INT;
		break;
	case RUART_TIME_OUT_INDICATION:
		IntStatus |= RUART_BIT_TIMEOUT_INT;
		break;
	case RUART_TX_FIFO_EMPTY:
		IntStatus |= RUART_BIT_TX_EMPTY;
		break;
	case RUART_MODEM_STATUS:
		IntStatus |= RUART_BIT_MODEM_INT;
		break;
	case RUART_LP_RX_MONITOR_DONE:
		IntStatus |= RUART_BIT_MONITOR_DONE_INT;
		break;
	default:
		// RTK_LOGS(NOTAG, RTK_LOG_ERROR, "Unknown Interrupt \n");
		break;
	}

	return IntStatus;
}

/**
 * @brief Clear specified UARTx interrupt status.
 * @param UARTx UART device, where x can be 0~3.
 * @param UART_IT Specified UARTx interrupt sources to be clear.
 * @return None
 */
void UART_INT_Clear(UART_TypeDef *UARTx, u32 UART_IT)
{
	switch (UART_IT) {
	case RUART_BIT_MDICF:
		(void)UART_RxMonitorSatusGet(UARTx);
		break;

	case RUART_BIT_MICF:
		(void)UART_ModemStatusGet(UARTx);
		break;

	case RUART_BIT_RLSICF:
		(void)UARTx->LSR;
		break;

	case RUART_BIT_TOICF:
		(void)UARTx->RB_THR; // read RBR to clear interrupt
	default:
		// RTK_LOGS(NOTAG, RTK_LOG_ERROR, "Unknown Interrupt \n");
		break;
	}
}

char *__wrap_strncpy(char *dst0, const char *src0, int count)
{
	return __real_strncpy(dst0, src0, count);
}

char *__wrap_strcpy(char *s1, const char *s2)
{
	return __real_strcpy(s1, s2);
}

long __wrap_strtol(const char *nptr, char **endptr, int base)
{
	return __real_strtol(nptr, endptr, base);
}

/**
  * @brief  Check whether the APB peripheral's clock has been enabled or not
  * @param  APBPeriph_Clock_in: specifies the APB peripheral to check.
  *      This parameter can be one of @ref APBPeriph_UART0_CLOCK, APBPeriph_ATIM_CLOCK and etc.
  * @retval TRUE: The APB peripheral's clock has been enabled
  * 		FALSE: The APB peripheral's clock has not been enabled
  */
u8 RCC_PeriphClockEnableChk(u32 APBPeriph_Clock_in)
{
	u8 ret;
	u32 TempVal, CkeRegOffset = 0;
	u32 ClkRegIndx = (APBPeriph_Clock_in >> 30) & 0x03;
	u32 APBPeriph_Clock = APBPeriph_Clock_in & (~(BIT(31) | BIT(30)));
	assert_param((ClkRegIndx == 0x1) || (ClkRegIndx == 0x2) || (ClkRegIndx == 0x3));

	switch (ClkRegIndx) {
	case SYS_CLK_CTRL1:
		CkeRegOffset = REG_HS_PERI_CLK_CTRL1;
		break;
	case SYS_CLK_CTRL2:
		CkeRegOffset = REG_HS_PERI_CLK_CTRL2;
		break;
	case SYS_CLK_CTRL3:
		CkeRegOffset = REG_HS_PERI_CLK_CTRL3;
		break;
	}

	TempVal = HAL_READ32(SYSTEM_CTRL_BASE, CkeRegOffset);
	if (TempVal & APBPeriph_Clock) {
		ret = TRUE;
	} else {
		ret = FALSE;
	}

	return ret;
}
