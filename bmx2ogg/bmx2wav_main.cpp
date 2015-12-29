// bmx2wav_remake.cpp : Defines the entry point for the console application.
//

#include "targetver.h"
#include "bmsbel\bms_bms.h"
#include "bmsbel\bms_parser.h"
#include "bmx2wav_common.h"
#include "bms_resource.h"
#include "exception.h"
#include <tchar.h>
#include <wchar.h>
#include <string>

#include "bmx2wav_wav_maker.h"
#include "bmx2wav_ogg.h"
using namespace Bmx2Wav;

#define NOT(v) (!(v))
#define CMP(a, b) (wcscmp((a), (b)) == 0)

namespace BMX2WAVParameter {
	enum OUTPUT_TYPE { OUTPUT_OGG, OUTPUT_WAV } output_type;
	std::wstring bms_path;
	std::wstring output_path;
	bool overwrite;
	bool autofilename;

	std::wstring substitute_output_extension(const std::wstring& filename) {
		if (output_type == OUTPUT_OGG) {
			return IO::substitute_extension(filename, L".ogg");
		}
		else {
			return IO::substitute_extension(filename, L".wav");
		}
	}

	void help() {
		printf("input args: (bmx path) -(option) (output path)\n"
			"available option:\n"
			"-oc: output file to current directory (default)\n"
			"-ob: output file to bms directory\n"
			"-o: output file to my custom path (you need to enter more argument)\n"
			"-wav: output audio as wav\n"
			"-ogg: output audio as ogg (default)\n"
			"-ow: overwrite output file (default)\n"
			"-autofn, -noautofn: automatically reset file name (ex: [artist] title.ogg) (default)\n"
			"\n"
			"EASY USE:\n"
			"JUST DRAG FILE to me and there will be *.ogg file!\n"
			);
	}

