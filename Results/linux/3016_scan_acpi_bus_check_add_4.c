static acpi_status acpi_bus_check_add(acpi_handle handle, u32 lvl_not_used,
				      void *not_used, void **return_value)
{
	struct acpi_device *device = NULL;
	int type;
	unsigned long long sta;
	int result;

	acpi_bus_get_device(handle, &device);
	if (device)
		goto out;

	result = acpi_bus_type_and_status(handle, &type, &sta);
	if (result)
		return AE_OK;

	if (type == ACPI_BUS_TYPE_POWER) {
		acpi_add_power_resource(handle);
		return AE_OK;
	}

	acpi_add_single_object(&device, handle, type, sta);
	if (!device)
		return AE_CTRL_DEPTH;

	acpi_scan_init_hotplug(device);
	acpi_device_dep_initialize(device);

 out:
	if (!*return_value)
		*return_value = device;

	return AE_OK;
}


// Source: scan.c
// Lines 1852-1885
