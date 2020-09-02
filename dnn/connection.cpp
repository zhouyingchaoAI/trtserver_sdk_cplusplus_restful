#include "connection.h"

connection::connection(std::string srvip, unsigned int srvport, const char * model)
{
    m_strip = srvip;
    m_iport = srvport;
    m_strmodel = model;

    char url[256] = {0};
    sprintf(url, "http://%s:%d/api/status/%s?format=json", m_strip.c_str(), m_iport, model);
    vector<MODELINFO> modellist;
    modellist.clear();
    getstate(m_strip, m_iport, url, model, modellist);

    for(auto a:modellist) {
        m_itype = a.type;
        break;
    }

    resolver_ = std::make_shared<tcp::resolver>(ioc_);
    stream_ = std::make_shared<beast::tcp_stream>(ioc_);
    try{
        auto const results = resolver_->resolve(m_strip, to_string(m_iport));
        stream_->connect(results);
        m_bisconnect = true;
    }
    catch(std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        m_bisconnect = false;
    }
}

connection::~connection()
{
    ioc_.stop();
    while(!m_bExit)
        boost::this_thread::sleep(boost::posix_time::microseconds(10));
}


void connection::run()
{
    m_bExit = false;
    net::io_context::work work(ioc_);
    ioc_.run();
    m_bExit = true;
}

void connection::update(string srvip, unsigned int srvport)
{
    m_strip = srvip;
    m_iport = srvport;

	resolver_ = std::make_shared<tcp::resolver>(ioc_);
	stream_ = std::make_shared<beast::tcp_stream>(ioc_);
	try {
		auto const results = resolver_->resolve(m_strip, to_string(m_iport));
		stream_->connect(results);
		m_bisconnect = true;
	}
	catch (std::exception const& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		m_bisconnect = false;
	}
}

string connection::gettarget(string url)
{
    const std::string& http___ = "http://";
    const std::string& https___ = "https://";
    std::string temp_data = url;
    std::string target;

    // 截断http协议头
    if (temp_data.find(http___) == 0)
        temp_data = temp_data.substr(http___.length());
    else if (temp_data.find(https___) == 0)
        temp_data = temp_data.substr(https___.length());

    // 解析域名
    std::size_t idx = temp_data.find('/');
    // 解析 域名后的page
    if (std::string::npos == idx)
    {
        target = "/";
        idx = temp_data.size();
    }
    else
        target = temp_data.substr(idx);
    return target;
}

