MessageLite* ExtensionSet::ReleaseMessage(const FieldDescriptor* descriptor,
                                          MessageFactory* factory) {
  Extension* extension = FindOrNull(descriptor->number());
  if (extension == nullptr) {
    // Not present.  Return nullptr.
    return nullptr;
  } else {
    GOOGLE_DCHECK_TYPE(*extension, OPTIONAL, MESSAGE);
    MessageLite* ret = nullptr;
    if (extension->is_lazy) {
      ret = extension->lazymessage_value->ReleaseMessage(
          *factory->GetPrototype(descriptor->message_type()), arena_);
      if (arena_ == nullptr) {
        delete extension->lazymessage_value;
      }
    } else {
      if (arena_ != nullptr) {
        ret = extension->message_value->New();
        ret->CheckTypeAndMergeFrom(*extension->message_value);
      } else {
        ret = extension->message_value;
      }
    }
    Erase(descriptor->number());
    return ret;
  }


// Source: extension_set_heavy.cc
// Lines 171-196
