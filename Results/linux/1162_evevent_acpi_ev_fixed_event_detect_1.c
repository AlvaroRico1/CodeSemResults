u32 acpi_ev_fixed_event_detect(void)
{
	u32 int_status = ACPI_INTERRUPT_NOT_HANDLED;
	u32 fixed_status;
	u32 fixed_enable;
	u32 i;
	acpi_status status;

	ACPI_FUNCTION_NAME(ev_fixed_event_detect);

	/*
	 * Read the fixed feature status and enable registers, as all the cases
	 * depend on their values. Ignore errors here.
	 */
	status = acpi_hw_register_read(ACPI_REGISTER_PM1_STATUS, &fixed_status);
	status |=
	    acpi_hw_register_read(ACPI_REGISTER_PM1_ENABLE, &fixed_enable);
	if (ACPI_FAILURE(status)) {
		return (int_status);
	}

	ACPI_DEBUG_PRINT((ACPI_DB_INTERRUPTS,
			  "Fixed Event Block: Enable %08X Status %08X\n",
			  fixed_enable, fixed_status));

	/*
	 * Check for all possible Fixed Events and dispatch those that are active
	 */
	for (i = 0; i < ACPI_NUM_FIXED_EVENTS; i++) {

		/* Both the status and enable bits must be on for this event */

		if ((fixed_status & acpi_gbl_fixed_event_info[i].
		     status_bit_mask)
		    && (fixed_enable & acpi_gbl_fixed_event_info[i].
			enable_bit_mask)) {
			/*
			 * Found an active (signalled) event. Invoke global event
			 * handler if present.
			 */
			acpi_fixed_event_count[i]++;
			if (acpi_gbl_global_event_handler) {
				acpi_gbl_global_event_handler
				    (ACPI_EVENT_TYPE_FIXED, NULL, i,
				     acpi_gbl_global_event_handler_context);
			}

			int_status |= acpi_ev_fixed_event_dispatch(i);
		}
	}

	return (int_status);
}


// Source: evevent.c
// Lines 167-219
