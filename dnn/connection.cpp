#include "connection.h"

connection::connection(unsigned int chn, std::string srvip, unsigned int srvport, int srvtype, const char * model, pfnDnnCb pcb, void * puser)
{
    m_strip = srvip;
    m_iport = srvport;
    m_itype = srvtype;
    m_strmodel = model;
    m_ichn = chn;
    m_pcb   = pcb;
    m_puser = puser;

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
    boost::thread(boost::bind(&connection::run, this)).detach();
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
    boost::thread(boost::bind(&connection::do_predict, this)).detach();
    net::io_context::work work(ioc_);
    ioc_.run();
    m_bExit = true;
}

void connection::update(string srvip, unsigned int srvport, int srvtype)
{
    m_strip = srvip;
    m_iport = srvport;
    m_itype = srvtype;
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

void connection::predict(unsigned char * data, int w, int h, string modelname, unsigned int imgid)
{

    auto httreq = std::make_shared<HttpReq>();
    char url[256] = {0};
    uint32_t imid = imgid;
    char charid[4];
    memcpy(charid, &imid, 4);

    std::copy(charid, charid+4, std::back_inserter(httreq->query));
    std::copy(data, data+w*h*3*4, std::back_inserter(httreq->query));
    std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
    sprintf(url, "http://%s:%d/api/infer/%s/1", m_strip.c_str(), m_iport, modelname.c_str());
    string nvreq = "";
    nvreq = nvreq + "batch_size: 1 input { name: \"__imid\" } input { name: \"image\" } " +
            " output { name: \"imid\" } output { name: \"detection_boxes\" } " +
            " output { name: \"detection_classes\" }" +
            " output { name: \"detection_scores\" } output { name: \"num_detections\" } ";

    std::copy(url, url+256, std::back_inserter(httreq->url));
    httreq->nvreq = nvreq;

    mtxque_.lock();
    queue_.push(httreq);
    mtxque_.unlock();

    std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
    //std::cout << "predict took me " << time_span.count() << " seconds." << endl;
    return;

}


void connection::do_predict()
{
    if(m_ichn==0)
        cout << "here" << endl;
    while(!m_bExit && m_bisconnect)
    {
        std::shared_ptr<HttpReq> req;
        mtxque_.lock();
        bool isempty = queue_.empty();
        if(isempty)
        {
            mtxque_.unlock();
            continue;
        }

        req = queue_.front();
        queue_.pop();
        if(queue_.size() > 5)
        {
            queue_.pop();queue_.pop();queue_.pop();queue_.pop();
        }
        mtxque_.unlock();


        req_.body().clear();
        req_.target(gettarget(req->url.data()));
        req_.method(http::verb::post);
        req_.version(11);
        req_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        req_.set(http::field::host, m_strip);
        req_.set(http::field::content_type, "application/octet-stream");
        req_.set(http::field::accept, "*/*");
        req_.set(http::field::connection, "keep-alive");
        req_.set(http::field::content_length, req->query.size());
        req_.insert("NV-InferRequest", req->nvreq);
        req_.body().assign(req->query.begin(), req->query.end());

        http::write(*stream_, req_);
        res_= {};
        http::read(*stream_, buffer_, res_);

        std::string result { boost::asio::buffers_begin(res_.body().data()),
                           boost::asio::buffers_end(res_.body().data()) };

        if(m_pcb)
            postresult(result);

    }
}



void connection::postresult(string str)
{
    if(str.size() == 0)
        return;
    string tmp = str.substr(str.find("model_name"));
    string modelname = tmp.substr(tmp.find_first_of("\"")+1);
    modelname = modelname.substr(0, modelname.find_first_of("\""));
    if (modelname != "helmet")
        cout << modelname;

    char* p = const_cast<char *>(str.c_str());
    uint32_t imid;
    float detection_boxes[100][4];
    float  detection_classes[100];
    float detection_scores[100];
    int   num_detections;

    memcpy(&imid, p, 4);
    memcpy(detection_boxes, p+4, 100*4*4);
    memcpy(detection_scores, p+1600+4, 100*4);
    memcpy(detection_classes, p+1600+400+4, 100*4);
    memcpy(&num_detections, p+1600+400+400+4, 4);
    p=nullptr;
    DNNTARGET *target = new DNNTARGET[num_detections];
    for(int i=0; i<num_detections; i++)
    {
        (target+i)->cid = static_cast<int>(detection_classes[i]);
        (target+i)->score = detection_scores[i];
        (target+i)->tlx = detection_boxes[i][0] / 416;
        (target+i)->tly = detection_boxes[i][1] / 416;
        (target+i)->brx = detection_boxes[i][2] / 416;
        (target+i)->bry = detection_boxes[i][3] / 416;
    }
    cout << "通道" << m_ichn << "收到推理结果 id=" << imid  << endl;
    m_pcb(m_ichn, imid, modelname.c_str(), target, num_detections, m_puser);
    delete [] target;
}

void connection::getstate(string ip, unsigned int port, string url, string modelname, vector<MODELINFO> &modellist)
{
    if(0 == DNN_SERVER_TRT)
    {
        net::io_context ioc;
        // These objects perform our I/O
        tcp::resolver resolver(ioc);
        beast::tcp_stream stream(ioc);

        // Look up the domain name
        auto const results = resolver.resolve(ip, to_string(port));

        // Make the connection on the IP address we get from a lookup
        stream.connect(results);

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
        MODELINFO modeinfo;
        bool modeready = versionlist[to_string(i)]["readyState"].asString() == "MODEL_READY";
        modeinfo.state = modeready;
        modeinfo.version = i;
        modellist.push_back(modeinfo);
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

