ssize_t hdmi_drm_infoframe_pack_only(const struct hdmi_drm_infoframe *frame,
				     void *buffer, size_t size)
{
	u8 *ptr = buffer;
	size_t length;
	int i;

	length = HDMI_INFOFRAME_HEADER_SIZE + frame->length;

	if (size < length)
		return -ENOSPC;

	memset(buffer, 0, size);

	ptr[0] = frame->type;
	ptr[1] = frame->version;
	ptr[2] = frame->length;
	ptr[3] = 0; /* checksum */

	/* start infoframe payload */
	ptr += HDMI_INFOFRAME_HEADER_SIZE;

	*ptr++ = frame->eotf;
	*ptr++ = frame->metadata_type;

	for (i = 0; i < 3; i++) {
		*ptr++ = frame->display_primaries[i].x;
		*ptr++ = frame->display_primaries[i].x >> 8;
		*ptr++ = frame->display_primaries[i].y;
		*ptr++ = frame->display_primaries[i].y >> 8;
	}

	*ptr++ = frame->white_point.x;
	*ptr++ = frame->white_point.x >> 8;

	*ptr++ = frame->white_point.y;
	*ptr++ = frame->white_point.y >> 8;

	*ptr++ = frame->max_display_mastering_luminance;
	*ptr++ = frame->max_display_mastering_luminance >> 8;

	*ptr++ = frame->min_display_mastering_luminance;
	*ptr++ = frame->min_display_mastering_luminance >> 8;

	*ptr++ = frame->max_cll;
	*ptr++ = frame->max_cll >> 8;

	*ptr++ = frame->max_fall;
	*ptr++ = frame->max_fall >> 8;

	hdmi_infoframe_set_checksum(buffer, length);

	return length;
}


// Source: hdmi.c
// Lines 711-764
