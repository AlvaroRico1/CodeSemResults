static u32 ati_ixp4x0_rev(struct pci_dev *dev)
{
	int err = 0;
	u32 d = 0;
	u8  b = 0;

	err = pci_read_config_byte(dev, 0xac, &b);
	b &= ~(1<<5);
	err |= pci_write_config_byte(dev, 0xac, b);
	err |= pci_read_config_dword(dev, 0x70, &d);
	d |= 1<<8;
	err |= pci_write_config_dword(dev, 0x70, d);
	err |= pci_read_config_dword(dev, 0x8, &d);
	d &= 0xff;
	dev_printk(KERN_DEBUG, &dev->dev, "SB4X0 revision 0x%x\n", d);

	WARN_ON_ONCE(err);

	return d;
}


// Source: quirks.c
// Lines 358-377
