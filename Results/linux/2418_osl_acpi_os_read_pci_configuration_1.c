acpi_os_read_pci_configuration(struct acpi_pci_id * pci_id, u32 reg,
			       u64 *value, u32 width)
{
	int result, size;
	u32 value32;

	if (!value)
		return AE_BAD_PARAMETER;

	switch (width) {
	case 8:
		size = 1;
		break;
	case 16:
		size = 2;
		break;
	case 32:
		size = 4;
		break;
	default:
		return AE_ERROR;
	}

	result = raw_pci_read(pci_id->segment, pci_id->bus,
				PCI_DEVFN(pci_id->device, pci_id->function),
				reg, size, &value32);
	*value = value32;

	return (result ? AE_ERROR : AE_OK);
}


// Source: osl.c
// Lines 760-789
