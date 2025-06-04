bool WireFormatLite::SkipField(io::CodedInputStream* input, uint32_t tag,
                               io::CodedOutputStream* output) {
  // Field number 0 is illegal.
  if (WireFormatLite::GetTagFieldNumber(tag) == 0) return false;
  switch (WireFormatLite::GetTagWireType(tag)) {
    case WireFormatLite::WIRETYPE_VARINT: {
      uint64_t value;
      if (!input->ReadVarint64(&value)) return false;
      output->WriteVarint32(tag);
      output->WriteVarint64(value);
      return true;
    }
    case WireFormatLite::WIRETYPE_FIXED64: {
      uint64_t value;
      if (!input->ReadLittleEndian64(&value)) return false;
      output->WriteVarint32(tag);
      output->WriteLittleEndian64(value);
      return true;
    }
    case WireFormatLite::WIRETYPE_LENGTH_DELIMITED: {
      uint32_t length;
      if (!input->ReadVarint32(&length)) return false;
      output->WriteVarint32(tag);
      output->WriteVarint32(length);
      // TODO(mkilavuz): Provide API to prevent extra string copying.
      std::string temp;
      if (!input->ReadString(&temp, length)) return false;
      output->WriteString(temp);
      return true;
    }
    case WireFormatLite::WIRETYPE_START_GROUP: {
      output->WriteVarint32(tag);
      if (!input->IncrementRecursionDepth()) return false;
      if (!SkipMessage(input, output)) return false;
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
      output->WriteVarint32(tag);
      output->WriteLittleEndian32(value);
      return true;
    }


// Source: wire_format_lite.cc
// Lines 171-223
