// bmx2wav_remake.cpp : Defines the entry point for the console application.
//

#include "targetver.h"
#include "bmsbel/bms_bms.h"
#include "bmsbel/bms_parser.h"
#include "bmx2wav_common.h"
#include "exception.h"
#ifdef _WIN32
#include <tchar.h>
#endif
#include <wchar.h>
#include <string>
#include <string.h>
#include <stdio.h>

#include "bms_resource.h"
#include "audio.h"

#define NOT(v) (!(v))
#define CMP(a, b) (strcmp(a, b) == 0)

namespace BMX2WAVParameter {
	enum OUTPUT_TYPE { OUTPUT_OGG, OUTPUT_WAV, OUTPUT_FLAC } output_type;
	std::string bms_path;
	std::string output_path;
	bool overwrite;
	bool autofilename;
	bool normalize = false;
	double quality = 0.95;

	std::string substitute_output_extension(const std::string& filename) {
		if (output_type == OUTPUT_OGG) {
			return IO::substitute_extension(filename, ".ogg");
		}
		else if (output_type == OUTPUT_FLAC) {
			return IO::substitute_extension(filename, ".flac");
		}
		else {
			return IO::substitute_extension(filename, ".wav");
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
			"-flac: output audio as flac\n"
			"-q: set VBR quality (0 ~ 1, default: 0.95)\n"
			"-norm: normalize sound volume\n"
			"-ow: overwrite output file (default)\n"
			"-autofn, -noautofn: automatically reset file name (ex: [artist] title.ogg) (default)\n"
			"\n"
			"EASY USE:\n"
			"JUST DRAG FILE to me and there will be *.ogg file!\n"
			);
	}

	int parse(int argc, char** argv) {
		if (argc <= 1 || CMP(argv[1], "-?") || CMP(argv[1], "-h")) {
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
			if (CMP(argv[i], "-oc")) {
				// it's default, so do nothing
			}
			else if (CMP(argv[i], "-ob")) {
				// destion to bms directory
				output_path = substitute_output_extension(bms_path);
			}
			else if (CMP(argv[i], "-o")) {
				if (++i == argc)
					return -1;
				output_path = argv[i];
				if (output_path.back() == '\\' || output_path.back() == '/') {
					// if destination is folder, then automatically add filename
					output_path = output_path + substitute_output_extension(IO::get_filename(bms_path));
				}
			}
			else if (CMP(argv[i], "-ow")) {
				overwrite = true;
			}
			else if (CMP(argv[i], "-wav")) {
				output_type = OUTPUT_WAV;
				output_path = substitute_output_extension(output_path);
			}
			else if (CMP(argv[i], "-ogg")) {
				// it's default, so do nothing
			}
			else if (CMP(argv[i], "-flac")) {
				output_type = OUTPUT_FLAC;
				output_path = substitute_output_extension(output_path);
			}
			else if (CMP(argv[i], "-q")) {
				if (++i == argc) return -1;
				quality = atof(argv[i]);
			}
			else if (CMP(argv[i], "-norm")) {
				normalize = true;
			}
			else if (CMP(argv[i], "-noautofn")) {
				autofilename = false;
			}
		}

		return 0;
	}

#ifdef _WIN32
	int parse(int argc, _TCHAR* argv[]) {
		// convert all arguments to utf8
		char** args = new char*[argc];
		for (int i = 0; i < argc; i++) {
			args[i] = new char[1024];
			ENCODING::wchar_to_utf8(argv[i], args[i], 1024);
		}
		int r = parse(argc, args);
		for (int i = 0; i < argc; i++) {
			delete args[i];
		}
		delete args;
		return r;
	}
#endif
}

