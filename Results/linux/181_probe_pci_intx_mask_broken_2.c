static int pci_intx_mask_broken(struct pci_dev *dev)
{
	u16 orig, toggle, new;

	pci_read_config_word(dev, PCI_COMMAND, &orig);
	toggle = orig ^ PCI_COMMAND_INTX_DISABLE;
	pci_write_config_word(dev, PCI_COMMAND, toggle);
	pci_read_config_word(dev, PCI_COMMAND, &new);

	pci_write_config_word(dev, PCI_COMMAND, orig);

	/*
	 * PCI_COMMAND_INTX_DISABLE was reserved and read-only prior to PCI
	 * r2.3, so strictly speaking, a device is not *broken* if it's not
	 * writable.  But we'll live with the misnomer for now.
	 */
	if (new != toggle)
		return 1;
	return 0;
}


// Source: probe.c
// Lines 1662-1681
