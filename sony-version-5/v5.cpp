// Poco Libraries
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/StreamCopier.h>
#include <Poco/DOM/Document.h>
#include <Poco/DOM/DOMParser.h>
#include <Poco/DOM/Element.h>
#include <Poco/Path.h>
#include <Poco/URI.h>
#include <Poco/Exception.h>
// RapidJSON Libraries
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filereadstream.h"
// Standard C++ Libraries
#include <iostream>
#include <sstream>
#include<string>
#include <cstdio>
#include <bits/stdc++.h>

using namespace Poco::Net;


// rapidjson::Document sonyDocJSON;

Poco::XML::Element* getThroughPathElement(Poco::XML::Element*, const std::vector<std::string>&);

std::string getRemoteControlInfo(std::string tv_ip) {
    try {
        std::string ip = "http://" + tv_ip + "/sony/system";

        Poco::URI uri(ip);
        HTTPClientSession session(uri.getHost(), uri.getPort());

        std::string path (uri.getPathAndQuery());
        if(path.empty()) {
            path = '/';
        }

        std::string requestRemoteInfoBody =
            "{"
                "'method': 'getRemoteControllerInfo',"
                "'id': 54,"
                "'params': [],"
                "'version': '1.0'"
            "}";

        HTTPRequest req(HTTPRequest::HTTP_POST, path, HTTPMessage::HTTP_1_1);
        req.setContentLength(requestRemoteInfoBody.length());
        req.setContentType("application/json");
        req.add("X-Auth-PSK", "aura-sony");
        session.sendRequest(req) << requestRemoteInfoBody;

        HTTPResponse res;
        std::cout<<"getRemoteControllerInfo: "<<res.getStatus()<<" "<<res.getReason()<<"\n";

        std::istream &is = session.receiveResponse(res);

        //StreamCopier::copyStream(is, std::cout);
        //std::cout<<ss.str();

        std::stringstream responseStream;
        Poco::StreamCopier::copyStream(is, responseStream);
        if(!responseStream.str().empty()) {
            return responseStream.str();
        }


        /*
        // Print prettified JSON Response
        StringBuffer buf;
        Writer<StringBuffer> writer(buf);
        sonyDocJSON.Accept(writer);
        std::cout<<"OG JSON :\n\t"<<buf.GetString()<<"\n";
        */

    }
    catch(Poco::Exception &e) {
        std::cerr<<e.displayText()<<"\n";
    }
    std::cout<<"\ngetRemoteControlInfo() - Response Error\n";
    return NULL;
}



std::string getIRCC(std::string command, std::string response) {
    /* 
        {
            "result":[
                {},
                [
                    {
                        "name": "command",
                        "value": "value"
                    },
                    {
                        "name": "command",
                        "value": "value"
                    },
                    .
                    .
                    .
                ] // result[1]
            ] // result
        }
    */
    rapidjson::Document sonyDocJSON;
    sonyDocJSON.Parse(response.c_str());
    //rapidjson::ParseResult rapidError = sonyDocJSON;

    // Check Parsing
    if(sonyDocJSON.HasParseError()) {
        std::cout<<"\ngetIRCC() : JSON Parse Error\n";
        return "";
    }

    // Check endpoint error
    if(sonyDocJSON.HasMember("error")) {
        rapidjson::StringBuffer buf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
        sonyDocJSON.Accept(writer);
        std::cout<<"getIRCC() - Remote Info Error :\n\t"<<buf.GetString()<<"\n";
        return "";
    }

    // Search IRCC Code for Command
    // If found, return IRCC code, else return COMMAND_NOT_FOUND
    for(auto& v : sonyDocJSON["result"][1].GetArray()) {
        if(v["name"].GetString() == command) {
            std::cout<<"\ngetIRCC() - Command found : "<<v["value"].GetString()<<"\n";
            return v["value"].GetString();
        }
    }
   return "COMMAND_NOT_FOUND";
}

// ++++++++++++++++++++++++++++++++++++++++++++++++


int checkErrorIRCC(const char *response) {
    Poco::XML::DOMParser parser;
    Poco::AutoPtr<Poco::XML::Document> xmlResponse;
    
    try {
        xmlResponse = parser.parseString(response);
        Poco::XML::Element *el = xmlResponse->documentElement();

        const std::vector<std::string>& errCode{"s:Body", "s:Fault", "detail", "UPnPError", "errorCode"};
        const std::vector<std::string>& errDesc{"s:Body", "s:Fault", "detail", "UPnPError", "errorDescription"};
        
        if(getThroughPathElement(el, {"s:Body", "u:X_SendIRCCResponse"})){
            std::cout<<"\nSuccessful Sony IRCC Request\n";
        }
        else if(getThroughPathElement(el, errCode) && getThroughPathElement(el, errDesc)) {
            std::cout<<"\nError in Sony IRCC Request\n\t";
            std::cout<<getThroughPathElement(el, errCode)->innerText();
            std::cout<<" : ";
            std::cout<<getThroughPathElement(el, errDesc)->innerText()<<"\n";
        }
        return 0;
    }
    catch(Poco::Exception &e) {
        throw e;
    }
    return -1;
}


