#ifndef __NETIO_RTSP_SESSION_H__
#define __NETIO_RTSP_SESSION_H__


#include <boost/lexical_cast.hpp>
#include <boost/ref.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include "base/System.h"
#include "tcp_listener.hpp"
#include "http_inbuf.hpp"
#include "http_message.hpp"



namespace netio {



template <class RtpSource>
class rtsp_session : public boost::enable_shared_from_this<rtsp_session<RtpSource> >
{
    typedef rtsp_session<RtpSource>      this_type;
    typedef boost::shared_ptr<this_type> this_ptr;
    typedef RtpSource                    source_type;
    typedef boost::shared_ptr<RtpSource> source_ptr;

public:
    rtsp_session(socket_ptr socket)
        : socket_(socket)
        , loop_timer_(socket->get_io_service())
    {
    }

    ~rtsp_session()
    {
    }

    void start()
    {
        async_read_request();
    }

private:
    void async_read_request()
    {
        printf("this(%p), rtsp_session::async_read()\n", this);
        netio::http_inbuf_ptr inbuf = netio::http_inbuf::create();
        inbuf->async_read_from(*socket_, ReadHandler(this_type::shared_from_this(), inbuf));
    }

    void loop_try_read_request()
    {
        printf("this(%p), rtsp_session::loop_try_read_request()\n", this);
        if (socket_->available())
        {
            async_read_request();
        }
        else
        {
            // loop try
            loop_timer_.expires_from_now(boost::posix_time::seconds(3));
            loop_timer_.async_wait(
                boost::bind(&this_type::handle_looptimer_timeout, this_type::shared_from_this(),
                    boost::asio::placeholders::error));
        }
    }

    void handle_looptimer_timeout(boost::system::error_code const& ec)
    {
        if (!ec)
        {
            loop_try_read_request();
        }
    }

    void stop_loop_try_timer()
    {
        loop_timer_.cancel();
    }

    struct ReadHandler
    {
        this_ptr session_;
        netio::http_inbuf_ptr inbuf_;
        ReadHandler(this_ptr session, netio::http_inbuf_ptr inbuf) : session_(session), inbuf_(inbuf) {}

        void operator()(boost::system::error_code ec)
        {
            //printf("this(%p), rtsp_session::ReadHandler ec(%d)\n", this, ec.value());
            if (!ec)
            {
                session_->handle_request(inbuf_);
            }
        }
    };

    template <class WriteHandler>
    void async_write_header(http_status status, http_headers headers, WriteHandler handler)
    {
        //std::cout << status << headers;
        std::ostream out(&outbuf_);
        out << status << headers;
        boost::asio::async_write(*socket_, outbuf_, handler);
    }

    template <class WriteHandler>
    void async_write_body(std::string const& body, WriteHandler handler)
    {
        //std::cout << body;
        boost::asio::async_write(*socket_, boost::asio::buffer(body), handler);
    }

    static void post_write_rtp_packet(boost::weak_ptr<this_type> this_wp, boost::shared_ptr<char> data, size_t len)
    {
        boost::shared_ptr<this_type> thiz = this_wp.lock();
        if (thiz)
        {
            thiz->socket_->get_io_service().post(
                boost::bind(&this_type::async_write_rtp_packet, thiz, data, len));
        }
    }