void connection::fail(beast::error_code ec, const char *what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

void connection::predict(unsigned char *data, int w, int h, DNNTARGET *objs, int *size, int jpgsize)
{
    if(!m_bisconnect)
        return;
    char url[256] = {0};
    std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
    sprintf(url, "http://%s:%d/api/infer/%s/1", m_strip.c_str(), m_iport, m_strmodel.c_str());

    http::response<http::dynamic_body> res_;

    if(m_itype == DNN_SERVER_TRT)
    {
        string nvreq = "";
        nvreq = nvreq + "batch_size: 1 input { name: \"image\" } " +
                " output { name: \"detection_boxes\" } " +
                " output { name: \"detection_classes\" }" +
                " output { name: \"detection_scores\" } output { name: \"num_detections\" } ";

        http::request<http::buffer_body> req_;
        req_ = {};
        req_.target(gettarget(url));
        req_.method(http::verb::post);
        req_.version(11);
        req_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        req_.set(http::field::host, m_strip);
        req_.set(http::field::content_type, "application/octet-stream");
        req_.set(http::field::accept, "*/*");
        req_.set(http::field::connection, "keep-alive");
        req_.set(http::field::content_length, 416*416*3*4);
        req_.insert("NV-InferRequest", nvreq);
        req_.body().data = data;
        req_.body().size = 416*416*3*4;
        req_.body().more = false;
        http::write(*stream_, req_);
    }
    else {
        uint32_t sz = static_cast<uint32_t>(jpgsize);
        char sz8[4];
        memcpy(sz8, &sz, 4);
        vector<unsigned char> trtrequest(1024 * 1024);
        trtrequest.clear();
        std::copy(sz8, sz8+4, std::back_inserter(trtrequest));
        std::copy(data, data+sz, std::back_inserter(trtrequest));
        string nvreq = "batch_size: 1 input { name: \"inputs\" dims: 1 batch_byte_size: " + to_string(trtrequest.size()) +
                " } output { name: \"detection_boxes\" } output { name: \"detection_classes\" }" +
                " output { name: \"detection_scores\" } output { name: \"num_detections\" } ";
        http::request<http::buffer_body> req_;
        req_ = {};
        req_.target(gettarget(url));
        req_.method(http::verb::post);
        req_.version(11);
        req_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        req_.set(http::field::host, m_strip);
        req_.set(http::field::content_type, "application/octet-stream");
        req_.set(http::field::accept, "*/*");
        req_.set(http::field::connection, "keep-alive");
        req_.set(http::field::content_length, trtrequest.size());
        req_.insert("NV-InferRequest", nvreq);
        req_.body().data = &trtrequest[0];
        req_.body().size = trtrequest.size();
        req_.body().more = false;
//        req_.body().assign(trtrequest.begin(), trtrequest.end());
        http::write(*stream_, req_);
    }

    res_= {};
    buffer_ = {};
    http::read(*stream_, buffer_, res_);

    std::string result { boost::asio::buffers_begin(res_.body().data()),
                       boost::asio::buffers_end(res_.body().data()) };


    if(result.size() == 0)
        return;

    char* p = const_cast<char *>(result.c_str());
    float detection_boxes[100][4];
    float  detection_classes[100];
    float detection_scores[100];
    int   num_detections;
    if(m_itype == DNN_SERVER_TRT)
    {
        memcpy(detection_boxes, p, 100*4*4);
        memcpy(detection_scores, p+1600, 100*4);
        memcpy(detection_classes, p+1600+400, 100*4);
        memcpy(&num_detections, p+1600+400+400, 4);
    }
    else
    {
        memcpy(detection_boxes, p, 100*4*4);
        memcpy(detection_classes, p+1600, 100*4);
        memcpy(detection_scores, p+1600+400, 100*4);
        memcpy(&num_detections, p+1600+400+400, 4);
    }

    p=nullptr;
    DNNTARGET *target = new DNNTARGET[num_detections];
    int savedmodecnt = 0;
    if(m_itype == DNN_SERVER_TFS)
    {
        for(int i=0; i<num_detections; i++)
        {
            if(detection_scores[i] > 0.3)
                savedmodecnt++;
        }
        num_detections = savedmodecnt;
    }

    for(int i=0; i<num_detections; i++)
    {
        (target+i)->score = detection_scores[i];
        (target+i)->cid = static_cast<int>(detection_classes[i]);
        (target+i)->tlx = m_itype == DNN_SERVER_TFS ? detection_boxes[i][1] : detection_boxes[i][0] / 416;
        (target+i)->tly = m_itype == DNN_SERVER_TFS ? detection_boxes[i][0] : detection_boxes[i][1] / 416;
        (target+i)->brx = m_itype == DNN_SERVER_TFS ? detection_boxes[i][3] : detection_boxes[i][2] / 416;
        (target+i)->bry = m_itype == DNN_SERVER_TFS ? detection_boxes[i][2] : detection_boxes[i][3] / 416;
    }

    std::copy(&target[0], &target[0]+num_detections, objs);
    *size = num_detections;
    delete [] target;
    return;
}


void connection::getstate(string ip, unsigned int port, string url, string modelname, vector<MODELINFO> &modellist)
{
    net::io_context ioc;
    // These objects perform our I/O
    tcp::resolver resolver(ioc);
    beast::tcp_stream stream(ioc);

    try{
        // Look up the domain name
        auto const results = resolver.resolve(ip, to_string(port));
        // Make the connection on the IP address we get from a lookup
        stream.connect(results);
    }
    catch(std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return;
    }

    // Set up an HTTP GET request message
    http::request<http::string_body> req{http::verb::get, gettarget(url), 11};
    req.set(http::field::host, ip);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    // Send the HTTP request to the remote host
    http::write(stream, req);

    // This buffer is used for reading and must be persisted
    beast::flat_buffer buffer;

    // Declare a container to hold the response
    http::response<http::dynamic_body> res;

    // Receive the HTTP response
    http::read(stream, buffer, res);

    // Write the message to standard out
    std::string statestr { boost::asio::buffers_begin(res.body().data()),
                       boost::asio::buffers_end(res.body().data()) };

    state2st(statestr, modelname, modellist);

        // Gracefully close the socket
//        beast::error_code ec;
//        stream.socket().shutdown(tcp::socket::shutdown_both, ec);

        // not_connected happens sometimes
        // so don't bother reporting it.
        //
//        if(ec && ec != beast::errc::not_connected)
//            throw beast::system_error{ec};
}

void connection::state2st(string str, string modelname, vector<MODELINFO> &modellist)
{
    Json::Value root;
    string2json(str.c_str(), &root);
    if(root["modelStatus"].isNull())
        return;
    if(root["modelStatus"][modelname]["versionStatus"].isNull())
        return;
    int versionsz = static_cast<int>(root["modelStatus"][modelname]["versionStatus"].size());
    Json::Value versionlist = root["modelStatus"][modelname]["versionStatus"];
    for(int i=1; i<=versionsz; i++)
    {
        MODELINFO modelinfo;
        bool modeready = versionlist[to_string(i)]["readyState"].asString() == "MODEL_READY";
        modelinfo.state = modeready;
        modelinfo.version = i;
        string modeltype = root["modelStatus"][modelname]["config"]["platform"].asCString();
        modelinfo.type = modeltype == "tensorflow_savedmodel" ? 1 : 0;
        modellist.push_back(modelinfo);
    }
}

std::string connection::json2string(Json::Value &json)
{
    Json::StreamWriterBuilder builder;
    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    std::ostringstream os;
    writer->write(json, &os);
    return os.str();
}

bool connection::string2json(const char * str, Json::Value * json)
{
    Json::CharReaderBuilder builder;
    std::unique_ptr<Json::CharReader> const reader(builder.newCharReader());

    std::string error;
    if(!reader->parse(str, str+strlen(str), json, &error))
        return false;
    return true;
}

