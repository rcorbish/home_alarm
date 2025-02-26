
#include <ostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <chrono>
#include <thread>
#include <iostream>

#include "mongoose.h"
#include "radio.hpp"
#include "signal_processor.hpp"

constexpr const char *s_http_port = "https://0.0.0.0:8111";
constexpr const char *CertFileName = "cert.pem";
constexpr const char *KeyFileName = "key.pem";

constexpr int  SecurityPort = 9111 ;
constexpr int  Buffer_Size = 1024 ;

struct mg_tls_opts tls_opts;
struct mg_http_serve_opts html_opts;
struct mg_http_serve_opts css_opts;
struct mg_http_serve_opts pdf_opts;
struct mg_http_message home;

Radio * radio = nullptr;

using namespace std ;

void ev_handler(struct mg_connection *nc, int ev, void *ev_data);

void my_handler(int s) {
	cout << "  Closing radio" << endl ; 
	delete radio ;
	exit( 0 ) ;
}

int main( int argc, char *argv[] ){
	signal( SIGINT, my_handler ) ;

    memset( &tls_opts, 0, sizeof(tls_opts));

    tls_opts.cert = mg_file_read(&mg_fs_posix, CertFileName );
    tls_opts.key = mg_file_read(&mg_fs_posix, KeyFileName );

    memset( &html_opts, 0, sizeof(html_opts));
    html_opts.mime_types = "html=text/html";
    html_opts.extra_headers = "Content-Type: text/html\nServer: Radio\r\n";

    memset( &css_opts, 0, sizeof(css_opts));
    css_opts.mime_types = "html=text/css";
    css_opts.extra_headers = "Content-Type: text/css\nServer: Radio\r\n";

    memset( &home, 0, sizeof(home));

    struct mg_mgr mgr;
    struct mg_connection *nc;
    const char *err_str;

    mg_mgr_init( &mgr ) ;

    nc = mg_http_listen(&mgr, s_http_port, ev_handler, nullptr ) ;
    if (nc == nullptr) {
        std::cerr << "Error starting server on port " << s_http_port << std::endl ;
        exit( 1 ) ;
    }

    radio = Radio::getMonitoringInstance() ;
    radio->start() ;

    std::cout << "Starting RESTful server on port " << s_http_port << std::endl ;
    for (;;) {
        mg_mgr_poll(&mgr, 1000);
    }
    mg_mgr_free(&mgr);

    return 0;
}
    

void ev_handler(struct mg_connection *nc, int ev, void *ev_data ) {
    struct http_message *hm = (struct http_message *)ev_data;

    // const auto args = (const Args *)nc->fn_data;

    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *msg = (struct mg_http_message*)ev_data;  
        if( mg_match(msg->uri, mg_str("/history"), nullptr ) ) {
            std::string s("TEST");
            mg_http_reply(nc, 200, "Content-Type: application/json\nServer: Sprinklers\r\n", "%s", s.c_str() ) ;
        } else {
            char addr_buf[256];
            const auto len = mg_snprintf( addr_buf, sizeof(addr_buf), "%.*s, Bad call from %M", (int)msg->uri.len, msg->uri.buf,  mg_print_ip, &nc->rem );
            std::cerr << addr_buf << std::endl;
            mg_http_reply(nc, 400, nullptr, "");
        }
    } else if (ev == MG_EV_ACCEPT) {
        mg_tls_init( nc, &tls_opts);
    } else if (ev == MG_EV_ERROR) {
        std::cerr << "Error: " << (char *) ev_data << std::endl ;
    }
}

