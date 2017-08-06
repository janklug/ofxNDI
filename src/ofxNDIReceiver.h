#pragma once

#include "ofxNDI.h"
#include <Processing.NDI.Lib.h>
#include <string>
#include <vector>
#include "ofxNDIVideoFrameDecoder.h"

class ofxNDIReceiver
{
public:
	virtual ~ofxNDIReceiver();

	struct Source {
		Source(){}
		Source(const char *_ndi_name, const char *_ip_address);
		std::string machine_name="";
		std::string source_name="";
		std::string ip="";
		int port=-1;
		operator NDIlib_source_t() const {
			return NDIlib_source_t{ndi_name.c_str(), ip_address.c_str()};
		}
	private:
		std::string ndi_name="";
		std::string ip_address="";
	};
	struct Settings {
		Settings(){}
		NDIlib_recv_color_format_e color_format=NDIlib_recv_color_format_e_RGBX_RGBA;
		NDIlib_recv_bandwidth_e bandwidth=NDIlib_recv_bandwidth_highest;
		bool deinterlace=false;
	};
	enum class Location {
		LOCAL,REMOTE,BOTH
	};
	static std::vector<Source> listSources(Location location=Location::BOTH, const std::string &group="", const std::vector<std::string> extra_ips={});
	
	bool setup(std::size_t index=0, const Settings &settings=Settings());
	bool setup(const Source &source, const Settings &settings=Settings());
	bool isSetup() const { return receiver_!=nullptr; }
	
	bool isConnected() const;
	
	void update();
	bool isVideoFrameNew() const { return video_.isFrameNew(); }
	template<typename Result>
	void getVideoFrame(Result &result) { video_.decodeTo(result); }
	
private:
	NDIlib_recv_instance_t receiver_=nullptr;
	std::size_t timeout_ms_=1000;
	
	NDIlib_audio_frame_t audio_frame_;
	NDIlib_metadata_frame_t metadata_frame_;
	
	ofxNDI::VideoDecoder video_;
};
