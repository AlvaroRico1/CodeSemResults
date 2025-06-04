static acpi_status acpi_ex_name_segment(u8 ** in_aml_address, char *name_string)
{
	char *aml_address = (void *)*in_aml_address;
	acpi_status status = AE_OK;
	u32 index;
	char char_buf[5];

	ACPI_FUNCTION_TRACE(ex_name_segment);

	/*
	 * If first character is a digit, then we know that we aren't looking
	 * at a valid name segment
	 */
	char_buf[0] = *aml_address;

	if ('0' <= char_buf[0] && char_buf[0] <= '9') {
		ACPI_ERROR((AE_INFO, "Invalid leading digit: %c", char_buf[0]));
		return_ACPI_STATUS(AE_CTRL_PENDING);
	}

	for (index = 0;
	     (index < ACPI_NAMESEG_SIZE)
	     && (acpi_ut_valid_name_char(*aml_address, 0)); index++) {
		char_buf[index] = *aml_address++;
	}

	/* Valid name segment  */

	if (index == 4) {

		/* Found 4 valid characters */

		char_buf[4] = '\0';

		if (name_string) {
			ACPI_DEBUG_PRINT((ACPI_DB_NAMES,
					  "Appending NameSeg %s\n", char_buf));
			strcat(name_string, char_buf);
		} else {
			ACPI_DEBUG_PRINT((ACPI_DB_NAMES,
					  "No Name string - %s\n", char_buf));
		}
	} else if (index == 0) {
		/*
		 * First character was not a valid name character,
		 * so we are looking at something other than a name.
		 */
		ACPI_DEBUG_PRINT((ACPI_DB_INFO,
				  "Leading character is not alpha: %02Xh (not a name)\n",
				  char_buf[0]));
		status = AE_CTRL_PENDING;
	} else {
		/*
		 * Segment started with one or more valid characters, but fewer than
		 * the required 4
		 */
		status = AE_AML_BAD_NAME;
		ACPI_ERROR((AE_INFO,
			    "Bad character 0x%02x in name, at %p",
			    *aml_address, aml_address));
	}

	*in_aml_address = ACPI_CAST_PTR(u8, aml_address);
	return_ACPI_STATUS(status);
}


// Source: exnames.c
// Lines 123-187