	int parse(int argc, _TCHAR* argv[]) {
		if (argc <= 1 || CMP(argv[1], L"-?") || CMP(argv[1], L"-h")) {
			return -1;
		}

		// default
		bms_path = argv[1];
		output_type = OUTPUT_OGG;
		output_path = substitute_output_extension(IO::get_filedir(argv[0]) + PATH_SEPARATOR + IO::get_filename(bms_path));
		overwrite = true;
		autofilename = true;

		// parse
		for (int i = 2; i < argc; i++) {
			if (CMP(argv[i], L"-oc")) {
				// it's default, so do nothing
			}
			else if (CMP(argv[i], L"-ob")) {
				// destion to bms directory
				output_path = substitute_output_extension(bms_path);
			}
			else if (CMP(argv[i], L"-o")) {
				if (++i == argc)
					return -1;
				output_path = argv[i];
				if (output_path.back() == PATH_SEPARATOR_CHAR) {
					// if destination is folder, then automatically add filename
					output_path = output_path + substitute_output_extension(IO::get_filename(bms_path));
				}
			}
			else if (CMP(argv[i], L"-ow")) {
				overwrite = true;
			}
			else if (CMP(argv[i], L"-wav")) {
				output_type = OUTPUT_WAV;
				output_path = substitute_output_extension(output_path);
			}
			else if (CMP(argv[i], L"-ogg")) {
				// it's default, so do nothing
			}
			else if (CMP(argv[i], L"-noautofn")) {
				autofilename = false;
			}
		}

		return 0;
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	// parse argument
	if (BMX2WAVParameter::parse(argc, argv) == -1) {
		BMX2WAVParameter::help();
		return -1;
	}

	// load bms file
	BmsBms bms;
	try {
		BmsParser::Parse(BMX2WAVParameter::bms_path, bms);
	}
	catch (BmsFileNotSupportedEncoding &e) {
		wprintf(L"Failed to make proper iconv encoding\n");
		return -1;
	}
	BmsTimeManager bms_time;
	BmsNoteContainer bms_note;
	bms.CalculateTime(bms_time);
	bms.GetNotes(bms_note);

	// should we have to change filename?
	if (BMX2WAVParameter::autofilename && bms.GetHeaders().IsExists(L"TITLE") && bms.GetHeaders().IsExists(L"ARTIST")) {
		std::wstring newname = L"[" + bms.GetHeaders()[L"ARTIST"] + L"] " + bms.GetHeaders()[L"TITLE"];
		BMX2WAVParameter::output_path = IO::substitute_filename(BMX2WAVParameter::output_path, newname);
	}
	// replace invalid char
	BMX2WAVParameter::output_path = IO::make_filename_safe(BMX2WAVParameter::output_path);
	// check overwrite file exists
	if (!BMX2WAVParameter::overwrite && IO::is_file_exists(BMX2WAVParameter::output_path)) {
		wprintf(L"output file already exists!");
		return -1;
	}

	// print brief information
	wprintf(L"BMS Path: %ls\n", BMX2WAVParameter::bms_path.c_str());
	if (bms.GetHeaders().IsExists(L"TITLE"))
		wprintf(L"BMS Title: %ls\n", bms.GetHeaders()[L"TITLE"].c_str());
	wprintf(L"BMS Length: %.03lf (sec)\n", bms_time.GetEndTime());
	wprintf(L"Output Path: %ls\n", BMX2WAVParameter::output_path.c_str());

	// load audio data
	wprintf(L"Loading Audio Data ...\n");
	WavMaker wav_maker(true);	// true: use low-pass filter
	BmsWavResource<HQWav> wav_table;
	std::vector<unsigned int> last_used_wav_pos(BmsConst::WORD_MAX_COUNT, 0);
	for (unsigned int i = 0; i < BmsConst::WORD_MAX_COUNT; ++i) {
		BmsWord word(i);
		if (bms.GetRegistArraySet()[L"WAV"].IsExists(word)) {
			std::wstring path(IO::get_filedir(BMX2WAVParameter::bms_path) + PATH_SEPARATOR + bms.GetRegistArraySet()[L"WAV"][word]);
			std::wstring ogg_path = IO::substitute_extension(path, L".ogg");

			if (path == BMX2WAVParameter::output_path) {
				wprintf(L"Bmx resource file path(%ls) cannot same with output path!\n", path.c_str());
				return -1;
			}

			bool path_exists = IO::is_file_exists(path);
			bool ogg_path_exists = IO::is_file_exists(ogg_path);

			if (NOT(path_exists) && NOT(ogg_path_exists)) {
				wprintf(L"[Warning] Cannot find wav/ogg file(%ls). ignore.\n", path.c_str());
				wav_table.SetWAV(word.ToInteger(), wav_maker.MakeNewWav());	// Set Null Sound
			}
			else {
				try {
					if (ogg_path_exists) {
						HQWav* tmp = wav_maker.MakeNewWavFromOggFile(ogg_path);
						wav_table.SetWAV(word.ToInteger(), tmp);
					}
					else if (path_exists) {
						HQWav* tmp = wav_maker.MakeNewWavFromWavFile(path, true);	// default: overlook error
						wav_table.SetWAV(word.ToInteger(), tmp);
					}
				}
				catch (Bmx2WavInvalidWAVFile& e) {
					wprintf(L"Cannot parse wav/ogg file(%ls) correctly. ignore.\n", path.c_str());
					wav_table.SetWAV(word.ToInteger(), wav_maker.MakeNewWav());	// Set Null Sound
				}
			}

			last_used_wav_pos[i] = -1 * static_cast<int>(wav_table.GetWAV(word.ToInteger())->GetLength());
		}
	}

	// start mixing
	wprintf(L"Start Mixing ...\n");
	HQWav result;
	double mixing_pos;
	for (int i = 0; i <= bms.GetPlayableMaxPosition(); i++) {
		mixing_pos = bms_time.GetRow(i).time * HQWav::FREQUENCY;
		for (BmsChannelManager::ConstIterator it = bms.GetChannelManager().Begin(); it != bms.GetChannelManager().End(); ++it) {
			BmsChannel& current_channel = *it->second;
			for (BmsChannel::ConstIterator it2 = current_channel.Begin(); it2 != current_channel.End(); it2++) {
				BmsChannelBuffer& current_buffer = **it2;
				BmsWord current_word = current_buffer[i];
				int wav_channel = current_word.ToInteger();

				if (current_channel.IsShouldPlayWavChannel() && current_word != BmsWord::MIN) {
					// check is Longnote channel & Longnote ending sound
					// if it does, ignore it
					if (current_channel.IsLongNoteChannel()
						&& bms_note[current_channel.GetChannelNumber()][i].type == BmsNote::NOTE_LNEND) {
						continue;
					}
					if (!wav_table.IsLoaded(wav_channel)) {
						// ignore not loaded wav file
						continue;
					}
					// turn off previous WAV if same one is playing
					if (static_cast<unsigned int>(mixing_pos)-last_used_wav_pos[wav_channel]
						< wav_table.GetWAV(wav_channel)->GetLength()) {
						result.DeductAt(static_cast<int>(mixing_pos), *wav_table.GetWAV(wav_channel),
							static_cast<int>(mixing_pos)-last_used_wav_pos[wav_channel]);
					}
					result.MixinAt(static_cast<int>(mixing_pos), *wav_table.GetWAV(wav_channel));
					last_used_wav_pos[wav_channel] = static_cast<int>(mixing_pos);
				}
			}
		}
	}

	// normalize
	wprintf(L"Normalize ...\n");
	double change_ratio;
	result.AverageNormalize(&change_ratio);

	// write (setting tag)
	wprintf(L"Writing file ...\n");
	std::wstring tag_title;
	std::wstring tag_artist;
	if (bms.GetHeaders().IsExists(L"TITLE"))	tag_title = bms.GetHeaders()[L"TITLE"];
	if (bms.GetHeaders().IsExists(L"ARTIST"))	tag_artist = bms.GetHeaders()[L"ARTIST"];
	try {
		if (BMX2WAVParameter::output_type == BMX2WAVParameter::OUTPUT_WAV) {
			result.WriteToFile(BMX2WAVParameter::output_path);
		}
		else if (BMX2WAVParameter::output_type == BMX2WAVParameter::OUTPUT_OGG) {
			HQOgg ogg(&result);
			ogg.SetMetadata(tag_title, tag_artist);
			ogg.WriteToFile(BMX2WAVParameter::output_path);
		}
	}
	catch (Bmx2WavInvalidFile& e) {
		wprintf(L"[ERROR] Cannot write file to %ls. Check other program is using the output file.\n", BMX2WAVParameter::output_path.c_str());
		wprintf(e.Message().c_str());
		wprintf(L"\n");
		return -1;
	}

	// finished!
	wprintf(L"Finished!\n");

	return 0;
}

