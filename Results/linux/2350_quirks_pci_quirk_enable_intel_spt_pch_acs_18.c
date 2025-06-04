static int pci_quirk_enable_intel_spt_pch_acs(struct pci_dev *dev)
{
	int pos;
	u32 cap, ctrl;

	if (!pci_quirk_intel_spt_pch_acs_match(dev))
		return -ENOTTY;

	pos = pci_find_ext_capability(dev, PCI_EXT_CAP_ID_ACS);
	if (!pos)
		return -ENOTTY;

	pci_read_config_dword(dev, pos + PCI_ACS_CAP, &cap);
	pci_read_config_dword(dev, pos + INTEL_SPT_ACS_CTRL, &ctrl);

	ctrl |= (cap & PCI_ACS_SV);
	ctrl |= (cap & PCI_ACS_RR);
	ctrl |= (cap & PCI_ACS_CR);
	ctrl |= (cap & PCI_ACS_UF);

	pci_write_config_dword(dev, pos + INTEL_SPT_ACS_CTRL, ctrl);

	pci_info(dev, "Intel SPT PCH root port ACS workaround enabled\n");

	return 0;
}


// Source: quirks.c
// Lines 4693-4718
