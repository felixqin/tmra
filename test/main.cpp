#include <stdio.h>
#include <boost/thread.hpp>
#include "netio/rtsp_session.hpp"
#include "TcpSession.h"
#include "HttpSession.h"
#include "RtpSource.h"



template <class Session>
class CServer
{
    typedef netio::tcp_listener<Session> Listener;
    typedef boost::shared_ptr<Listener> ListenerPtr;

public:
    CServer()
    {
        mListener = boost::make_shared<Listener>(mIoService, 8554);
    }

    bool Start()
    {
        mListener->open();
        mThread1 = boost::make_shared<boost::thread>(
                        boost::bind(&boost::asio::io_service::run, &mIoService));
        mThread2 = boost::make_shared<boost::thread>(
                        boost::bind(&boost::asio::io_service::run, &mIoService));
        return true;
    }

    bool Stop()
    {
        mListener->close();
        mThread1->join();
        mThread2->join();
        return true;
    }

private:
    boost::asio::io_service mIoService;
    boost::shared_ptr<boost::thread> mThread1;
    boost::shared_ptr<boost::thread> mThread2;
    ListenerPtr mListener;
};


int main(int argc, char* argv[])
{
    printf("hello\n");

//#define TEST_TCP_SESSION
//#define TEST_HTTP_SESSION
#define TEST_RTSP_SESSION

#if defined(TEST_TCP_SESSION)
    typedef CTcpSession SessionType;
#elif defined(TEST_HTTP_SESSION)
    typedef CHttpSession SessionType;
#elif defined(TEST_RTSP_SESSION)
    typedef netio::rtsp_session<CRtpSource> SessionType;
#endif

    CServer<SessionType> server;
    server.Start();
    sleep(5);
    server.Stop();
    return 0;
}

