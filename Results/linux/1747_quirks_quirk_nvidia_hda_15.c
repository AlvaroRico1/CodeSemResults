static void quirk_nvidia_hda(struct pci_dev *gpu)
{
	u8 hdr_type;
	u32 val;

	/* There was no integrated HDA controller before MCP89 */
	if (gpu->device < PCI_DEVICE_ID_NVIDIA_GEFORCE_320M)
		return;

	/* Bit 25 at offset 0x488 enables the HDA controller */
	pci_read_config_dword(gpu, 0x488, &val);
	if (val & BIT(25))
		return;

	pci_info(gpu, "Enabling HDA controller\n");
	pci_write_config_dword(gpu, 0x488, val | BIT(25));

	/* The GPU becomes a multi-function device when the HDA is enabled */
	pci_read_config_byte(gpu, PCI_HEADER_TYPE, &hdr_type);
	gpu->multifunction = !!(hdr_type & 0x80);
}


// Source: quirks.c
// Lines 5018-5038
