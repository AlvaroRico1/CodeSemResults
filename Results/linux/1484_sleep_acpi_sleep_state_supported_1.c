bool acpi_sleep_state_supported(u8 sleep_state)
{
	acpi_status status;
	u8 type_a, type_b;

	status = acpi_get_sleep_type_data(sleep_state, &type_a, &type_b);
	return ACPI_SUCCESS(status) && (!acpi_gbl_reduced_hardware
		|| (acpi_gbl_FADT.sleep_control.address
			&& acpi_gbl_FADT.sleep_status.address));
}


// Source: sleep.c
// Lines 80-89