void sendIRCCRequest(std::string tv_ip, std::string irccCode) {
try {

        // "http://localhost:9090/"
        // "http://192.168.137.25/sony/system"
        std::string requestStr = "http://" + tv_ip + "/sony/ircc";

        Poco::URI uri(requestStr);
        HTTPClientSession session(uri.getHost(), uri.getPort());

        std::string path (uri.getPathAndQuery());
        if(path.empty()) {
            path = '/';
        }
        std::string reqBody =
            "<s:Envelope\
                xmlns:s='http://schemas.xmlsoap.org/soap/envelope/'\
                s:encodingStyle='http://schemas.xmlsoap.org/soap/encoding/'>\
                <s:Body>\
                    <u:X_SendIRCC xmlns:u='urn:schemas-sony-com:service:IRCC:1'>\
                        <IRCCCode>" + irccCode + "</IRCCCode>\
                    </u:X_SendIRCC>\
                </s:Body>\
            </s:Envelope>";

        HTTPRequest req(HTTPRequest::HTTP_POST, path, HTTPMessage::HTTP_1_1);
        req.setContentLength(reqBody.length());
        req.setContentType("text/xml; charset=UTF-8");
        req.setKeepAlive(true);
        req.add("X-Auth-PSK", "aura-sony");
        req.add("SOAPACTION", "\"urn:schemas-sony-com:service:IRCC:1#X_SendIRCC\"");
        session.sendRequest(req) << reqBody;

        HTTPResponse res;
        std::cout<<"\n"<<res.getStatus()<<" "<<res.getReason();

        std::istream &is = session.receiveResponse(res);

        std::cout<<"\nTV Response:";
        /* StreamCopier::copyStream(is, std::cout);
        std::cout<<"\n"; */

        std::stringstream xmlSS;
        Poco::StreamCopier::copyStream(is, xmlSS);
        if(checkErrorIRCC(xmlSS.str().c_str()) == -1) {
            std::cout<<"\nXML Parsing error\n";
        }
    }
    catch(Poco::Exception &ex) {
        std::cerr<<ex.displayText()<<"\n";
    }
}

Poco::XML::Element* getThroughPathElement(Poco::XML::Element *e, const std::vector<std::string>& stringPath) {
    for(int i=0; i<stringPath.size(); ++i) {
        if(e->getChildElement(stringPath.at(i))) {
            e = e->getChildElement(stringPath.at(i));
            if(e->tagName() == stringPath.back()) {
                return e;
            } // if tagName matches the std::string path
        } // if child element is present
    } // i
    return NULL;
} // getThroughPathElement

void sonyWol(char **args) {
    std::string reqBody;
    std::string val = args[3];
    if(strcmp(args[3], "check") == 0) {
        reqBody =
        "{\
            \"method\": \"getWolMode\",\
            \"id\": 50,\
            \"params\": [],\
            \"version\": \"1.0\"\
        }";
    }
    else {
        reqBody =
        "{\
            \"method\": \"setWolMode\",\
            \"id\": 55,\
            \"params\": [{\"enabled\":" + val + "}],\
            \"version\": \"1.0\"\
        }";
    }

    std::string requestStr = "http://";
    requestStr.append(args[1]);
    requestStr.append("/sony/system");

    Poco::URI uri(requestStr);
    HTTPClientSession session(uri.getHost(), uri.getPort());

    std::string path (uri.getPathAndQuery());
    if(path.empty()) {
        path = '/';
    }

    HTTPRequest req(HTTPRequest::HTTP_POST, path, HTTPMessage::HTTP_1_1);
    req.setContentLength(reqBody.length());
    req.setContentType("application/json");
    req.setKeepAlive(true);
    req.add("X-Auth-PSK", "aura-sony");
    session.sendRequest(req) << reqBody;

    HTTPResponse res;
    std::cout<<res.getStatus()<<" "<<res.getReason()<<"\n"<<"Response:\n";

    std::istream &is = session.receiveResponse(res);
    std::stringstream ss;
    Poco::StreamCopier::copyStream(is, std::cout);
    std::cout<<"\n";
}

int main(int argc, char **argv) {
    std::string ip_addr = argv[1];
    if(strcmp(argv[2], "wol") == 0) {
        sonyWol(argv);
    } else {
        std::string command = argv[2];
        sendIRCCRequest(ip_addr, getIRCC(command, getRemoteControlInfo(ip_addr)));
    }
    return 0;
}