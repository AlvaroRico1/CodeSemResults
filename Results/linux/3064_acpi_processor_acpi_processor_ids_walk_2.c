static acpi_status __init acpi_processor_ids_walk(acpi_handle handle,
						  u32 lvl,
						  void *context,
						  void **rv)
{
	acpi_status status;
	acpi_object_type acpi_type;
	unsigned long long uid;
	union acpi_object object = { 0 };
	struct acpi_buffer buffer = { sizeof(union acpi_object), &object };

	status = acpi_get_type(handle, &acpi_type);
	if (ACPI_FAILURE(status))
		return status;

	switch (acpi_type) {
	case ACPI_TYPE_PROCESSOR:
		status = acpi_evaluate_object(handle, NULL, NULL, &buffer);
		if (ACPI_FAILURE(status))
			goto err;
		uid = object.processor.proc_id;
		break;

	case ACPI_TYPE_DEVICE:
		status = acpi_evaluate_integer(handle, "_UID", NULL, &uid);
		if (ACPI_FAILURE(status))
			goto err;
		break;
	default:
		goto err;
	}

	processor_validated_ids_update(uid);
	return AE_OK;

err:
	/* Exit on error, but don't abort the namespace walk */
	acpi_handle_info(handle, "Invalid processor object\n");
	return AE_OK;

}


// Source: acpi_processor.c
// Lines 634-674
