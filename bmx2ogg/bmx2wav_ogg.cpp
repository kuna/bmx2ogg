#include "bmx2wav_ogg.h"
using namespace Bmx2Wav;

#include "vorbis\vorbisenc.h"
#include "bmx2wav_common.h"
#include "exception.h"
#include <time.h>

// how much we'll going to read/process for chunk/
#define OGG_CHUNKSIZE 1024

HQOgg::HQOgg() {
	// empty sound, basically
	this->wav = new HQWav();
}

HQOgg::HQOgg(HQWav *wav) {
	this->wav = new HQWav(*wav);
}

HQOgg::~HQOgg() {
	Release();
}

HQWav* HQOgg::GetPtr()
{
	return wav;
}

void HQOgg::Release() {
	delete wav;
}

void HQOgg::SetPtr(HQWav* wav)
{
	this->wav = wav;
}

void HQOgg::SetMetadata(const std::wstring& title, const std::wstring& artist)
{
	this->title = title;
	this->artist = artist;
}

void HQOgg::WriteToFile(const std::wstring& path) {
	// make raw data first ...
	std::vector<char> raw_data;
	wav->WriteToBuffer(raw_data);

	// open fp...
	FILE *f;
	if (_wfopen_s(&f, path.c_str(), L"wb+") != 0)
		throw Bmx2WavInvalidFile(path, errno);


	// from: https://svn.xiph.org/trunk/vorbis/examples/encoder_example.c

	ogg_stream_state os; /* take physical pages, weld into a logical
						 stream of packets */
	ogg_page         og; /* one Ogg bitstream page.  Vorbis packets are inside */
	ogg_packet       op; /* one raw packet of data for decode */

	vorbis_info      vi; /* struct that stores all the static vorbis bitstream
						 settings */
	vorbis_comment   vc; /* struct that stores all the user comments */

	vorbis_dsp_state vd; /* central working state for the packet->PCM decoder */
	vorbis_block     vb; /* local working space for packet->PCM decode */

	/* encoding setup */
	vorbis_info_init(&vi);

	/* choose an encoding mode.  A few possibilities commented out, one
	actually used: */

	/*********************************************************************
	Encoding using a VBR quality mode.  The usable range is -.1
	(lowest quality, smallest file) to 1. (highest quality, largest file).
	Example quality mode .4: 44kHz stereo coupled, roughly 128kbps VBR

	ret = vorbis_encode_init_vbr(&vi,2,44100,.4);

	---------------------------------------------------------------------

	Encoding using an average bitrate mode (ABR).
	example: 44kHz stereo coupled, average 128kbps VBR

	ret = vorbis_encode_init(&vi,2,44100,-1,128000,-1);

	---------------------------------------------------------------------

	Encode using a quality mode, but select that quality mode by asking for
	an approximate bitrate.  This is not ABR, it is true VBR, but selected
	using the bitrate interface, and then turning bitrate management off:

	ret = ( vorbis_encode_setup_managed(&vi,2,44100,-1,128000,-1) ||
	vorbis_encode_ctl(&vi,OV_ECTL_RATEMANAGE2_SET,NULL) ||
	vorbis_encode_setup_init(&vi));

	*********************************************************************/

	int ret = vorbis_encode_init_vbr(&vi, 2, 44100, 0.95);
	int eos = 0;	// end of stream

	/* do not continue if setup failed; this can happen if we ask for a
	mode that libVorbis does not support (eg, too low a bitrate, etc,
	will return 'OV_EIMPL') */

	if (ret)exit(1);

	/* add a comment */
	vorbis_comment_init(&vc);
	char utf8_buf[256];
	vorbis_comment_add_tag(&vc, "ENCODER", "Bmx2Ogg");
	if (ENCODING::wchar_to_utf8(artist.c_str(), utf8_buf, 256))
		vorbis_comment_add_tag(&vc, "ARTIST", utf8_buf);
	if (ENCODING::wchar_to_utf8(title.c_str(), utf8_buf, 256))
		vorbis_comment_add_tag(&vc, "TITLE", utf8_buf);

	/* set up the analysis state and auxiliary encoding storage */
	vorbis_analysis_init(&vd, &vi);
	vorbis_block_init(&vd, &vb);

	/* set up our packet->stream encoder */
	/* pick a random serial number; that way we can more likely build
	chained streams just by concatenation */
	srand(time(NULL));
	ogg_stream_init(&os, rand());

	/* Vorbis streams begin with three headers; the initial header (with
	most of the codec setup parameters) which is mandated by the Ogg
	bitstream spec.  The second header holds any comment fields.  The
	third header holds the bitstream codebook.  We merely need to
	make the headers, then pass them to libvorbis one at a time;
	libvorbis handles the additional Ogg bitstream constraints */

	{
		ogg_packet header;
		ogg_packet header_comm;
		ogg_packet header_code;

		vorbis_analysis_headerout(&vd, &vc, &header, &header_comm, &header_code);
		ogg_stream_packetin(&os, &header); /* automatically placed in its own
										   page */
		ogg_stream_packetin(&os, &header_comm);
		ogg_stream_packetin(&os, &header_code);

		/* This ensures the actual
		* audio data will start on a new page, as per spec
		*/
		while (!eos){
			int result = ogg_stream_flush(&os, &og);
			if (result == 0)break;
			fwrite(og.header, 1, og.header_len, f);
			fwrite(og.body, 1, og.body_len, f);
		}

	}

	long chunks = raw_data.size()/4;
	long cur_read, cur_pos = 0;

	while (!eos) {
		cur_read = chunks - cur_pos;
		if (cur_read > OGG_CHUNKSIZE) cur_read = OGG_CHUNKSIZE;

		if (cur_read > 0) {
			/* data to encode */
			/* expose the buffer to submit data */
			float **buffer = vorbis_analysis_buffer(&vd, OGG_CHUNKSIZE);

			/* uninterleave samples */
			for (long i = 0; i<cur_read; i++){
				buffer[0][i] = ((raw_data[cur_pos * 4 + i * 4 + 1] << 8) |
					(0x00ff & (int)raw_data[cur_pos * 4 + i * 4])) / 32768.f;
				buffer[1][i] = ((raw_data[cur_pos * 4 + i * 4 + 3] << 8) |
					(0x00ff & (int)raw_data[cur_pos * 4 + i * 4 + 2])) / 32768.f;
			}
		}
		/* tell the library how much we actually submitted */
		vorbis_analysis_wrote(&vd, cur_read);
		cur_pos += cur_read;

		/* vorbis does some data preanalysis, then divvies up blocks for
		more involved (potentially parallel) processing.  Get a single
		block for encoding now */
		while (vorbis_analysis_blockout(&vd, &vb) == 1){

			/* analysis, assume we want to use bitrate management */
			vorbis_analysis(&vb, NULL);
			vorbis_bitrate_addblock(&vb);

			while (vorbis_bitrate_flushpacket(&vd, &op)){

				/* weld the packet into the bitstream */
				ogg_stream_packetin(&os, &op);

				/* write out pages (if any) */
				while (!eos){
					int result = ogg_stream_pageout(&os, &og);
					if (result == 0)break;
					fwrite(og.header, 1, og.header_len, f);
					fwrite(og.body, 1, og.body_len, f);

					/* this could be set above, but for illustrative purposes, I do
					it here (to show that vorbis does know where the stream ends) */

					if (ogg_page_eos(&og))eos = 1;
				}
			}
		}
	}

	/* clean up and exit.  vorbis_info_clear() must be called last */

	ogg_stream_clear(&os);
	vorbis_block_clear(&vb);
	vorbis_dsp_clear(&vd);
	vorbis_comment_clear(&vc);
	vorbis_info_clear(&vi);

	/* ogg_page and ogg_packet structs always point to storage in
	libvorbis.  They're never freed or manipulated directly */

	fclose(f);
}