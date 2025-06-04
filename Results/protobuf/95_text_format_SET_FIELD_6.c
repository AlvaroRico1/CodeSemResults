#define SET_FIELD(CPPTYPE, VALUE)                    \
  if (field->is_repeated()) {                        \
    reflection->Add##CPPTYPE(message, field, VALUE); \
  } else {                                           \
    reflection->Set##CPPTYPE(message, field, VALUE); \
  }

    switch (field->cpp_type()) {
      case FieldDescriptor::CPPTYPE_INT32: {
        int64_t value;
        DO(ConsumeSignedInteger(&value, kint32max));
        SET_FIELD(Int32, static_cast<int32_t>(value));
        break;
      }

      case FieldDescriptor::CPPTYPE_UINT32: {
        uint64_t value;
        DO(ConsumeUnsignedInteger(&value, kuint32max));
        SET_FIELD(UInt32, static_cast<uint32_t>(value));
        break;
      }

      case FieldDescriptor::CPPTYPE_INT64: {
        int64_t value;
        DO(ConsumeSignedInteger(&value, kint64max));
        SET_FIELD(Int64, value);
        break;
      }

      case FieldDescriptor::CPPTYPE_UINT64: {
        uint64_t value;
        DO(ConsumeUnsignedInteger(&value, kuint64max));
        SET_FIELD(UInt64, value);
        break;
      }

      case FieldDescriptor::CPPTYPE_FLOAT: {
        double value;
        DO(ConsumeDouble(&value));
        SET_FIELD(Float, io::SafeDoubleToFloat(value));
        break;
      }

      case FieldDescriptor::CPPTYPE_DOUBLE: {
        double value;
        DO(ConsumeDouble(&value));
        SET_FIELD(Double, value);
        break;
      }

      case FieldDescriptor::CPPTYPE_STRING: {
        std::string value;
        DO(ConsumeString(&value));
        SET_FIELD(String, value);
        break;
      }

      case FieldDescriptor::CPPTYPE_BOOL: {
        if (LookingAtType(io::Tokenizer::TYPE_INTEGER)) {
          uint64_t value;
          DO(ConsumeUnsignedInteger(&value, 1));
          SET_FIELD(Bool, value);
        } else {
          std::string value;
          DO(ConsumeIdentifier(&value));
          if (value == "true" || value == "True" || value == "t") {
            SET_FIELD(Bool, true);
          } else if (value == "false" || value == "False" || value == "f") {
            SET_FIELD(Bool, false);
          } else {
            ReportError("Invalid value for boolean field \"" + field->name() +
                        "\". Value: \"" + value + "\".");
            return false;
          }
        }


// Source: text_format.cc
// Lines 753-827
