acpi_ex_write_with_update_rule(union acpi_operand_object *obj_desc,
			       u64 mask,
			       u64 field_value, u32 field_datum_byte_offset)
{
	acpi_status status = AE_OK;
	u64 merged_value;
	u64 current_value;

	ACPI_FUNCTION_TRACE_U32(ex_write_with_update_rule, mask);

	/* Start with the new bits  */

	merged_value = field_value;

	/* If the mask is all ones, we don't need to worry about the update rule */

	if (mask != ACPI_UINT64_MAX) {

		/* Decode the update rule */

		switch (obj_desc->common_field.
			field_flags & AML_FIELD_UPDATE_RULE_MASK) {
		case AML_FIELD_UPDATE_PRESERVE:
			/*
			 * Check if update rule needs to be applied (not if mask is all
			 * ones)  The left shift drops the bits we want to ignore.
			 */
			if ((~mask << (ACPI_MUL_8(sizeof(mask)) -
				       ACPI_MUL_8(obj_desc->common_field.
						  access_byte_width))) != 0) {
				/*
				 * Read the current contents of the byte/word/dword containing
				 * the field, and merge with the new field value.
				 */
				status =
				    acpi_ex_field_datum_io(obj_desc,
							   field_datum_byte_offset,
							   &current_value,
							   ACPI_READ);
				if (ACPI_FAILURE(status)) {
					return_ACPI_STATUS(status);
				}

				merged_value |= (current_value & ~mask);
			}
			break;

		case AML_FIELD_UPDATE_WRITE_AS_ONES:

			/* Set positions outside the field to all ones */

			merged_value |= ~mask;
			break;

		case AML_FIELD_UPDATE_WRITE_AS_ZEROS:

			/* Set positions outside the field to all zeros */

			merged_value &= mask;
			break;

		default:

			ACPI_ERROR((AE_INFO,
				    "Unknown UpdateRule value: 0x%X",
				    (obj_desc->common_field.field_flags &
				     AML_FIELD_UPDATE_RULE_MASK)));
			return_ACPI_STATUS(AE_AML_OPERAND_VALUE);
		}
	}

	ACPI_DEBUG_PRINT((ACPI_DB_BFIELD,
			  "Mask %8.8X%8.8X, DatumOffset %X, Width %X, "
			  "Value %8.8X%8.8X, MergedValue %8.8X%8.8X\n",
			  ACPI_FORMAT_UINT64(mask),
			  field_datum_byte_offset,
			  obj_desc->common_field.access_byte_width,
			  ACPI_FORMAT_UINT64(field_value),
			  ACPI_FORMAT_UINT64(merged_value)));

	/* Write the merged value */

	status =
	    acpi_ex_field_datum_io(obj_desc, field_datum_byte_offset,
				   &merged_value, ACPI_WRITE);

	return_ACPI_STATUS(status);
}


// Source: exfldio.c
// Lines 544-631
