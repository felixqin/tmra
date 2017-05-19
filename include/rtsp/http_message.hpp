#ifndef __NETIO_HTTP_MESSAGE_H__
#define __NETIO_HTTP_MESSAGE_H__



#include <string>
#include <utility>
#include <boost/unordered_map.hpp>
#include <boost/lexical_cast.hpp>
#include "base/Algorithm.h"
#include "http_utils.hpp"



namespace netio {


class http_request;
class http_status;
class http_headers;
typedef boost::shared_ptr<http_request> http_request_ptr;
typedef boost::shared_ptr<http_status>  http_status_ptr;
typedef boost::shared_ptr<http_headers> http_headers_ptr;


class http_request
{
public:
    http_request()
    {
    }

    std::string method() const
    {
        return method_;
    }

    void method(std::string const& value)
    {
        method_ = value;
    }

    std::string uri() const
    {
        return uri_;
    }

    void uri(std::string const& value)
    {
        uri_ = value;
    }

    std::string version() const
    {
        return version_;
    }

    void version(std::string const& value)
    {
        version_ = value;
    }

    template <class InStream>
    friend InStream& operator>>(InStream& in, http_request& msg)
    {
        std::string line;
        std::getline(in, line);
        msg.parse(line);

        return in;
    }

private:
    void parse(std::string const& line)
    {
        auto ptr = line.begin();

        method_.clear();
        ptr = Base::Split(ptr, line.end(), ' ', std::back_inserter(method_));

        uri_.clear();
        ptr = Base::Split(ptr, line.end(), ' ', std::back_inserter(uri_));

        version_.clear();
        ptr = Base::Split(ptr, line.end(), '\r', std::back_inserter(version_));

        return;
    }

private:
    std::string method_;
    std::string uri_;
    std::string version_;
};



class http_status
{
public:
    http_status()
    {
    }

    std::string version() const
    {
        return version_;
    }

    void version(std::string const& value)
    {
        version_ = value;
    }

    int code() const
    {
        return (int)code_;
    }

    void code(int code)
    {
        code_ = (status_code::type)code;
        comment_ = to_string(code_);
    }

    template <class OutStream>
    friend OutStream& operator<<(OutStream& out, http_status const& status)
    {
        return out << status.version_ << ' ' << (int)status.code_ << ' ' << status.comment_ << "\r\n";
    }

private:
    std::string version_;
    status_code::type code_;
    std::string comment_;
};



class http_headers
{
public:
    http_headers()
    {
    }

    std::string get(std::string const& key) const
    {
        boost::unordered_map<std::string, std::string>::const_iterator it = headers_.find(key);
        return it != headers_.end() ? it->second : std::string();
    }

    void set(std::string const& key, std::string const& value)
    {
        headers_.insert(std::make_pair(key,value));
    }

    void set(std::string const& key, int value)
    {
        headers_[key] = boost::lexical_cast<std::string>(value);
    }

    template <class InStream>
    friend InStream& operator>>(InStream& in, http_headers& msg)
    {
        std::string line;
        while (!in.eof())
        {
            std::getline(in, line);
            msg.parse(line);
        }

        return in;
    }

    template <class OutStream>
    friend OutStream& operator<<(OutStream& out, http_headers const& msg)
    {
        boost::unordered_map<std::string, std::string>::const_iterator it = msg.headers_.begin();
        for (;it != msg.headers_.end(); it++)
        {
            out << it->first << ": " << it->second << "\r\n";
        }
        out << "\r\n";

        return out;
    }

private:
    void parse(std::string &line)
    {
        std::string key;
        std::string value;

        auto ptr = line.begin();
        ptr = Base::Split(ptr, line.end(), ':', std::back_inserter(key));
        ptr = Base::Split(ptr, line.end(), '\r', std::back_inserter(value));
        if (!key.empty())
        headers_.insert(std::make_pair(key, value));

        return;
    }

private:
    boost::unordered_map<std::string, std::string> headers_;
};


} // namespace netio

#endif // __NETIO_HTTP_MESSAGE_H__