#ifdef _WIN32
int _tmain(int argc, _TCHAR* argv[])
#else
int main(int argc, char** argv)
#endif
{
	// 2 vars is enough
	Audio audio_out;
	Mixer mixer;

	// parse argument
	if (BMX2WAVParameter::parse(argc, argv) == -1) {
		BMX2WAVParameter::help();
		return -1;
	}

	// load bms file
	BmsBms bms;
	BmsParser::Parser p(bms);
	if (!p.Load(BMX2WAVParameter::bms_path.c_str())) {
		printf("Error occured during parsing Bms file!\n");
		printf("Details:\n");
		printf(p.GetLog().c_str());
	}
	std::string artist_ = "(none)";
	std::string title_ = "(none)";
	bms.GetHeaders().Query("ARTIST", artist_);
	bms.GetHeaders().Query("TITLE", title_);
	// should we have to change filename?
	if (BMX2WAVParameter::autofilename) {
		char newname_[1024];
		sprintf_s(newname_, "[%s] %s", artist_.c_str(), title_.c_str());
		BMX2WAVParameter::output_path = IO::substitute_filename(
			BMX2WAVParameter::output_path, 
			IO::make_filename_safe(newname_)
		);
	}
	// check overwrite file exists
	if (!BMX2WAVParameter::overwrite && IO::is_file_exists(BMX2WAVParameter::output_path)) {
		printf("output file already exists!");
		return -1;
	}

	// print brief information
	printf("BMS Path: %s\n", BMX2WAVParameter::bms_path.c_str());
	printf("BMS Title: %s\n", title_.c_str());
	printf("BMS Artist: %s\n", artist_.c_str());
	printf("BMS Length: %.03lf (sec)\n", bms.GetEndTime());
	printf("Output Path: %s\n", BMX2WAVParameter::output_path.c_str());

	// load audio data
	printf("Loading Audio Data ...\n");
	for (unsigned int i = 0; i < BmsConst::WORD_MAX_COUNT; ++i) {
		BmsWord word(i);
		if (bms.GetRegistArraySet()["WAV"].IsExists(word)) {
			std::string path(IO::get_filedir(BMX2WAVParameter::bms_path) + 
				PATH_SEPARATOR + 
				bms.GetRegistArraySet()["WAV"].At(word));
			std::string ogg_path = IO::substitute_extension(path, ".ogg");

			if (path == BMX2WAVParameter::output_path) {
				printf("Bmx resource file path(%s) cannot same with output path!\n", path.c_str());
				return -1;
			}

			bool path_exists = IO::is_file_exists(path);
			bool ogg_path_exists = IO::is_file_exists(ogg_path);

			if (NOT(path_exists) && NOT(ogg_path_exists)) {
				printf("[Warning] Cannot find wav/ogg file(%s). ignore.\n", path.c_str());
			}
			else {
				bool r = true;
				if (ogg_path_exists) {
					r = mixer.LoadFile(i, ogg_path);
				}
				else if (path_exists) {
					r = mixer.LoadFile(i, path);
				}
				if (!r){
					printf("Cannot parse wav/ogg file(%s) correctly. ignore.\n", path.c_str());
				}
			}
		}
	}

	// start mixing
	printf("Start Mixing ...\n");
	double mixing_sample;
	std::set<barindex> barmap;
	bms.GetChannelManager().GetObjectExistBar(barmap);
	BmsNoteManager note;
	bms.GetNoteData(note);
	// prepare for recording
	mixer.StartMixing();
	audio_out.Create();
	for (auto it = barmap.begin(); it != barmap.end(); ++it) {
		barindex bar = *it;
		mixing_sample = (int)(bms.GetTimeManager().GetTimeFromBar(bar) * FREQUENCY) * 2;

		// record samples to audio
		while (mixer.GetTick() < mixing_sample) {
			audio_out.Add(mixer.Tick());
		}

		// control mixer channel
		for (BmsChannelManager::ConstIterator it = bms.GetChannelManager().Begin(); it != bms.GetChannelManager().End(); ++it) {
			BmsChannel& current_channel = *it->second;
			for (BmsChannel::ConstIterator it2 = current_channel.Begin(); it2 != current_channel.End(); it2++) {
				BmsBuffer& current_buffer = **it2;
				BmsWord current_word = current_buffer.Get(bar);
				int wav_channel = current_word.ToInteger();

				// reached an object and it should be played!
				if (current_channel.IsShouldPlayWavChannel() && current_word != BmsWord::MIN) {
					// check is Longnote channel & Longnote ending sound
					// if it does, ignore it
					if (current_channel.IsLongNoteChannel()
						&& note[current_channel.GetChannelNumber()].Get(bar).type == BmsNote::NOTE_LNEND) {
						continue;
					}
					// Replay WAV (in case of it was playing)
					mixer.Stop(wav_channel);
					mixer.Start(wav_channel);
				}
			}
		}
	}

	// normalize
#if 0
	// TODO
	printf("Normalize ...\n");
	double change_ratio;
	result.AverageNormalize(&change_ratio);
#endif

	// write (setting tag)
	audio_out.SetTitle(title_);
	audio_out.SetArtist(artist_);
	audio_out.SetQuality(BMX2WAVParameter::quality);
	printf("Writing file ...\n");
	{
		// output format is automatically determined by extension
		bool r = true;
		int flag = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
		if (BMX2WAVParameter::output_type == BMX2WAVParameter::OUTPUT_WAV) {
		}
		else if (BMX2WAVParameter::output_type == BMX2WAVParameter::OUTPUT_OGG) {
			flag = SF_FORMAT_OGG | SF_FORMAT_VORBIS;
		}
		else if (BMX2WAVParameter::output_type == BMX2WAVParameter::OUTPUT_FLAC) {
			flag = SF_FORMAT_FLAC | SF_FORMAT_PCM_16;
		}
		r = audio_out.SaveFile(BMX2WAVParameter::output_path, flag);

		if (!r) {
			printf("[ERROR] Cannot write file to %s. Check other program is using the output file.\n", BMX2WAVParameter::output_path.c_str());
			return -1;
		}
	}

	// finished!
	printf("Finished!\n");

	return 0;
}

