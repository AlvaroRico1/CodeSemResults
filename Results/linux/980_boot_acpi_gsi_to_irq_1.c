int acpi_gsi_to_irq(u32 gsi, unsigned int *irqp)
{
	int rc, irq, trigger, polarity;

	if (acpi_irq_model == ACPI_IRQ_MODEL_PIC) {
		*irqp = gsi;
		return 0;
	}

	rc = acpi_get_override_irq(gsi, &trigger, &polarity);
	if (rc)
		return rc;

	trigger = trigger ? ACPI_LEVEL_SENSITIVE : ACPI_EDGE_SENSITIVE;
	polarity = polarity ? ACPI_ACTIVE_LOW : ACPI_ACTIVE_HIGH;
	irq = acpi_register_gsi(NULL, gsi, trigger, polarity);
	if (irq < 0)
		return irq;

	*irqp = irq;
	return 0;
}


// Source: boot.c
// Lines 604-625
