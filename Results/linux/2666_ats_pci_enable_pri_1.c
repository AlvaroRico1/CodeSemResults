int pci_enable_pri(struct pci_dev *pdev, u32 reqs)
{
	u16 control, status;
	u32 max_requests;
	int pos;

	if (WARN_ON(pdev->pri_enabled))
		return -EBUSY;

	pos = pci_find_ext_capability(pdev, PCI_EXT_CAP_ID_PRI);
	if (!pos)
		return -EINVAL;

	pci_read_config_word(pdev, pos + PCI_PRI_STATUS, &status);
	if (!(status & PCI_PRI_STATUS_STOPPED))
		return -EBUSY;

	pci_read_config_dword(pdev, pos + PCI_PRI_MAX_REQ, &max_requests);
	reqs = min(max_requests, reqs);
	pdev->pri_reqs_alloc = reqs;
	pci_write_config_dword(pdev, pos + PCI_PRI_ALLOC_REQ, reqs);

	control = PCI_PRI_CTRL_ENABLE;
	pci_write_config_word(pdev, pos + PCI_PRI_CTRL, control);

	pdev->pri_enabled = 1;

	return 0;
}


// Source: ats.c
// Lines 179-207
