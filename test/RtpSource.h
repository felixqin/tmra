#ifndef __RTPSOURCE_H__
#define __RTPSOURCE_H__


#include <string>



class CRtpSource
{
public:
    CRtpSource(std::string const& uri)
    {
        printf("this(%p), CRtpSource::ctor()\n", this);
    }

    ~CRtpSource()
    {
        printf("this(%p), CRtpSource::dtor()\n", this);
    }

    std::string sdp()
    {
        printf("this(%p), CRtpSource::sdp()\n", this);
        return "v=0\r\n"
               "o=- 2251938183 2251938183 IN IP4 0.0.0.0\r\n"
               "s=RTSP Session/2.0\r\n"
               "c=IN IP4 0.0.0.0\r\n"
               "t=0 0\r\n"
               "a=control:*\r\n"
               "a=range:npt=now-\r\n"
               "m=video 0 RTP/AVP 96\r\n"
               "a=control:trackID=0\r\n"
               "a=framerate:25.000000\r\n"
               "a=rtpmap:96 H264/90000\r\n"
               "a=fmtp:96 packetization-mode=1;profile-level-id=64001E;sprop-parameter-sets=Z2QAHq2EAQwgCGEAQwgCGEAQwgCEO1BYCTTcBAQECA==,aO48sA==\r\n"
               "m=audio 0 RTP/AVP 8\r\n"
               "a=control:trackID=1\r\n"
               "a=rtpmap:8 PCMA/8000\r\n";
    }

    template <class Sender>
    void set_sender(std::string const& protocol, std::string const& track, Sender)
    {
    }

    void play()
    {
    }

    void stop()
    {
    }

private:
};


#endif // __RTPSOURCE_H__

