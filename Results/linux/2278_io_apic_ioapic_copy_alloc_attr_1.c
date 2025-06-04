static void ioapic_copy_alloc_attr(struct irq_alloc_info *dst,
				   struct irq_alloc_info *src,
				   u32 gsi, int ioapic_idx, int pin)
{
	int trigger, polarity;

	copy_irq_alloc_info(dst, src);
	dst->type = X86_IRQ_ALLOC_TYPE_IOAPIC;
	dst->ioapic_id = mpc_ioapic_id(ioapic_idx);
	dst->ioapic_pin = pin;
	dst->ioapic_valid = 1;
	if (src && src->ioapic_valid) {
		dst->ioapic_node = src->ioapic_node;
		dst->ioapic_trigger = src->ioapic_trigger;
		dst->ioapic_polarity = src->ioapic_polarity;
	} else {
		dst->ioapic_node = NUMA_NO_NODE;
		if (acpi_get_override_irq(gsi, &trigger, &polarity) >= 0) {
			dst->ioapic_trigger = trigger;
			dst->ioapic_polarity = polarity;
		} else {
			/*
			 * PCI interrupts are always active low level
			 * triggered.
			 */
			dst->ioapic_trigger = IOAPIC_LEVEL;
			dst->ioapic_polarity = IOAPIC_POL_LOW;
		}
	}
}


// Source: io_apic.c
// Lines 886-915
