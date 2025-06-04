void ntlm_memzero(void *data, size_t size)
{
	volatile uint8_t *scan = (volatile uint8_t *)data;

	while (size--)
		*scan++ = 0x0;
}


// Source: util.c
// Lines 16-22
