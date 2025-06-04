void acpi_ut_delete_internal_object_list(union acpi_operand_object **obj_list)
{
	union acpi_operand_object **internal_obj;

	ACPI_FUNCTION_ENTRY();

	/* Walk the null-terminated internal list */

	for (internal_obj = obj_list; *internal_obj; internal_obj++) {
		acpi_ut_remove_reference(*internal_obj);
	}

	/* Free the combined parameter pointer list and object array */

	ACPI_FREE(obj_list);
	return;
}


// Source: utdelete.c
// Lines 325-341
