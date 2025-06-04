acpi_ns_repair_CID(struct acpi_evaluate_info *info,
		   union acpi_operand_object **return_object_ptr)
{
	acpi_status status;
	union acpi_operand_object *return_object = *return_object_ptr;
	union acpi_operand_object **element_ptr;
	union acpi_operand_object *original_element;
	u16 original_ref_count;
	u32 i;

	/* Check for _CID as a simple string */

	if (return_object->common.type == ACPI_TYPE_STRING) {
		status = acpi_ns_repair_HID(info, return_object_ptr);
		return (status);
	}

	/* Exit if not a Package */

	if (return_object->common.type != ACPI_TYPE_PACKAGE) {
		return (AE_OK);
	}

	/* Examine each element of the _CID package */

	element_ptr = return_object->package.elements;
	for (i = 0; i < return_object->package.count; i++) {
		original_element = *element_ptr;
		original_ref_count = original_element->common.reference_count;

		status = acpi_ns_repair_HID(info, element_ptr);
		if (ACPI_FAILURE(status)) {
			return (status);
		}

		if (original_element != *element_ptr) {

			/* Update reference count of new object */

			(*element_ptr)->common.reference_count =
			    original_ref_count;
		}

		element_ptr++;
	}

	return (AE_OK);
}


// Source: nsrepair2.c
// Lines 337-384
