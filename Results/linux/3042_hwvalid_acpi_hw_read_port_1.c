acpi_status acpi_hw_read_port(acpi_io_address address, u32 *value, u32 width)
{
	acpi_status status;
	u32 one_byte;
	u32 i;

	/* Truncate address to 16 bits if requested */

	if (acpi_gbl_truncate_io_addresses) {
		address &= ACPI_UINT16_MAX;
	}

	/* Validate the entire request and perform the I/O */

	status = acpi_hw_validate_io_request(address, width);
	if (ACPI_SUCCESS(status)) {
		status = acpi_os_read_port(address, value, width);
		return (status);
	}

	if (status != AE_AML_ILLEGAL_ADDRESS) {
		return (status);
	}

	/*
	 * There has been a protection violation within the request. Fall
	 * back to byte granularity port I/O and ignore the failing bytes.
	 * This provides compatibility with other ACPI implementations.
	 */
	for (i = 0, *value = 0; i < width; i += 8) {

		/* Validate and read one byte */

		if (acpi_hw_validate_io_request(address, 8) == AE_OK) {
			status = acpi_os_read_port(address, &one_byte, 8);
			if (ACPI_FAILURE(status)) {
				return (status);
			}

			*value |= (one_byte << i);
		}

		address++;
	}

	return (AE_OK);
}


// Source: hwvalid.c
// Lines 186-232
