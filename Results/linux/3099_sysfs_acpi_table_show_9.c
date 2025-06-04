static ssize_t acpi_table_show(struct file *filp, struct kobject *kobj,
			       struct bin_attribute *bin_attr, char *buf,
			       loff_t offset, size_t count)
{
	struct acpi_table_attr *table_attr =
	    container_of(bin_attr, struct acpi_table_attr, attr);
	struct acpi_table_header *table_header = NULL;
	acpi_status status;
	ssize_t rc;

	status = acpi_get_table(table_attr->name, table_attr->instance,
				&table_header);
	if (ACPI_FAILURE(status))
		return -ENODEV;

	rc = memory_read_from_buffer(buf, count, &offset, table_header,
			table_header->length);
	acpi_put_table(table_header);
	return rc;
}


// Source: sysfs.c
// Lines 341-360
