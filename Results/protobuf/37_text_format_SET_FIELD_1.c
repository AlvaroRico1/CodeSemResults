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


// Source: text_format.cc
// Lines 753-780