    void async_write_rtp_packet(boost::shared_ptr<char> data, size_t len)
    {
        boost::asio::async_write(*socket_, boost::asio::buffer(data.get(), len),
            boost::bind(&this_type::handle_write_rtp_packet, this_type::shared_from_this(), data,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }

    void handle_write_rtp_packet(boost::shared_ptr<char> data, boost::system::error_code ec, size_t bytes_transferred)
    {
    }

    struct DummyHandler
    {
        void operator()(boost::system::error_code ec, size_t /*bytes_transferred*/)
        {
        }
    };

    struct HandleForNextRequest
    {
        this_ptr session_;
        HandleForNextRequest(this_ptr session) : session_(session) {}
        void operator()(boost::system::error_code ec, size_t /*bytes_transferred*/)
        {
            if (!ec)
            {
                session_->async_read_request();
            }
        }
    };

    struct HandleForLoopTryNextRequest
    {
        this_ptr session_;
        HandleForLoopTryNextRequest(this_ptr session) : session_(session) {}
        void operator()(boost::system::error_code ec, size_t /*bytes_transferred*/)
        {
            if (!ec)
            {
                session_->loop_try_read_request();
            }
        }
    };

    template <class NextHandler>
    struct HandleForWriteBody
    {
        this_ptr session_;
        std::string body_;
        HandleForWriteBody(this_ptr session, std::string const& body) : session_(session), body_(body) {}
        void operator()(boost::system::error_code ec, size_t /*bytes_transferred*/)
        {
            if (!ec)
            {
                session_->async_write_body(body_, NextHandler(session_));
            }
        }
    };

    void reply(int code, http_headers const& headers)
    {
        http_status status;
        status.version("RTSP/1.0");
        status.code(code);
        async_write_header(status, headers,
            HandleForNextRequest(this_type::shared_from_this()));
    }

    void reply(int code, http_headers const& headers, std::string const& body)
    {
        http_status status;
        status.version("RTSP/1.0");
        status.code(code);
        async_write_header(status, headers,
            HandleForWriteBody<HandleForNextRequest>(this_type::shared_from_this(), body));
    }

    void reply_then_loop_try_next(int code, http_headers const& headers)
    {
        http_status status;
        status.version("RTSP/1.0");
        status.code(code);
        async_write_header(status, headers,
            HandleForLoopTryNextRequest(this_type::shared_from_this()));
    }

    void reply_then_over(int code, http_headers const& headers)
    {
        http_status status;
        status.version("RTSP/1.0");
        status.code(code);
        async_write_header(status, headers, DummyHandler());
    }

    void reply_ok(http_headers const& headers)
    {
        reply(200, headers);
    }

    void reply_ok(http_headers const& headers, std::string const& body)
    {
        reply(200, headers, body);
    }

    void handle_request(http_inbuf_ptr inbuf)
    {
        http_request_ptr request = inbuf->request();
        http_headers_ptr headers = inbuf->headers();
        //boost::shared_ptr<char> body = inbuf->body();
        size_t bodylen = inbuf->body_size();

        //std::cout << *headers << std::endl;
        std::string const& method = request->method();
        printf("this(%p), rtsp_session::handle_request() method(%s) uri(%s) bodylen(%d)\n", this, method.c_str(), request->uri().c_str(), bodylen);

        if (method == "OPTIONS")
        {
            uri_ = request->uri();
            source_ = boost::make_shared<source_type>(uri_);

            std::string const& cseq = headers->get("CSeq");

            http_headers reply_headers;
            reply_headers.set("Server", "Rtsp Server/2.0");
            reply_headers.set("CSeq", cseq);
            reply_headers.set("Public", "OPTIONS,DESCRIBE,SETUP,PLAY,PAUSE,TEARDOWN,SET_PARAMETER");

            reply_ok(reply_headers);
        }
        else if (method == "DESCRIBE")
        {
            std::string const& sdp = source_->sdp();
            std::string const& cseq = headers->get("CSeq");

            http_headers reply_headers;
            reply_headers.set("Server", "Rtsp Server/2.0");
            reply_headers.set("CSeq", cseq);
            reply_headers.set("Content-Base", uri_);
            reply_headers.set("Content-Type", "application/sdp");
            reply_headers.set("Content-Length", sdp.size());
            reply_headers.set("Cache-Control", "must-revalidate");
            reply_headers.set("x-Accept-Dynamic-Rate", "1");

            reply_ok(reply_headers, sdp);
        }
        else if (method == "SETUP")
        {
            std::string const& cseq = headers->get("CSeq");
            std::string session = headers->get("Session");
            if (session.empty())
            {
                uint64_t tickcount = Base::CSystem::GetTickCount();
                session = boost::lexical_cast<std::string>(tickcount);
            }

            source_->set_sender("TCP", "TrackID",
                boost::bind(&this_type::post_write_rtp_packet, this->weak_from_this(), _1, _2));

            http_headers reply_headers;
            reply_headers.set("Server", "Rtsp Server/2.0");
            reply_headers.set("CSeq", cseq);
            reply_headers.set("Session", session + ";timeout=60");
            reply_headers.set("Transport", "RTP/AVP/TCP;unicast;interleaved=0-1;ssrc=0c00cdee;mode=PLAY;ssrc=0000001E");
            reply_headers.set("x-dynamic-rate", "1");

            reply_ok(reply_headers);
        }
        else if (method == "PLAY")
        {
            std::string const& cseq = headers->get("CSeq");
            std::string session = headers->get("Session");

            source_->play();

            http_headers reply_headers;
            reply_headers.set("Server", "Rtsp Server/2.0");
            reply_headers.set("CSeq", cseq);
            reply_headers.set("Session", session);
            reply_headers.set("RTP-Info", "url=trackID=0;seq=35013;rtptime=569160445,url=trackID=1;seq=48874;rtptime=1732727168");
            reply_headers.set("Range", "npt=0.000000-");

            reply_then_loop_try_next(200, reply_headers);
        }
        else if (method == "TEARDOWN")
        {
            std::string const& cseq = headers->get("CSeq");
            std::string session = headers->get("Session");

            stop_loop_try_timer();
            source_->stop();

            http_headers reply_headers;
            reply_headers.set("Server", "Rtsp Server/2.0");
            reply_headers.set("CSeq", cseq);
            reply_headers.set("Session", session);

            reply_then_over(200, reply_headers);
        }
    }

private:
    socket_ptr socket_;
    boost::asio::deadline_timer loop_timer_;
    boost::asio::streambuf outbuf_;
    source_ptr source_;
    std::string uri_;
};

} // namespace netio

#endif // __NETIO_RTSP_SESSION_H__


