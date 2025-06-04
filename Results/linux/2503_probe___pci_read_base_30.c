int __pci_read_base(struct pci_dev *dev, enum pci_bar_type type,
		    struct resource *res, unsigned int pos)
{
	u32 l = 0, sz = 0, mask;
	u64 l64, sz64, mask64;
	u16 orig_cmd;
	struct pci_bus_region region, inverted_region;

	mask = type ? PCI_ROM_ADDRESS_MASK : ~0;

	/* No printks while decoding is disabled! */
	if (!dev->mmio_always_on) {
		pci_read_config_word(dev, PCI_COMMAND, &orig_cmd);
		if (orig_cmd & PCI_COMMAND_DECODE_ENABLE) {
			pci_write_config_word(dev, PCI_COMMAND,
				orig_cmd & ~PCI_COMMAND_DECODE_ENABLE);
		}
	}

	res->name = pci_name(dev);

	pci_read_config_dword(dev, pos, &l);
	pci_write_config_dword(dev, pos, l | mask);
	pci_read_config_dword(dev, pos, &sz);
	pci_write_config_dword(dev, pos, l);

	/*
	 * All bits set in sz means the device isn't working properly.
	 * If the BAR isn't implemented, all bits must be 0.  If it's a
	 * memory BAR or a ROM, bit 0 must be clear; if it's an io BAR, bit
	 * 1 must be clear.
	 */
	if (sz == 0xffffffff)
		sz = 0;

	/*
	 * I don't know how l can have all bits set.  Copied from old code.
	 * Maybe it fixes a bug on some ancient platform.
	 */
	if (l == 0xffffffff)
		l = 0;

	if (type == pci_bar_unknown) {
		res->flags = decode_bar(dev, l);
		res->flags |= IORESOURCE_SIZEALIGN;
		if (res->flags & IORESOURCE_IO) {
			l64 = l & PCI_BASE_ADDRESS_IO_MASK;
			sz64 = sz & PCI_BASE_ADDRESS_IO_MASK;
			mask64 = PCI_BASE_ADDRESS_IO_MASK & (u32)IO_SPACE_LIMIT;
		} else {
			l64 = l & PCI_BASE_ADDRESS_MEM_MASK;
			sz64 = sz & PCI_BASE_ADDRESS_MEM_MASK;
			mask64 = (u32)PCI_BASE_ADDRESS_MEM_MASK;
		}
	} else {
		if (l & PCI_ROM_ADDRESS_ENABLE)
			res->flags |= IORESOURCE_ROM_ENABLE;
		l64 = l & PCI_ROM_ADDRESS_MASK;
		sz64 = sz & PCI_ROM_ADDRESS_MASK;
		mask64 = PCI_ROM_ADDRESS_MASK;
	}

	if (res->flags & IORESOURCE_MEM_64) {
		pci_read_config_dword(dev, pos + 4, &l);
		pci_write_config_dword(dev, pos + 4, ~0);
		pci_read_config_dword(dev, pos + 4, &sz);
		pci_write_config_dword(dev, pos + 4, l);

		l64 |= ((u64)l << 32);
		sz64 |= ((u64)sz << 32);
		mask64 |= ((u64)~0 << 32);
	}

	if (!dev->mmio_always_on && (orig_cmd & PCI_COMMAND_DECODE_ENABLE))
		pci_write_config_word(dev, PCI_COMMAND, orig_cmd);

	if (!sz64)
		goto fail;

	sz64 = pci_size(l64, sz64, mask64);
	if (!sz64) {
		pci_info(dev, FW_BUG "reg 0x%x: invalid BAR (can't size)\n",
			 pos);
		goto fail;
	}

	if (res->flags & IORESOURCE_MEM_64) {
		if ((sizeof(pci_bus_addr_t) < 8 || sizeof(resource_size_t) < 8)
		    && sz64 > 0x100000000ULL) {
			res->flags |= IORESOURCE_UNSET | IORESOURCE_DISABLED;
			res->start = 0;
			res->end = 0;
			pci_err(dev, "reg 0x%x: can't handle BAR larger than 4GB (size %#010llx)\n",
				pos, (unsigned long long)sz64);
			goto out;
		}

		if ((sizeof(pci_bus_addr_t) < 8) && l) {
			/* Above 32-bit boundary; try to reallocate */
			res->flags |= IORESOURCE_UNSET;
			res->start = 0;
			res->end = sz64 - 1;
			pci_info(dev, "reg 0x%x: can't handle BAR above 4GB (bus address %#010llx)\n",
				 pos, (unsigned long long)l64);
			goto out;
		}
	}

	region.start = l64;
	region.end = l64 + sz64 - 1;

	pcibios_bus_to_resource(dev->bus, res, &region);
	pcibios_resource_to_bus(dev->bus, &inverted_region, res);

	/*
	 * If "A" is a BAR value (a bus address), "bus_to_resource(A)" is
	 * the corresponding resource address (the physical address used by
	 * the CPU.  Converting that resource address back to a bus address
	 * should yield the original BAR value:
	 *
	 *     resource_to_bus(bus_to_resource(A)) == A
	 *
	 * If it doesn't, CPU accesses to "bus_to_resource(A)" will not
	 * be claimed by the device.
	 */
	if (inverted_region.start != region.start) {
		res->flags |= IORESOURCE_UNSET;
		res->start = 0;
		res->end = region.end - region.start;
		pci_info(dev, "reg 0x%x: initial BAR value %#010llx invalid\n",
			 pos, (unsigned long long)region.start);
	}

	goto out;


fail:
	res->flags = 0;
out:
	if (res->flags)
		pci_info(dev, "reg 0x%x: %pR\n", pos, res);

	return (res->flags & IORESOURCE_MEM_64) ? 1 : 0;
}


// Source: probe.c
// Lines 180-323
