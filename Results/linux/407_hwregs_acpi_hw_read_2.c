acpi_status acpi_hw_read(u64 *value, struct acpi_generic_address *reg)
{
	u64 address;
	u8 access_width;
	u32 bit_width;
	u8 bit_offset;
	u64 value64;
	u32 value32;
	u8 index;
	acpi_status status;

	ACPI_FUNCTION_NAME(hw_read);

	/* Validate contents of the GAS register */

	status = acpi_hw_validate_register(reg, 64, &address);
	if (ACPI_FAILURE(status)) {
		return (status);
	}

	/*
	 * Initialize entire 64-bit return value to zero, convert access_width
	 * into number of bits based
	 */
	*value = 0;
	access_width = acpi_hw_get_access_bit_width(address, reg, 64);
	bit_width = reg->bit_offset + reg->bit_width;
	bit_offset = reg->bit_offset;

	/*
	 * Two address spaces supported: Memory or IO. PCI_Config is
	 * not supported here because the GAS structure is insufficient
	 */
	index = 0;
	while (bit_width) {
		if (bit_offset >= access_width) {
			value64 = 0;
			bit_offset -= access_width;
		} else {
			if (reg->space_id == ACPI_ADR_SPACE_SYSTEM_MEMORY) {
				status =
				    acpi_os_read_memory((acpi_physical_address)
							address +
							index *
							ACPI_DIV_8
							(access_width),
							&value64, access_width);
			} else {	/* ACPI_ADR_SPACE_SYSTEM_IO, validated earlier */

				status = acpi_hw_read_port((acpi_io_address)
							   address +
							   index *
							   ACPI_DIV_8
							   (access_width),
							   &value32,
							   access_width);
				value64 = (u64)value32;
			}
		}

		/*
		 * Use offset style bit writes because "Index * AccessWidth" is
		 * ensured to be less than 64-bits by acpi_hw_validate_register().
		 */
		ACPI_SET_BITS(value, index * access_width,
			      ACPI_MASK_BITS_ABOVE_64(access_width), value64);

		bit_width -=
		    bit_width > access_width ? access_width : bit_width;
		index++;
	}

	ACPI_DEBUG_PRINT((ACPI_DB_IO,
			  "Read:  %8.8X%8.8X width %2d from %8.8X%8.8X (%s)\n",
			  ACPI_FORMAT_UINT64(*value), access_width,
			  ACPI_FORMAT_UINT64(address),
			  acpi_ut_get_region_name(reg->space_id)));

	return (status);
}


// Source: hwregs.c
// Lines 195-274
