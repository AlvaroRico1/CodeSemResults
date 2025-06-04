acpi_status acpi_ex_opcode_2A_0T_1R(struct acpi_walk_state *walk_state)
{
	union acpi_operand_object **operand = &walk_state->operands[0];
	union acpi_operand_object *return_desc = NULL;
	acpi_status status = AE_OK;
	u8 logical_result = FALSE;

	ACPI_FUNCTION_TRACE_STR(ex_opcode_2A_0T_1R,
				acpi_ps_get_opcode_name(walk_state->opcode));

	/* Create the internal return object */

	return_desc = acpi_ut_create_internal_object(ACPI_TYPE_INTEGER);
	if (!return_desc) {
		status = AE_NO_MEMORY;
		goto cleanup;
	}

	/* Execute the Opcode */

	if (walk_state->op_info->flags & AML_LOGICAL_NUMERIC) {

		/* logical_op (Operand0, Operand1) */

		status = acpi_ex_do_logical_numeric_op(walk_state->opcode,
						       operand[0]->integer.
						       value,
						       operand[1]->integer.
						       value, &logical_result);
		goto store_logical_result;
	} else if (walk_state->op_info->flags & AML_LOGICAL) {

		/* logical_op (Operand0, Operand1) */

		status = acpi_ex_do_logical_op(walk_state->opcode, operand[0],
					       operand[1], &logical_result);
		goto store_logical_result;
	}

	switch (walk_state->opcode) {
	case AML_ACQUIRE_OP:	/* Acquire (mutex_object, Timeout) */

		status =
		    acpi_ex_acquire_mutex(operand[1], operand[0], walk_state);
		if (status == AE_TIME) {
			logical_result = TRUE;	/* TRUE = Acquire timed out */
			status = AE_OK;
		}
		break;

	case AML_WAIT_OP:	/* Wait (event_object, Timeout) */

		status = acpi_ex_system_wait_event(operand[1], operand[0]);
		if (status == AE_TIME) {
			logical_result = TRUE;	/* TRUE, Wait timed out */
			status = AE_OK;
		}
		break;

	default:

		ACPI_ERROR((AE_INFO, "Unknown AML opcode 0x%X",
			    walk_state->opcode));

		status = AE_AML_BAD_OPCODE;
		goto cleanup;
	}

store_logical_result:
	/*
	 * Set return value to according to logical_result. logical TRUE (all ones)
	 * Default is FALSE (zero)
	 */
	if (logical_result) {
		return_desc->integer.value = ACPI_UINT64_MAX;
	}

cleanup:

	/* Delete return object on error */

	if (ACPI_FAILURE(status)) {
		acpi_ut_remove_reference(return_desc);
	}

	/* Save return object on success */

	else {
		walk_state->result_obj = return_desc;
	}

	return_ACPI_STATUS(status);
}


// Source: exoparg2.c
// Lines 465-557
