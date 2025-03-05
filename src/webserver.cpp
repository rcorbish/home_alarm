
#include <vector>
#include <algorithm>

#include "mongoose.h"
#include "radio.hpp"
#include "signal_processor.hpp"

using namespace std ;

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

static vector<struct mg_connection*> ws_clients;

Radio * radio = nullptr;

void ev_handler(struct mg_connection *nc, int ev, void *ev_data);
void sendWSMessage( const char *msg, const int msg_length ) ;

void my_handler(int s) {
	cout << "  Closing radio" << endl ; 
	delete radio ;
	exit( 0 ) ;
}

int main( int argc, char *argv[] ){
	signal( SIGINT, my_handler ) ;
    signal( SIGPIPE, SIG_IGN ) ;      // in case websocket is closed unexpectedly

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

    radio = Radio::getMonitoringInstance( sendWSMessage) ;
    radio->start() ;

    struct mg_mgr mgr;
    struct mg_connection *nc;
    const char *err_str;

    mg_mgr_init( &mgr ) ;

    nc = mg_http_listen(&mgr, s_http_port, ev_handler, radio) ;
    if (nc == nullptr) {
        std::cerr << "Error starting server on port " << s_http_port << std::endl ;
        exit( 1 ) ;
    }

    std::cout << "Starting RESTful server on port " << s_http_port << std::endl ;
    for (;;) {
        mg_mgr_poll(&mgr, 1000);
    }
    mg_mgr_free(&mgr);

    return 0;
}
    

void ev_handler(struct mg_connection *nc, int ev, void *ev_data ) {
    // const auto args = (const Args *)nc->fn_data;

    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *msg = (struct mg_http_message*)ev_data;  
        if( mg_match(msg->uri, mg_str("/"), nullptr ) ) {
            mg_http_serve_file( nc, &home, "home.html", &html_opts) ;
        } else if( mg_match(msg->uri, mg_str("/signal"), nullptr ) ) {
            mg_ws_upgrade(nc, msg, nullptr);
            ws_clients.push_back(nc);
        } else if( mg_match(msg->uri, mg_str("/css.css"), nullptr ) ) {
            mg_http_serve_file( nc, &home, "css.css", &css_opts);
        } else {
            char addr_buf[256];
            const auto len = mg_snprintf( addr_buf, sizeof(addr_buf), "%.*s, Bad call from %M", (int)msg->uri.len, msg->uri.buf,  mg_print_ip, &nc->rem );
            std::cerr << addr_buf << std::endl;
            mg_http_reply(nc, 400, nullptr, "");
        }
    } else if (ev == MG_EV_WS_MSG) {
        // Got websocket frame. Received data is wm->data. Echo it back!
        struct mg_ws_message *wm = (struct mg_ws_message *) ev_data;
        //sendWSMessage( wm->data.buf, wm->data.len);
    } else if (ev == MG_EV_CLOSE) {
        auto it = find(ws_clients.begin(), ws_clients.end(), nc);
        if (it != ws_clients.end()) {
            ws_clients.erase(it);
        }
    } else if (ev == MG_EV_ACCEPT) {
        mg_tls_init( nc, &tls_opts);
    } else if (ev == MG_EV_ERROR) {
        std::cerr << "Error: " << (char *) ev_data << std::endl ;
    }
}

void sendWSMessage( const char *msg, const int msg_length ) {
    for( auto &c : ws_clients ) {
        try {
            mg_ws_send( c, msg, msg_length, WEBSOCKET_OP_TEXT ) ;
        } catch(... ) {
            std::exception_ptr ex = std::current_exception();
            cerr << ex.__cxa_exception_type() << endl ;
        }
    }
}

