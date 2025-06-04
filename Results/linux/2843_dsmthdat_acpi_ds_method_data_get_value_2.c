acpi_ds_method_data_get_value(u8 type,
			      u32 index,
			      struct acpi_walk_state *walk_state,
			      union acpi_operand_object **dest_desc)
{
	acpi_status status;
	struct acpi_namespace_node *node;
	union acpi_operand_object *object;

	ACPI_FUNCTION_TRACE(ds_method_data_get_value);

	/* Validate the object descriptor */

	if (!dest_desc) {
		ACPI_ERROR((AE_INFO, "Null object descriptor pointer"));
		return_ACPI_STATUS(AE_BAD_PARAMETER);
	}

	/* Get the namespace node for the arg/local */

	status = acpi_ds_method_data_get_node(type, index, walk_state, &node);
	if (ACPI_FAILURE(status)) {
		return_ACPI_STATUS(status);
	}

	/* Get the object from the node */

	object = node->object;

	/* Examine the returned object, it must be valid. */

	if (!object) {
		/*
		 * Index points to uninitialized object.
		 * This means that either 1) The expected argument was
		 * not passed to the method, or 2) A local variable
		 * was referenced by the method (via the ASL)
		 * before it was initialized. Either case is an error.
		 */

		/* If slack enabled, init the local_x/arg_x to an Integer of value zero */

		if (acpi_gbl_enable_interpreter_slack) {
			object = acpi_ut_create_integer_object((u64) 0);
			if (!object) {
				return_ACPI_STATUS(AE_NO_MEMORY);
			}

			node->object = object;
		}

		/* Otherwise, return the error */

		else
			switch (type) {
			case ACPI_REFCLASS_ARG:

				ACPI_ERROR((AE_INFO,
					    "Uninitialized Arg[%u] at node %p",
					    index, node));

				return_ACPI_STATUS(AE_AML_UNINITIALIZED_ARG);

			case ACPI_REFCLASS_LOCAL:
				/*
				 * No error message for this case, will be trapped again later to
				 * detect and ignore cases of Store(local_x,local_x)
				 */
				return_ACPI_STATUS(AE_AML_UNINITIALIZED_LOCAL);

			default:

				ACPI_ERROR((AE_INFO,
					    "Not a Arg/Local opcode: 0x%X",
					    type));
				return_ACPI_STATUS(AE_AML_INTERNAL);
			}
	}

	/*
	 * The Index points to an initialized and valid object.
	 * Return an additional reference to the object
	 */
	*dest_desc = object;
	acpi_ut_add_reference(object);

	return_ACPI_STATUS(AE_OK);
}


// Source: dsmthdat.c
// Lines 333-420
