static bool acpi_ec_ecdt_get_handle(acpi_handle *phandle)
{
	struct acpi_table_ecdt *ecdt_ptr;
	acpi_status status;
	acpi_handle handle;

	status = acpi_get_table(ACPI_SIG_ECDT, 1,
				(struct acpi_table_header **)&ecdt_ptr);
	if (ACPI_FAILURE(status))
		return false;

	status = acpi_get_handle(NULL, ecdt_ptr->id, &handle);
	if (ACPI_FAILURE(status))
		return false;

	*phandle = handle;
	return true;
}


// Source: ec.c
// Lines 1556-1573
