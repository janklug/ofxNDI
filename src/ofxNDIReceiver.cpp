#include "ofxNDIReceiver.h"
#include "ofUtils.h"
#include <regex>

#include <Processing.NDI.Lib.h>

using namespace std;

ofxNDIReceiver::Source::Source(const char *_ndi_name, const char *_ip_address)
{
	ndi_name = _ndi_name;
	ip_address = _ip_address;
	{
		smatch match;
		string raw = string(_ndi_name);
		if(regex_match(raw, match, regex(R"((.*?) \((.*)\))"))) {
			machine_name = match[1];
			source_name = match[2];
		}
	}
	{
		smatch match;
		string raw = string(_ip_address);
		if(regex_match(raw, match, regex(R"(([\d.]*?):(\d*))"))) {
			ip = match[1];
			port = ofToInt(match[2]);
		}
	}
}


vector<ofxNDIReceiver::Source> ofxNDIReceiver::listSources(Location location, const string &group, const vector<string> extra_ips)
{
	if (!NDIlib_initialize())
	{
		printf("Cannot run NDI.");
		return {};
	}

	auto getSourceInfo = [](bool local, const string &group, const vector<string> extra_ips) -> vector<Source> {
		const NDIlib_find_create_t settings = {local, group.c_str(), ofJoinString(extra_ips, ",").c_str()};
		NDIlib_find_instance_t finder = NDIlib_find_create2(&settings);
		if (!finder) return {};
		
		unsigned int num_sources;
		NDIlib_find_wait_for_sources(finder, 1000);
		const NDIlib_source_t* sources = NDIlib_find_get_current_sources(finder, &num_sources);
		
		vector<ofxNDIReceiver::Source> ret;
		ret.reserve(num_sources);
		for(;num_sources-->0;) {
			ret.emplace_back(sources->p_ndi_name, sources->p_ip_address);
			cout << "NDI Source Detected : " << sources->p_ndi_name << "," << sources->p_ip_address << endl;
			++sources;
		}
		
		NDIlib_find_destroy(finder);
		return std::move(ret);
	};

	vector<ofxNDIReceiver::Source> ret;
	if(location == Location::BOTH || location == Location::LOCAL) {
		auto &&sources = getSourceInfo(true, group, extra_ips);
		copy(begin(sources), end(sources), back_inserter(ret));
	}
	if(location == Location::BOTH || location == Location::LOCAL) {
		auto &&sources = getSourceInfo(false, group, extra_ips);
		copy(begin(sources), end(sources), back_inserter(ret));
	}
	return move(ret);
}

bool ofxNDIReceiver::setup(size_t index, const Settings &settings)
{
	auto &&sources = listSources();
	if(index < sources.size()) {
		return setup(sources[index], settings);
	}
	return false;
}
bool ofxNDIReceiver::setup(const Source &source, const Settings &settings)
{
	NDIlib_recv_create_t creator = { source, settings.color_format, settings.bandwidth, settings.deinterlace };
	receiver_ = NDIlib_recv_create2(&creator);
	if (!receiver_) {
		return false;
	}
	video_.setup(receiver_, timeout_ms_, false);
	return true;
}

void ofxNDIReceiver::update()
{
	video_.update();
	return;
	// The descriptors
	NDIlib_video_frame_t video_frame;
	NDIlib_audio_frame_t audio_frame;
	NDIlib_metadata_frame_t metadata_frame;
	
	switch (NDIlib_recv_capture(receiver_, nullptr, &audio_frame, &metadata_frame, timeout_ms_))
	{	
			// No data
		case NDIlib_frame_type_none:
			printf("No data received.\n");
			break;
			
			// Video data
		case NDIlib_frame_type_video:
			printf("Video data received (%dx%d).\n", video_frame.xres, video_frame.yres);
			NDIlib_recv_free_video(receiver_, &video_frame);
			break;
			
			// Audio data
		case NDIlib_frame_type_audio:
			printf("Audio data received (%d samples).\n", audio_frame.no_samples);
			NDIlib_recv_free_audio(receiver_, &audio_frame);
			break;
			
			// Meta data
		case NDIlib_frame_type_metadata:
			printf("Meta data received (%s).\n", metadata_frame.p_data);
			NDIlib_recv_free_metadata(receiver_, &metadata_frame);
			break;
			
			// Everything else
		default:
			break;
	}
}

ofxNDIReceiver::~ofxNDIReceiver()
{
	// Destroy the receiver
	NDIlib_recv_destroy(receiver_);
	
	// Not required, but nice
	NDIlib_destroy();
}
