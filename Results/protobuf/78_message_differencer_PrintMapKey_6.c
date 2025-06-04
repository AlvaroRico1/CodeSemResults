void MessageDifferencer::StreamReporter::PrintMapKey(
    bool left_side, const SpecificField& specific_field) {
  if (message1_ == nullptr || message2_ == nullptr) {
    GOOGLE_LOG(INFO) << "PrintPath cannot log map keys; "
                 "use SetMessages to provide the messages "
                 "being compared prior to any processing.";
    return;
  }

  const Message* found_message =
      left_side ? specific_field.map_entry1 : specific_field.map_entry2;
  std::string key_string = "";
  if (found_message != nullptr) {
    // NB: the map key is always the first field
    const FieldDescriptor* fd = found_message->GetDescriptor()->field(0);
    if (fd->cpp_type() == FieldDescriptor::CPPTYPE_STRING) {
      // Not using PrintFieldValueToString for strings to avoid extra
      // characters
      key_string = found_message->GetReflection()->GetString(
          *found_message, found_message->GetDescriptor()->field(0));
    } else {
      TextFormat::PrintFieldValueToString(*found_message, fd, -1, &key_string);
    }
    if (key_string.empty()) {
      key_string = "''";
    }
    printer_->PrintRaw(StrCat("[", key_string, "]"));
  }
}


// Source: message_differencer.cc
// Lines 2103-2131
