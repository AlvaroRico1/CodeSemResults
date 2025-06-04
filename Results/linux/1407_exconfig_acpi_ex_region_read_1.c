acpi_ex_region_read(union acpi_operand_object *obj_desc, u32 length, u8 *buffer)
{
	acpi_status status;
	u64 value;
	u32 region_offset = 0;
	u32 i;

	/* Bytewise reads */

	for (i = 0; i < length; i++) {
		status =
		    acpi_ev_address_space_dispatch(obj_desc, NULL, ACPI_READ,
						   region_offset, 8, &value);
		if (ACPI_FAILURE(status)) {
			return (status);
		}

		*buffer = (u8)value;
		buffer++;
		region_offset++;
	}

	return (AE_OK);
}


// Source: exconfig.c
// Lines 221-244
