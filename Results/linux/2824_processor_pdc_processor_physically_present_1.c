static bool __init processor_physically_present(acpi_handle handle)
{
	int cpuid, type;
	u32 acpi_id;
	acpi_status status;
	acpi_object_type acpi_type;
	unsigned long long tmp;
	union acpi_object object = { 0 };
	struct acpi_buffer buffer = { sizeof(union acpi_object), &object };

	status = acpi_get_type(handle, &acpi_type);
	if (ACPI_FAILURE(status))
		return false;

	switch (acpi_type) {
	case ACPI_TYPE_PROCESSOR:
		status = acpi_evaluate_object(handle, NULL, NULL, &buffer);
		if (ACPI_FAILURE(status))
			return false;
		acpi_id = object.processor.proc_id;
		break;
	case ACPI_TYPE_DEVICE:
		status = acpi_evaluate_integer(handle, "_UID", NULL, &tmp);
		if (ACPI_FAILURE(status))
			return false;
		acpi_id = tmp;
		break;
	default:
		return false;
	}

	type = (acpi_type == ACPI_TYPE_DEVICE) ? 1 : 0;
	cpuid = acpi_get_cpuid(handle, type, acpi_id);

	return !invalid_logical_cpuid(cpuid);
}


// Source: processor_pdc.c
// Lines 22-57
