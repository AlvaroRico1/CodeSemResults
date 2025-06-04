acpi_ds_create_operand(struct acpi_walk_state *walk_state,
		       union acpi_parse_object *arg, u32 arg_index)
{
	acpi_status status = AE_OK;
	char *name_string;
	u32 name_length;
	union acpi_operand_object *obj_desc;
	union acpi_parse_object *parent_op;
	u16 opcode;
	acpi_interpreter_mode interpreter_mode;
	const struct acpi_opcode_info *op_info;

	ACPI_FUNCTION_TRACE_PTR(ds_create_operand, arg);

	/* A valid name must be looked up in the namespace */

	if ((arg->common.aml_opcode == AML_INT_NAMEPATH_OP) &&
	    (arg->common.value.string) &&
	    !(arg->common.flags & ACPI_PARSEOP_IN_STACK)) {
		ACPI_DEBUG_PRINT((ACPI_DB_DISPATCH, "Getting a name: Arg=%p\n",
				  arg));

		/* Get the entire name string from the AML stream */

		status = acpi_ex_get_name_string(ACPI_TYPE_ANY,
						 arg->common.value.buffer,
						 &name_string, &name_length);

		if (ACPI_FAILURE(status)) {
			return_ACPI_STATUS(status);
		}

		/* All prefixes have been handled, and the name is in name_string */

		/*
		 * Special handling for buffer_field declarations. This is a deferred
		 * opcode that unfortunately defines the field name as the last
		 * parameter instead of the first. We get here when we are performing
		 * the deferred execution, so the actual name of the field is already
		 * in the namespace. We don't want to attempt to look it up again
		 * because we may be executing in a different scope than where the
		 * actual opcode exists.
		 */
		if ((walk_state->deferred_node) &&
		    (walk_state->deferred_node->type == ACPI_TYPE_BUFFER_FIELD)
		    && (arg_index == (u32)
			((walk_state->opcode == AML_CREATE_FIELD_OP) ? 3 : 2))) {
			obj_desc =
			    ACPI_CAST_PTR(union acpi_operand_object,
					  walk_state->deferred_node);
			status = AE_OK;
		} else {	/* All other opcodes */

			/*
			 * Differentiate between a namespace "create" operation
			 * versus a "lookup" operation (IMODE_LOAD_PASS2 vs.
			 * IMODE_EXECUTE) in order to support the creation of
			 * namespace objects during the execution of control methods.
			 */
			parent_op = arg->common.parent;
			op_info =
			    acpi_ps_get_opcode_info(parent_op->common.
						    aml_opcode);

			if ((op_info->flags & AML_NSNODE) &&
			    (parent_op->common.aml_opcode !=
			     AML_INT_METHODCALL_OP)
			    && (parent_op->common.aml_opcode != AML_REGION_OP)
			    && (parent_op->common.aml_opcode !=
				AML_INT_NAMEPATH_OP)) {

				/* Enter name into namespace if not found */

				interpreter_mode = ACPI_IMODE_LOAD_PASS2;
			} else {
				/* Return a failure if name not found */

				interpreter_mode = ACPI_IMODE_EXECUTE;
			}

			status =
			    acpi_ns_lookup(walk_state->scope_info, name_string,
					   ACPI_TYPE_ANY, interpreter_mode,
					   ACPI_NS_SEARCH_PARENT |
					   ACPI_NS_DONT_OPEN_SCOPE, walk_state,
					   ACPI_CAST_INDIRECT_PTR(struct
								  acpi_namespace_node,
								  &obj_desc));
			/*
			 * The only case where we pass through (ignore) a NOT_FOUND
			 * error is for the cond_ref_of opcode.
			 */
			if (status == AE_NOT_FOUND) {
				if (parent_op->common.aml_opcode ==
				    AML_CONDITIONAL_REF_OF_OP) {
					/*
					 * For the Conditional Reference op, it's OK if
					 * the name is not found;  We just need a way to
					 * indicate this to the interpreter, set the
					 * object to the root
					 */
					obj_desc =
					    ACPI_CAST_PTR(union
								 acpi_operand_object,
								 acpi_gbl_root_node);
					status = AE_OK;
				} else if (parent_op->common.aml_opcode ==
					   AML_EXTERNAL_OP) {
					/*
					 * This opcode should never appear here. It is used only
					 * by AML disassemblers and is surrounded by an If(0)
					 * by the ASL compiler.
					 *
					 * Therefore, if we see it here, it is a serious error.
					 */
					status = AE_AML_BAD_OPCODE;
				} else {
					/*
					 * We just plain didn't find it -- which is a
					 * very serious error at this point
					 */
					status = AE_AML_NAME_NOT_FOUND;
				}
			}

			if (ACPI_FAILURE(status)) {
				ACPI_ERROR_NAMESPACE(walk_state->scope_info,
						     name_string, status);
			}
		}

		/* Free the namestring created above */

		ACPI_FREE(name_string);

		/* Check status from the lookup */

		if (ACPI_FAILURE(status)) {
			return_ACPI_STATUS(status);
		}

		/* Put the resulting object onto the current object stack */

		status = acpi_ds_obj_stack_push(obj_desc, walk_state);
		if (ACPI_FAILURE(status)) {
			return_ACPI_STATUS(status);
		}

		acpi_db_display_argument_object(obj_desc, walk_state);
	} else {
		/* Check for null name case */

		if ((arg->common.aml_opcode == AML_INT_NAMEPATH_OP) &&
		    !(arg->common.flags & ACPI_PARSEOP_IN_STACK)) {
			/*
			 * If the name is null, this means that this is an
			 * optional result parameter that was not specified
			 * in the original ASL. Create a Zero Constant for a
			 * placeholder. (Store to a constant is a Noop.)
			 */
			opcode = AML_ZERO_OP;	/* Has no arguments! */

			ACPI_DEBUG_PRINT((ACPI_DB_DISPATCH,
					  "Null namepath: Arg=%p\n", arg));
		} else {
			opcode = arg->common.aml_opcode;
		}

		/* Get the object type of the argument */

		op_info = acpi_ps_get_opcode_info(opcode);
		if (op_info->object_type == ACPI_TYPE_INVALID) {
			return_ACPI_STATUS(AE_NOT_IMPLEMENTED);
		}

		if ((op_info->flags & AML_HAS_RETVAL) ||
		    (arg->common.flags & ACPI_PARSEOP_IN_STACK)) {
			/*
			 * Use value that was already previously returned
			 * by the evaluation of this argument
			 */
			status = acpi_ds_result_pop(&obj_desc, walk_state);
			if (ACPI_FAILURE(status)) {
				/*
				 * Only error is underflow, and this indicates
				 * a missing or null operand!
				 */
				ACPI_EXCEPTION((AE_INFO, status,
						"Missing or null operand"));
				return_ACPI_STATUS(status);
			}
		} else {
			/* Create an ACPI_INTERNAL_OBJECT for the argument */

			obj_desc =
			    acpi_ut_create_internal_object(op_info->
							   object_type);
			if (!obj_desc) {
				return_ACPI_STATUS(AE_NO_MEMORY);
			}

			/* Initialize the new object */

			status =
			    acpi_ds_init_object_from_op(walk_state, arg, opcode,
							&obj_desc);
			if (ACPI_FAILURE(status)) {
				acpi_ut_delete_object_desc(obj_desc);
				return_ACPI_STATUS(status);
			}
		}

		/* Put the operand object on the object stack */

		status = acpi_ds_obj_stack_push(obj_desc, walk_state);
		if (ACPI_FAILURE(status)) {
			return_ACPI_STATUS(status);
		}

		acpi_db_display_argument_object(obj_desc, walk_state);
	}

	return_ACPI_STATUS(AE_OK);
}


// Source: dsutils.c
// Lines 422-645
