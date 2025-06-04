ACPI_EXPORT_SYMBOL(acpi_write_bit_register)
#endif				/* !ACPI_REDUCED_HARDWARE */
/*******************************************************************************
 *
 * FUNCTION:    acpi_get_sleep_type_data
 *
 * PARAMETERS:  sleep_state         - Numeric sleep state
 *              *sleep_type_a        - Where SLP_TYPa is returned
 *              *sleep_type_b        - Where SLP_TYPb is returned
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Obtain the SLP_TYPa and SLP_TYPb values for the requested
 *              sleep state via the appropriate \_Sx object.
 *
 *  The sleep state package returned from the corresponding \_Sx_ object
 *  must contain at least one integer.
 *
 *  March 2005:
 *  Added support for a package that contains two integers. This
 *  goes against the ACPI specification which defines this object as a
 *  package with one encoded DWORD integer. However, existing practice
 *  by many BIOS vendors is to return a package with 2 or more integer
 *  elements, at least one per sleep type (A/B).
 *
 *  January 2013:
 *  Therefore, we must be prepared to accept a package with either a
 *  single integer or multiple integers.
 *
 *  The single integer DWORD format is as follows:
 *      BYTE 0 - Value for the PM1A SLP_TYP register
 *      BYTE 1 - Value for the PM1B SLP_TYP register
 *      BYTE 2-3 - Reserved
 *
 *  The dual integer format is as follows:
 *      Integer 0 - Value for the PM1A SLP_TYP register
 *      Integer 1 - Value for the PM1A SLP_TYP register
 *
 ******************************************************************************/
acpi_status
acpi_get_sleep_type_data(u8 sleep_state, u8 *sleep_type_a, u8 *sleep_type_b)
{
	acpi_status status;
	struct acpi_evaluate_info *info;
	union acpi_operand_object **elements;

	ACPI_FUNCTION_TRACE(acpi_get_sleep_type_data);

	/* Validate parameters */

	if ((sleep_state > ACPI_S_STATES_MAX) || !sleep_type_a || !sleep_type_b) {
		return_ACPI_STATUS(AE_BAD_PARAMETER);
	}

	/* Allocate the evaluation information block */

	info = ACPI_ALLOCATE_ZEROED(sizeof(struct acpi_evaluate_info));
	if (!info) {
		return_ACPI_STATUS(AE_NO_MEMORY);
	}

	/*
	 * Evaluate the \_Sx namespace object containing the register values
	 * for this state
	 */
	info->relative_pathname = acpi_gbl_sleep_state_names[sleep_state];

	status = acpi_ns_evaluate(info);
	if (ACPI_FAILURE(status)) {
		if (status == AE_NOT_FOUND) {

			/* The _Sx states are optional, ignore NOT_FOUND */

			goto final_cleanup;
		}

		goto warning_cleanup;
	}

	/* Must have a return object */

	if (!info->return_object) {
		ACPI_ERROR((AE_INFO, "No Sleep State object returned from [%s]",
			    info->relative_pathname));
		status = AE_AML_NO_RETURN_VALUE;
		goto warning_cleanup;
	}

	/* Return object must be of type Package */

	if (info->return_object->common.type != ACPI_TYPE_PACKAGE) {
		ACPI_ERROR((AE_INFO,
			    "Sleep State return object is not a Package"));
		status = AE_AML_OPERAND_TYPE;
		goto return_value_cleanup;
	}

	/*
	 * Any warnings about the package length or the object types have
	 * already been issued by the predefined name module -- there is no
	 * need to repeat them here.
	 */
	elements = info->return_object->package.elements;
	switch (info->return_object->package.count) {
	case 0:

		status = AE_AML_PACKAGE_LIMIT;
		break;

	case 1:

		if (elements[0]->common.type != ACPI_TYPE_INTEGER) {
			status = AE_AML_OPERAND_TYPE;
			break;
		}

		/* A valid _Sx_ package with one integer */

		*sleep_type_a = (u8)elements[0]->integer.value;
		*sleep_type_b = (u8)(elements[0]->integer.value >> 8);
		break;

	case 2:
	default:

		if ((elements[0]->common.type != ACPI_TYPE_INTEGER) ||
		    (elements[1]->common.type != ACPI_TYPE_INTEGER)) {
			status = AE_AML_OPERAND_TYPE;
			break;
		}

		/* A valid _Sx_ package with two integers */

		*sleep_type_a = (u8)elements[0]->integer.value;
		*sleep_type_b = (u8)elements[1]->integer.value;
		break;
	}

return_value_cleanup:
	acpi_ut_remove_reference(info->return_object);

warning_cleanup:
	if (ACPI_FAILURE(status)) {
		ACPI_EXCEPTION((AE_INFO, status,
				"While evaluating Sleep State [%s]",
				info->relative_pathname));
	}

final_cleanup:
	ACPI_FREE(info);
	return_ACPI_STATUS(status);
}


// Source: hwxface.c
// Lines 295-446
