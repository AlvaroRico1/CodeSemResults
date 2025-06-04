static bool pci_ext_cfg_is_aliased(struct pci_dev *dev)
{
#ifdef CONFIG_PCI_QUIRKS
	int pos;
	u32 header, tmp;

	pci_read_config_dword(dev, PCI_VENDOR_ID, &header);

	for (pos = PCI_CFG_SPACE_SIZE;
	     pos < PCI_CFG_SPACE_EXP_SIZE; pos += PCI_CFG_SPACE_SIZE) {
		if (pci_read_config_dword(dev, pos, &tmp) != PCIBIOS_SUCCESSFUL
		    || header != tmp)
			return false;
	}

	return true;
#else
	return false;
#endif
}


// Source: probe.c
// Lines 1513-1532
