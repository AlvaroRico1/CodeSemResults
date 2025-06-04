ACPI_EXPORT_SYMBOL(acpi_buffer_to_resource)

/*******************************************************************************
 *
 * FUNCTION:    acpi_rs_create_resource_list
 *
 * PARAMETERS:  aml_buffer          - Pointer to the resource byte stream
 *              output_buffer       - Pointer to the user's buffer
 *
 * RETURN:      Status: AE_OK if okay, else a valid acpi_status code
 *              If output_buffer is not large enough, output_buffer_length
 *              indicates how large output_buffer should be, else it
 *              indicates how may u8 elements of output_buffer are valid.
 *
 * DESCRIPTION: Takes the byte stream returned from a _CRS, _PRS control method
 *              execution and parses the stream to create a linked list
 *              of device resources.
 *
 ******************************************************************************/
acpi_status
acpi_rs_create_resource_list(union acpi_operand_object *aml_buffer,
			     struct acpi_buffer *output_buffer)
{

	acpi_status status;
	u8 *aml_start;
	acpi_size list_size_needed = 0;
	u32 aml_buffer_length;
	void *resource;

	ACPI_FUNCTION_TRACE(rs_create_resource_list);

	ACPI_DEBUG_PRINT((ACPI_DB_INFO, "AmlBuffer = %p\n", aml_buffer));

	/* Params already validated, so we don't re-validate here */

	aml_buffer_length = aml_buffer->buffer.length;
	aml_start = aml_buffer->buffer.pointer;

	/*
	 * Pass the aml_buffer into a module that can calculate
	 * the buffer size needed for the linked list
	 */
	status = acpi_rs_get_list_length(aml_start, aml_buffer_length,
					 &list_size_needed);

	ACPI_DEBUG_PRINT((ACPI_DB_INFO, "Status=%X ListSizeNeeded=%X\n",
			  status, (u32) list_size_needed));
	if (ACPI_FAILURE(status)) {
		return_ACPI_STATUS(status);
	}

	/* Validate/Allocate/Clear caller buffer */

	status = acpi_ut_initialize_buffer(output_buffer, list_size_needed);
	if (ACPI_FAILURE(status)) {
		return_ACPI_STATUS(status);
	}

	/* Do the conversion */

	resource = output_buffer->pointer;
	status = acpi_ut_walk_aml_resources(NULL, aml_start, aml_buffer_length,
					    acpi_rs_convert_aml_to_resources,
					    &resource);
	if (ACPI_FAILURE(status)) {
		return_ACPI_STATUS(status);
	}

	ACPI_DEBUG_PRINT((ACPI_DB_INFO, "OutputBuffer %p Length %X\n",
			  output_buffer->pointer, (u32) output_buffer->length));
	return_ACPI_STATUS(AE_OK);
}


// Source: rscreate.c
// Lines 83-155
