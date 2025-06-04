static void quirk_apple_poweroff_thunderbolt(struct pci_dev *dev)
{
	acpi_handle bridge, SXIO, SXFP, SXLV;

	if (!x86_apple_machine)
		return;
	if (pci_pcie_type(dev) != PCI_EXP_TYPE_UPSTREAM)
		return;
	bridge = ACPI_HANDLE(&dev->dev);
	if (!bridge)
		return;

	/*
	 * SXIO and SXLV are present only on machines requiring this quirk.
	 * Thunderbolt bridges in external devices might have the same
	 * device ID as those on the host, but they will not have the
	 * associated ACPI methods. This implicitly checks that we are at
	 * the right bridge.
	 */
	if (ACPI_FAILURE(acpi_get_handle(bridge, "DSB0.NHI0.SXIO", &SXIO))
	    || ACPI_FAILURE(acpi_get_handle(bridge, "DSB0.NHI0.SXFP", &SXFP))
	    || ACPI_FAILURE(acpi_get_handle(bridge, "DSB0.NHI0.SXLV", &SXLV)))
		return;
	pci_info(dev, "quirk: cutting power to Thunderbolt controller...\n");

	/* magic sequence */
	acpi_execute_simple_method(SXIO, NULL, 1);
	acpi_execute_simple_method(SXFP, NULL, 0);
	msleep(300);
	acpi_execute_simple_method(SXLV, NULL, 0);
	acpi_execute_simple_method(SXIO, NULL, 0);
	acpi_execute_simple_method(SXLV, NULL, 0);
}


// Source: quirks.c
// Lines 3496-3528
