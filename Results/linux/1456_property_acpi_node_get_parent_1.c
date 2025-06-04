struct fwnode_handle *acpi_node_get_parent(const struct fwnode_handle *fwnode)
{
	if (is_acpi_data_node(fwnode)) {
		/* All data nodes have parent pointer so just return that */
		return to_acpi_data_node(fwnode)->parent;
	} else if (is_acpi_device_node(fwnode)) {
		acpi_handle handle, parent_handle;

		handle = to_acpi_device_node(fwnode)->handle;
		if (ACPI_SUCCESS(acpi_get_parent(handle, &parent_handle))) {
			struct acpi_device *adev;

			if (!acpi_bus_get_device(parent_handle, &adev))
				return acpi_fwnode_handle(adev);
		}


// Source: property.c
// Lines 1086-1100
