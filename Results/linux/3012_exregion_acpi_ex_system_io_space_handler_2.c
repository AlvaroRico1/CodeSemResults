acpi_ex_system_io_space_handler(u32 function,
				acpi_physical_address address,
				u32 bit_width,
				u64 *value,
				void *handler_context, void *region_context)
{
	acpi_status status = AE_OK;
	u32 value32;

	ACPI_FUNCTION_TRACE(ex_system_io_space_handler);

	ACPI_DEBUG_PRINT((ACPI_DB_INFO,
			  "System-IO (width %u) R/W %u Address=%8.8X%8.8X\n",
			  bit_width, function, ACPI_FORMAT_UINT64(address)));

	/* Decode the function parameter */

	switch (function) {
	case ACPI_READ:

		status = acpi_hw_read_port((acpi_io_address)address,
					   &value32, bit_width);
		*value = value32;
		break;

	case ACPI_WRITE:

		status = acpi_hw_write_port((acpi_io_address)address,
					    (u32)*value, bit_width);
		break;

	default:

		status = AE_BAD_PARAMETER;
		break;
	}

	return_ACPI_STATUS(status);
}


// Source: exregion.c
// Lines 274-312
