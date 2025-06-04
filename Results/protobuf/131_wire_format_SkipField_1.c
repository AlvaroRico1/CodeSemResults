bool WireFormat::SkipField(io::CodedInputStream* input, uint32_t tag,
                           UnknownFieldSet* unknown_fields) {
  int number = WireFormatLite::GetTagFieldNumber(tag);
  // Field number 0 is illegal.
  if (number == 0) return false;

  switch (WireFormatLite::GetTagWireType(tag)) {
    case WireFormatLite::WIRETYPE_VARINT: {
      uint64_t value;
      if (!input->ReadVarint64(&value)) return false;
      if (unknown_fields != nullptr) unknown_fields->AddVarint(number, value);
      return true;
    }
    case WireFormatLite::WIRETYPE_FIXED64: {
      uint64_t value;
      if (!input->ReadLittleEndian64(&value)) return false;
      if (unknown_fields != nullptr) unknown_fields->AddFixed64(number, value);
      return true;
    }
    case WireFormatLite::WIRETYPE_LENGTH_DELIMITED: {
      uint32_t length;
      if (!input->ReadVarint32(&length)) return false;
      if (unknown_fields == nullptr) {
        if (!input->Skip(length)) return false;
      } else {
        if (!input->ReadString(unknown_fields->AddLengthDelimited(number),
                               length)) {
          return false;
        }
      }
      return true;
    }
    case WireFormatLite::WIRETYPE_START_GROUP: {
      if (!input->IncrementRecursionDepth()) return false;
      if (!SkipMessage(input, (unknown_fields == nullptr)
                                  ? nullptr
                                  : unknown_fields->AddGroup(number))) {
        return false;
      }
      input->DecrementRecursionDepth();
      // Check that the ending tag matched the starting tag.
      if (!input->LastTagWas(
              WireFormatLite::MakeTag(WireFormatLite::GetTagFieldNumber(tag),
                                      WireFormatLite::WIRETYPE_END_GROUP))) {
        return false;
      }
      return true;
    }
    case WireFormatLite::WIRETYPE_END_GROUP: {
      return false;
    }
    case WireFormatLite::WIRETYPE_FIXED32: {
      uint32_t value;
      if (!input->ReadLittleEndian32(&value)) return false;
      if (unknown_fields != nullptr) unknown_fields->AddFixed32(number, value);
      return true;
    }


// Source: wire_format.cc
// Lines 85-141
