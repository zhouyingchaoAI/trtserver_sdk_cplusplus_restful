﻿#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#include "boost/thread/mutex.hpp"
#include "util.h"
#include <string>
#include <utdnn.h>
#include <map>
#include <iostream>
#include "boost/shared_ptr.hpp"
#include "boost/thread.hpp"
#include <boost/asio/strand.hpp>
#include <queue>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>


#include "utdnn.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
using namespace std;

struct HttpReq
{
    vector<char> url;
    string nvreq;
    vector<unsigned char> query;
};

class connection : public boost::enable_shared_from_this<connection>
{
public:
    connection(unsigned int chn, string srvip, unsigned int srvport, int srvtype, const char * model, pfnDnnCb pcb, void * puser);
    ~connection();

public:
    void predict(unsigned char * data, int w, int h, string modelname, unsigned int imgid);

    static void getstate(string ip, unsigned int port, string url, string modelname, vector<MODELINFO> &modellist);

    void update(string srvip, unsigned int srvport, int srvtype);

private:
    void postresult(string str);
    void run();
    void fail(beast::error_code ec, const char *what);
    void do_predict();
    void on_write(beast::error_code ec, std::size_t bytes_transferred);
    void on_read(beast::error_code ec, std::size_t bytes_transferred);
    static string gettarget(string url);
    static void state2st(string str, string modelname, vector<MODELINFO> &modellist);
    static std::string json2string(Json::Value &json);
    static bool string2json(const char * str, Json::Value * json);


public:
    std::shared_ptr<tcp::resolver>       resolver_;
    std::shared_ptr<beast::tcp_stream>   stream_;
    beast::flat_buffer buffer_; // (Must persist between reads)
    http::request<http::string_body> req_;
    http::response<http::dynamic_body> res_;
    net::io_context ioc_;
    queue<std::shared_ptr<HttpReq>> queue_;
    boost::mutex                    mtxque_;

private:
    string       m_strip;
    unsigned int m_iport;
    unsigned int m_ichn;
    int          m_itype;
    string       m_strmodel;
    pfnDnnCb     m_pcb;
    void *       m_puser;
    bool         m_bExit;


    bool         m_bisconnect;
};

#endif // __CONNECTION_H__
