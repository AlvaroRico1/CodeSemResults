void DescriptorBuilder::BuildFieldOrExtension(const FieldDescriptorProto& proto,
                                              Descriptor* parent,
                                              FieldDescriptor* result,
                                              bool is_extension,
                                              internal::FlatAllocator& alloc) {
  const std::string& scope =
      (parent == nullptr) ? file_->package() : parent->full_name();

  // We allocate all names in a single array, and dedup them.
  // We remember the indices for the potentially deduped values.
  auto all_names = alloc.AllocateFieldNames(
      proto.name(), scope,
      proto.has_json_name() ? &proto.json_name() : nullptr);
  result->all_names_ = all_names.array;
  result->lowercase_name_index_ = all_names.lowercase_index;
  result->camelcase_name_index_ = all_names.camelcase_index;
  result->json_name_index_ = all_names.json_index;

  ValidateSymbolName(proto.name(), result->full_name(), proto);

  result->file_ = file_;
  result->number_ = proto.number();
  result->is_extension_ = is_extension;
  result->is_oneof_ = false;
  result->proto3_optional_ = proto.proto3_optional();

  if (proto.proto3_optional() &&
      file_->syntax() != FileDescriptor::SYNTAX_PROTO3) {
    AddError(result->full_name(), proto, DescriptorPool::ErrorCollector::TYPE,
             "The [proto3_optional=true] option may only be set on proto3"
             "fields, not " +
                 result->full_name());
  }

  result->has_json_name_ = proto.has_json_name();

  // Some compilers do not allow static_cast directly between two enum types,
  // so we must cast to int first.
  result->type_ = static_cast<FieldDescriptor::Type>(
      implicit_cast<int>(proto.type()));
  result->label_ = static_cast<FieldDescriptor::Label>(
      implicit_cast<int>(proto.label()));

  if (result->label_ == FieldDescriptor::LABEL_REQUIRED) {
    // An extension cannot have a required field (b/13365836).
    if (result->is_extension_) {
      AddError(result->full_name(), proto,
               // Error location `TYPE`: we would really like to indicate
               // `LABEL`, but the `ErrorLocation` enum has no entry for this,
               // and we don't necessarily know about all implementations of the
               // `ErrorCollector` interface to extend them to handle the new
               // error location type properly.
               DescriptorPool::ErrorCollector::TYPE,
               "The extension " + result->full_name() + " cannot be required.");
    }
  }

  // Some of these may be filled in when cross-linking.
  result->containing_type_ = nullptr;
  result->type_once_ = nullptr;
  result->default_value_enum_ = nullptr;

  result->has_default_value_ = proto.has_default_value();
  if (proto.has_default_value() && result->is_repeated()) {
    AddError(result->full_name(), proto,
             DescriptorPool::ErrorCollector::DEFAULT_VALUE,
             "Repeated fields can't have default values.");
  }

  if (proto.has_type()) {
    if (proto.has_default_value()) {
      char* end_pos = nullptr;
      switch (result->cpp_type()) {
        case FieldDescriptor::CPPTYPE_INT32:
          result->default_value_int32_t_ =
              strtol(proto.default_value().c_str(), &end_pos, 0);
          break;
        case FieldDescriptor::CPPTYPE_INT64:
          result->default_value_int64_t_ =
              strto64(proto.default_value().c_str(), &end_pos, 0);
          break;
        case FieldDescriptor::CPPTYPE_UINT32:
          result->default_value_uint32_t_ =
              strtoul(proto.default_value().c_str(), &end_pos, 0);
          break;
        case FieldDescriptor::CPPTYPE_UINT64:
          result->default_value_uint64_t_ =
              strtou64(proto.default_value().c_str(), &end_pos, 0);
          break;
        case FieldDescriptor::CPPTYPE_FLOAT:
          if (proto.default_value() == "inf") {
            result->default_value_float_ =
                std::numeric_limits<float>::infinity();
          } else if (proto.default_value() == "-inf") {
            result->default_value_float_ =
                -std::numeric_limits<float>::infinity();
          } else if (proto.default_value() == "nan") {
            result->default_value_float_ =
                std::numeric_limits<float>::quiet_NaN();
          } else {
            result->default_value_float_ = io::SafeDoubleToFloat(
                io::NoLocaleStrtod(proto.default_value().c_str(), &end_pos));
          }
          break;
        case FieldDescriptor::CPPTYPE_DOUBLE:
          if (proto.default_value() == "inf") {
            result->default_value_double_ =
                std::numeric_limits<double>::infinity();
          } else if (proto.default_value() == "-inf") {
            result->default_value_double_ =
                -std::numeric_limits<double>::infinity();
          } else if (proto.default_value() == "nan") {
            result->default_value_double_ =
                std::numeric_limits<double>::quiet_NaN();
          } else {
            result->default_value_double_ =
                io::NoLocaleStrtod(proto.default_value().c_str(), &end_pos);
          }
          break;
        case FieldDescriptor::CPPTYPE_BOOL:
          if (proto.default_value() == "true") {
            result->default_value_bool_ = true;
          } else if (proto.default_value() == "false") {
            result->default_value_bool_ = false;
          } else {
            AddError(result->full_name(), proto,
                     DescriptorPool::ErrorCollector::DEFAULT_VALUE,
                     "Boolean default must be true or false.");
          }
          break;
        case FieldDescriptor::CPPTYPE_ENUM:
          // This will be filled in when cross-linking.
          result->default_value_enum_ = nullptr;
          break;
        case FieldDescriptor::CPPTYPE_STRING:
          if (result->type() == FieldDescriptor::TYPE_BYTES) {
            result->default_value_string_ = alloc.AllocateStrings(
                UnescapeCEscapeString(proto.default_value()));
          } else {
            result->default_value_string_ =
                alloc.AllocateStrings(proto.default_value());
          }
          break;
        case FieldDescriptor::CPPTYPE_MESSAGE:
          AddError(result->full_name(), proto,
                   DescriptorPool::ErrorCollector::DEFAULT_VALUE,
                   "Messages can't have default values.");
          result->has_default_value_ = false;
          result->default_generated_instance_ = nullptr;
          break;
      }

      if (end_pos != nullptr) {
        // end_pos is only set non-null by the parsers for numeric types,
        // above. This checks that the default was non-empty and had no extra
        // junk after the end of the number.
        if (proto.default_value().empty() || *end_pos != '\0') {
          AddError(result->full_name(), proto,
                   DescriptorPool::ErrorCollector::DEFAULT_VALUE,
                   "Couldn't parse default value \"" + proto.default_value() +
                       "\".");
        }
      }
    } else {
      // No explicit default value
      switch (result->cpp_type()) {
        case FieldDescriptor::CPPTYPE_INT32:
          result->default_value_int32_t_ = 0;
          break;
        case FieldDescriptor::CPPTYPE_INT64:
          result->default_value_int64_t_ = 0;
          break;
        case FieldDescriptor::CPPTYPE_UINT32:
          result->default_value_uint32_t_ = 0;
          break;
        case FieldDescriptor::CPPTYPE_UINT64:
          result->default_value_uint64_t_ = 0;
          break;
        case FieldDescriptor::CPPTYPE_FLOAT:
          result->default_value_float_ = 0.0f;
          break;
        case FieldDescriptor::CPPTYPE_DOUBLE:
          result->default_value_double_ = 0.0;
          break;
        case FieldDescriptor::CPPTYPE_BOOL:
          result->default_value_bool_ = false;
          break;
        case FieldDescriptor::CPPTYPE_ENUM:
          // This will be filled in when cross-linking.
          result->default_value_enum_ = nullptr;
          break;
        case FieldDescriptor::CPPTYPE_STRING:
          result->default_value_string_ = &internal::GetEmptyString();
          break;
        case FieldDescriptor::CPPTYPE_MESSAGE:
          result->default_generated_instance_ = nullptr;
          break;
      }
    }
  }

  if (result->number() <= 0) {
    AddError(result->full_name(), proto, DescriptorPool::ErrorCollector::NUMBER,
             "Field numbers must be positive integers.");
  } else if (!is_extension && result->number() > FieldDescriptor::kMaxNumber) {
    // Only validate that the number is within the valid field range if it is
    // not an extension. Since extension numbers are validated with the
    // extendee's valid set of extension numbers, and those are in turn
    // validated against the max allowed number, the check is unnecessary for
    // extension fields.
    // This avoids cross-linking issues that arise when attempting to check if
    // the extendee is a message_set_wire_format message, which has a higher max
    // on extension numbers.
    AddError(result->full_name(), proto, DescriptorPool::ErrorCollector::NUMBER,
             strings::Substitute("Field numbers cannot be greater than $0.",
                              FieldDescriptor::kMaxNumber));
  } else if (result->number() >= FieldDescriptor::kFirstReservedNumber &&
             result->number() <= FieldDescriptor::kLastReservedNumber) {
    AddError(result->full_name(), proto, DescriptorPool::ErrorCollector::NUMBER,
             strings::Substitute(
                 "Field numbers $0 through $1 are reserved for the protocol "
                 "buffer library implementation.",
                 FieldDescriptor::kFirstReservedNumber,
                 FieldDescriptor::kLastReservedNumber));
  }

  if (is_extension) {
    if (!proto.has_extendee()) {
      AddError(result->full_name(), proto,
               DescriptorPool::ErrorCollector::EXTENDEE,
               "FieldDescriptorProto.extendee not set for extension field.");
    }

    result->scope_.extension_scope = parent;

    if (proto.has_oneof_index()) {
      AddError(result->full_name(), proto, DescriptorPool::ErrorCollector::TYPE,
               "FieldDescriptorProto.oneof_index should not be set for "
               "extensions.");
    }
  } else {
    if (proto.has_extendee()) {
      AddError(result->full_name(), proto,
               DescriptorPool::ErrorCollector::EXTENDEE,
               "FieldDescriptorProto.extendee set for non-extension field.");
    }

    result->containing_type_ = parent;

    if (proto.has_oneof_index()) {
      if (proto.oneof_index() < 0 ||
          proto.oneof_index() >= parent->oneof_decl_count()) {
        AddError(result->full_name(), proto,
                 DescriptorPool::ErrorCollector::TYPE,
                 strings::Substitute("FieldDescriptorProto.oneof_index $0 is "
                                  "out of range for type \"$1\".",
                                  proto.oneof_index(), parent->name()));
      } else {
        result->is_oneof_ = true;
        result->scope_.containing_oneof =
            parent->oneof_decl(proto.oneof_index());
      }
    }
  }

  // Copy options.
  result->options_ = nullptr;  // Set to default_instance later if necessary.
  if (proto.has_options()) {
    AllocateOptions(proto.options(), result,
                    FieldDescriptorProto::kOptionsFieldNumber,
                    "google.protobuf.FieldOptions", alloc);
  }

  AddSymbol(result->full_name(), parent, result->name(), proto, Symbol(result));
}


// Source: descriptor.cc
// Lines 5498-5772
