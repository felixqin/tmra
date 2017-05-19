#ifndef __NETIO_HTTP_READBUFFER_H__
#define __NETIO_HTTP_READBUFFER_H__


#include <boost/ref.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include "http_message.hpp"



namespace netio {



class http_inbuf;
typedef boost::shared_ptr<http_inbuf> http_inbuf_ptr;


class http_inbuf : public boost::enable_shared_from_this<http_inbuf>
{
public:
    typedef boost::shared_ptr<http_request> request_ptr;
    typedef boost::shared_ptr<http_headers> headers_ptr;
    typedef boost::shared_ptr<char>         data_ptr;

public:
    static http_inbuf_ptr create()
    {
        return http_inbuf_ptr(new http_inbuf());
    }

    template <class ReadStream, class ReadHandler>
    void async_read_from(ReadStream& s, ReadHandler handler)
    {
        async_read_headers(s, handler);
    }

    request_ptr request() const
    {
        return request_;
    }

    headers_ptr headers() const
    {
        return headers_;
    }

    data_ptr body() const
    {
        return body_;
    }

    size_t body_size() const
    {
        return body_size_;
    }

private:
    http_inbuf()
        : body_size_(0)
        , remain_body_size_(0)
    {
    }

    template <class ReadStream, class ReadHandler>
    void async_read_headers(ReadStream& s, ReadHandler handler)
    {
        boost::asio::async_read_until(s, streambuf_, "\r\n\r\n",
            boost::bind(&http_inbuf::handle_read_headers<ReadStream, ReadHandler>, shared_from_this(),
                boost::ref(s), handler,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }

    template <class ReadStream, class ReadHandler>
    void handle_read_headers(ReadStream& s, ReadHandler handler, boost::system::error_code ec, size_t bytes_transferred)
    {
        //printf("this(%p), http_inbuf::handle_read_headers() ec(%d)\n", this, ec.value());
        if (!ec)
        {
            request_ = boost::make_shared<http_request>();
            headers_ = boost::make_shared<http_headers>();
            streambuf_.commit(bytes_transferred);
            std::istream in(&streambuf_);
            in >> *request_ >> *headers_;
            std::string const& bodylenstr = headers_->get("Content-Length");
            body_size_ = atol(bodylenstr.c_str());
            if (body_size_ > 0)
            {
                remain_body_size_ = body_size_;
                body_ = boost::shared_ptr<char>((char*)malloc(body_size_), free);
                async_read_body(s, handler);
                return;
            }
        }

        handler(ec);
    }

    template <class ReadStream, class ReadHandler>
    void async_read_body(ReadStream& s, ReadHandler handler)
    {
        assert(body_size_ > remain_body_size_);
        size_t offset = body_size_ - remain_body_size_;
        char* buf = body_.get() + offset;
        boost::asio::async_read(s, boost::asio::buffer(buf, offset),
            boost::bind(&http_inbuf::handle_read_body<ReadStream, ReadHandler>, shared_from_this(),
                boost::ref(s), handler,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }

    template <class ReadStream, class ReadHandler>
    void handle_read_body(ReadStream& s, ReadHandler handler, boost::system::error_code ec, size_t bytes_transferred)
    {
        if (!ec)
        {
            assert(remain_body_size_ >= bytes_transferred);
            remain_body_size_ -= bytes_transferred;
            if (remain_body_size_ > 0)
            {
                async_read_body(s, handler);
                return;
            }
        }

        handler(ec);
    }

private:
    boost::asio::streambuf streambuf_;
    request_ptr request_;
    headers_ptr headers_;
    data_ptr body_;
    size_t body_size_;
    size_t remain_body_size_;
};


} // namespace netio

#endif // __NETIO_HTTP_READBUFFER_H__


