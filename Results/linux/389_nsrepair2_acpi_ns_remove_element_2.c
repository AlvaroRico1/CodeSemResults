acpi_ns_remove_element(union acpi_operand_object *obj_desc, u32 index)
{
	union acpi_operand_object **source;
	union acpi_operand_object **dest;
	u32 count;
	u32 new_count;
	u32 i;

	ACPI_FUNCTION_NAME(ns_remove_element);

	count = obj_desc->package.count;
	new_count = count - 1;

	source = obj_desc->package.elements;
	dest = source;

	/* Examine all elements of the package object, remove matched index */

	for (i = 0; i < count; i++) {
		if (i == index) {
			acpi_ut_remove_reference(*source);	/* Remove one ref for being in pkg */
			acpi_ut_remove_reference(*source);
		} else {
			*dest = *source;
			dest++;
		}

		source++;
	}

	/* NULL terminate list and update the package count */

	*dest = NULL;
	obj_desc->package.count = new_count;
}


// Source: nsrepair2.c
// Lines 913-947
