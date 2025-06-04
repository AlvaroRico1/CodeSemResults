void pci_aer_clear_fatal_status(struct pci_dev *dev)
{
	int pos;
	u32 status, sev;

	pos = dev->aer_cap;
	if (!pos)
		return;

	if (pcie_aer_get_firmware_first(dev))
		return;

	/* Clear status bits for ERR_FATAL errors only */
	pci_read_config_dword(dev, pos + PCI_ERR_UNCOR_STATUS, &status);
	pci_read_config_dword(dev, pos + PCI_ERR_UNCOR_SEVER, &sev);
	status &= sev;
	if (status)
		pci_write_config_dword(dev, pos + PCI_ERR_UNCOR_STATUS, status);
}


// Source: aer.c
// Lines 400-418
