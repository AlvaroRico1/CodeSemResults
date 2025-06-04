static int do_pci_enable_device(struct pci_dev *dev, int bars)
{
	int err;
	struct pci_dev *bridge;
	u16 cmd;
	u8 pin;

	err = pci_set_power_state(dev, PCI_D0);
	if (err < 0 && err != -EIO)
		return err;

	bridge = pci_upstream_bridge(dev);
	if (bridge)
		pcie_aspm_powersave_config_link(bridge);

	err = pcibios_enable_device(dev, bars);
	if (err < 0)
		return err;
	pci_fixup_device(pci_fixup_enable, dev);

	if (dev->msi_enabled || dev->msix_enabled)
		return 0;

	pci_read_config_byte(dev, PCI_INTERRUPT_PIN, &pin);
	if (pin) {
		pci_read_config_word(dev, PCI_COMMAND, &cmd);
		if (cmd & PCI_COMMAND_INTX_DISABLE)
			pci_write_config_word(dev, PCI_COMMAND,
					      cmd & ~PCI_COMMAND_INTX_DISABLE);
	}

	return 0;
}


// Source: pci.c
// Lines 1594-1626
