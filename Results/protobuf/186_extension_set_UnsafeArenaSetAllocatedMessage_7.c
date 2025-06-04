void ExtensionSet::UnsafeArenaSetAllocatedMessage(
    int number, FieldType type, const FieldDescriptor* descriptor,
    MessageLite* message) {
  if (message == nullptr) {
    ClearExtension(number);
    return;
  }
  Extension* extension;
  if (MaybeNewExtension(number, descriptor, &extension)) {
    extension->type = type;
    GOOGLE_DCHECK_EQ(cpp_type(extension->type), WireFormatLite::CPPTYPE_MESSAGE);
    extension->is_repeated = false;
    extension->is_lazy = false;
    extension->message_value = message;
  } else {
    GOOGLE_DCHECK_TYPE(*extension, OPTIONAL_FIELD, MESSAGE);
    if (extension->is_lazy) {
      extension->lazymessage_value->UnsafeArenaSetAllocatedMessage(message,
                                                                   arena_);
    } else {
      if (arena_ == nullptr) {
        delete extension->message_value;
      }
      extension->message_value = message;
    }
  }
  extension->is_cleared = false;
}


// Source: extension_set.cc
// Lines 718-745
