void ZeroFieldsBase::MergeImpl(Message* to_param, const Message& from_param) {
  auto* to = static_cast<ZeroFieldsBase*>(to_param);
  const auto* from = static_cast<const ZeroFieldsBase*>(&from_param);
  GOOGLE_DCHECK_NE(from, to);
  to->_internal_metadata_.MergeFrom<UnknownFieldSet>(from->_internal_metadata_);
}


// Source: generated_message_bases.cc
// Lines 101-106
