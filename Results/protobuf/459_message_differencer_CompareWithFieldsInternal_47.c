bool MessageDifferencer::CompareWithFieldsInternal(
    const Message& message1, const Message& message2,
    const FieldDescriptorArray& message1_fields,
    const FieldDescriptorArray& message2_fields,
    std::vector<SpecificField>* parent_fields) {
  bool isDifferent = false;
  int field_index1 = 0;
  int field_index2 = 0;

  const Reflection* reflection1 = message1.GetReflection();
  const Reflection* reflection2 = message2.GetReflection();

  while (true) {
    const FieldDescriptor* field1 = message1_fields[field_index1];
    const FieldDescriptor* field2 = message2_fields[field_index2];

    // Once we have reached sentinel values, we are done the comparison.
    if (field1 == NULL && field2 == NULL) {
      break;
    }

    // Check for differences in the field itself.
    if (FieldBefore(field1, field2)) {
      // Field 1 is not in the field list for message 2.
      if (IsIgnored(message1, message2, field1, *parent_fields)) {
        // We are ignoring field1. Report the ignore and move on to
        // the next field in message1_fields.
        if (reporter_ != NULL) {
          SpecificField specific_field;
          specific_field.field = field1;
          parent_fields->push_back(specific_field);
          if (report_ignores_) {
            reporter_->ReportIgnored(message1, message2, *parent_fields);
          }
          parent_fields->pop_back();
        }
        ++field_index1;
        continue;
      }

      if (reporter_ != NULL) {
        assert(field1 != NULL);
        int count = field1->is_repeated()
                        ? reflection1->FieldSize(message1, field1)
                        : 1;

        for (int i = 0; i < count; ++i) {
          SpecificField specific_field;
          specific_field.field = field1;
          if (field1->is_repeated()) {
            AddSpecificIndex(&specific_field, message1, field1, i);
          } else {
            specific_field.index = -1;
          }

          parent_fields->push_back(specific_field);
          reporter_->ReportDeleted(message1, message2, *parent_fields);
          parent_fields->pop_back();
        }

        isDifferent = true;
      } else {
        return false;
      }

      ++field_index1;
      continue;
    } else if (FieldBefore(field2, field1)) {
      // Field 2 is not in the field list for message 1.
      if (IsIgnored(message1, message2, field2, *parent_fields)) {
        // We are ignoring field2. Report the ignore and move on to
        // the next field in message2_fields.
        if (reporter_ != NULL) {
          SpecificField specific_field;
          specific_field.field = field2;
          parent_fields->push_back(specific_field);
          if (report_ignores_) {
            reporter_->ReportIgnored(message1, message2, *parent_fields);
          }
          parent_fields->pop_back();
        }
        ++field_index2;
        continue;
      }

      if (reporter_ != NULL) {
        int count = field2->is_repeated()
                        ? reflection2->FieldSize(message2, field2)
                        : 1;

        for (int i = 0; i < count; ++i) {
          SpecificField specific_field;
          specific_field.field = field2;
          if (field2->is_repeated()) {
            specific_field.index = i;
            AddSpecificNewIndex(&specific_field, message2, field2, i);
          } else {
            specific_field.index = -1;
            specific_field.new_index = -1;
          }

          parent_fields->push_back(specific_field);
          reporter_->ReportAdded(message1, message2, *parent_fields);
          parent_fields->pop_back();
        }


// Source: message_differencer.cc
// Lines 763-867
