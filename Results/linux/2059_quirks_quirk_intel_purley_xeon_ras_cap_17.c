static void quirk_intel_purley_xeon_ras_cap(struct pci_dev *pdev)
{
	u32 capid0, capid5;

	pci_read_config_dword(pdev, 0x84, &capid0);
	pci_read_config_dword(pdev, 0x98, &capid5);

	/*
	 * CAPID0{7:6} indicate whether this is an advanced RAS SKU
	 * CAPID5{8:5} indicate that various NVDIMM usage modes are
	 * enabled, so memory machine check recovery is also enabled.
	 */
	if ((capid0 & 0xc0) == 0xc0 || (capid5 & 0x1e0))
		static_branch_inc(&mcsafe_key);

}


// Source: quirks.c
// Lines 647-662
