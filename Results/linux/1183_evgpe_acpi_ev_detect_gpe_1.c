acpi_ev_detect_gpe(struct acpi_namespace_node *gpe_device,
		   struct acpi_gpe_event_info *gpe_event_info, u32 gpe_number)
{
	u32 int_status = ACPI_INTERRUPT_NOT_HANDLED;
	u8 enabled_status_byte;
	u64 status_reg;
	u64 enable_reg;
	u32 register_bit;
	struct acpi_gpe_register_info *gpe_register_info;
	struct acpi_gpe_handler_info *gpe_handler_info;
	acpi_cpu_flags flags;
	acpi_status status;

	ACPI_FUNCTION_TRACE(ev_gpe_detect);

	flags = acpi_os_acquire_lock(acpi_gbl_gpe_lock);

	if (!gpe_event_info) {
		gpe_event_info = acpi_ev_get_gpe_event_info(gpe_device, gpe_number);
		if (!gpe_event_info)
			goto error_exit;
	}

	/* Get the info block for the entire GPE register */

	gpe_register_info = gpe_event_info->register_info;

	/* Get the register bitmask for this GPE */

	register_bit = acpi_hw_get_gpe_register_bit(gpe_event_info);

	/* GPE currently enabled (enable bit == 1)? */

	status = acpi_hw_read(&enable_reg, &gpe_register_info->enable_address);
	if (ACPI_FAILURE(status)) {
		goto error_exit;
	}

	/* GPE currently active (status bit == 1)? */

	status = acpi_hw_read(&status_reg, &gpe_register_info->status_address);
	if (ACPI_FAILURE(status)) {
		goto error_exit;
	}

	/* Check if there is anything active at all in this GPE */

	ACPI_DEBUG_PRINT((ACPI_DB_INTERRUPTS,
			  "Read registers for GPE %02X: Status=%02X, Enable=%02X, "
			  "RunEnable=%02X, WakeEnable=%02X\n",
			  gpe_number,
			  (u32)(status_reg & register_bit),
			  (u32)(enable_reg & register_bit),
			  gpe_register_info->enable_for_run,
			  gpe_register_info->enable_for_wake));

	enabled_status_byte = (u8)(status_reg & enable_reg);
	if (!(enabled_status_byte & register_bit)) {
		goto error_exit;
	}

	/* Invoke global event handler if present */

	acpi_gpe_count++;
	if (acpi_gbl_global_event_handler) {
		acpi_gbl_global_event_handler(ACPI_EVENT_TYPE_GPE,
					      gpe_device, gpe_number,
					      acpi_gbl_global_event_handler_context);
	}

	/* Found an active GPE */

	if (ACPI_GPE_DISPATCH_TYPE(gpe_event_info->flags) ==
	    ACPI_GPE_DISPATCH_RAW_HANDLER) {

		/* Dispatch the event to a raw handler */

		gpe_handler_info = gpe_event_info->dispatch.handler;

		/*
		 * There is no protection around the namespace node
		 * and the GPE handler to ensure a safe destruction
		 * because:
		 * 1. The namespace node is expected to always
		 *    exist after loading a table.
		 * 2. The GPE handler is expected to be flushed by
		 *    acpi_os_wait_events_complete() before the
		 *    destruction.
		 */
		acpi_os_release_lock(acpi_gbl_gpe_lock, flags);
		int_status |=
		    gpe_handler_info->address(gpe_device, gpe_number,
					      gpe_handler_info->context);
		flags = acpi_os_acquire_lock(acpi_gbl_gpe_lock);
	} else {
		/* Dispatch the event to a standard handler or method. */

		int_status |= acpi_ev_gpe_dispatch(gpe_device,
						   gpe_event_info, gpe_number);
	}

error_exit:
	acpi_os_release_lock(acpi_gbl_gpe_lock, flags);
	return (int_status);
}


// Source: evgpe.c
// Lines 626-730
