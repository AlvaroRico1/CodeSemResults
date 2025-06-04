int acpi_pci_link_allocate_irq(acpi_handle handle, int index, int *triggering,
			       int *polarity, char **name)
{
	int result;
	struct acpi_device *device;
	struct acpi_pci_link *link;

	result = acpi_bus_get_device(handle, &device);
	if (result) {
		printk(KERN_ERR PREFIX "Invalid link device\n");
		return -1;
	}

	link = acpi_driver_data(device);
	if (!link) {
		printk(KERN_ERR PREFIX "Invalid link context\n");
		return -1;
	}

	/* TBD: Support multiple index (IRQ) entries per Link Device */
	if (index) {
		printk(KERN_ERR PREFIX "Invalid index %d\n", index);
		return -1;
	}

	mutex_lock(&acpi_link_lock);
	if (acpi_pci_link_allocate(link)) {
		mutex_unlock(&acpi_link_lock);
		return -1;
	}

	if (!link->irq.active) {
		mutex_unlock(&acpi_link_lock);
		printk(KERN_ERR PREFIX "Link active IRQ is 0!\n");
		return -1;
	}
	link->refcnt++;
	mutex_unlock(&acpi_link_lock);

	if (triggering)
		*triggering = link->irq.triggering;
	if (polarity)
		*polarity = link->irq.polarity;
	if (name)
		*name = acpi_device_bid(link->device);
	ACPI_DEBUG_PRINT((ACPI_DB_INFO,
			  "Link %s is referenced\n",
			  acpi_device_bid(link->device)));
	return (link->irq.active);
}


// Source: pci_link.c
// Lines 616-665
