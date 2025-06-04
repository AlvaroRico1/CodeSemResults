static bool pci_acs_flags_enabled(struct pci_dev *pdev, u16 acs_flags)
{
	int pos;
	u16 cap, ctrl;

	pos = pci_find_ext_capability(pdev, PCI_EXT_CAP_ID_ACS);
	if (!pos)
		return false;

	/*
	 * Except for egress control, capabilities are either required
	 * or only required if controllable.  Features missing from the
	 * capability field can therefore be assumed as hard-wired enabled.
	 */
	pci_read_config_word(pdev, pos + PCI_ACS_CAP, &cap);
	acs_flags &= (cap | PCI_ACS_EC);

	pci_read_config_word(pdev, pos + PCI_ACS_CTRL, &ctrl);
	return (ctrl & acs_flags) == acs_flags;
}


// Source: pci.c
// Lines 3294-3313
