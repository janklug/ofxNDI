//
//  ofxNDIVideoFrameDecoder.cpp
//  TestMac
//
//  Created by Iwatani Nariaki on 2017/04/07.
//
//

#include "ofxNDIVideoFrameDecoder.h"

using namespace ofxNDI;

template<> void VideoDecoder::freeFrame() {
	NDIlib_recv_free_video(receiver_, &frame_.back());
}
template<> bool VideoDecoder::captureFrame() {
	return NDIlib_recv_capture(receiver_, &frame_.back(), nullptr, nullptr, timeout_ms_) == NDIlib_frame_type_video;
}


template<> template<>
void VideoDecoder::decodeTo<ofPixels>(ofPixels &dst)
{
	if(is_front_allocated_) {
		dst.setFromPixels(frame_.front().p_data, frame_.front().xres, frame_.front().yres, 4);
	}
}