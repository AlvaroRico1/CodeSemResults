acpi_ut_execute_CLS(struct acpi_namespace_node *device_node,
		    struct acpi_pnp_device_id **return_id)
{
	union acpi_operand_object *obj_desc;
	union acpi_operand_object **cls_objects;
	u32 count;
	struct acpi_pnp_device_id *cls;
	u32 length;
	acpi_status status;
	u8 class_code[3] = { 0, 0, 0 };

	ACPI_FUNCTION_TRACE(ut_execute_CLS);

	status = acpi_ut_evaluate_object(device_node, METHOD_NAME__CLS,
					 ACPI_BTYPE_PACKAGE, &obj_desc);
	if (ACPI_FAILURE(status)) {
		return_ACPI_STATUS(status);
	}

	/* Get the size of the String to be returned, includes null terminator */

	length = ACPI_PCICLS_STRING_SIZE;
	cls_objects = obj_desc->package.elements;
	count = obj_desc->package.count;

	if (obj_desc->common.type == ACPI_TYPE_PACKAGE) {
		if (count > 0
		    && cls_objects[0]->common.type == ACPI_TYPE_INTEGER) {
			class_code[0] = (u8)cls_objects[0]->integer.value;
		}
		if (count > 1
		    && cls_objects[1]->common.type == ACPI_TYPE_INTEGER) {
			class_code[1] = (u8)cls_objects[1]->integer.value;
		}
		if (count > 2
		    && cls_objects[2]->common.type == ACPI_TYPE_INTEGER) {
			class_code[2] = (u8)cls_objects[2]->integer.value;
		}
	}

	/* Allocate a buffer for the CLS */

	cls =
	    ACPI_ALLOCATE_ZEROED(sizeof(struct acpi_pnp_device_id) +
				 (acpi_size)length);
	if (!cls) {
		status = AE_NO_MEMORY;
		goto cleanup;
	}

	/* Area for the string starts after PNP_DEVICE_ID struct */

	cls->string =
	    ACPI_ADD_PTR(char, cls, sizeof(struct acpi_pnp_device_id));

	/* Simply copy existing string */

	acpi_ex_pci_cls_to_string(cls->string, class_code);
	cls->length = length;
	*return_id = cls;

cleanup:

	/* On exit, we must delete the return object */

	acpi_ut_remove_reference(obj_desc);
	return_ACPI_STATUS(status);
}


// Source: utids.c
// Lines 338-405
