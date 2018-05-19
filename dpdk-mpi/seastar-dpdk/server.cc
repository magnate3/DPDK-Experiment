#include "core/reactor.hh"
#include "core/app-template.hh"
#include "core/temporary_buffer.hh"
#include "core/distributed.hh"
#include <vector>
#include <iostream>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

using namespace seastar;

size_t BUFFER_SIZE = 64;
#include "testIpcShardMemory.h"
// static shared_use_st *pShardStuff;


class tcp_server {
    std::vector<server_socket> _tcp_listeners;
    std::vector<server_socket> _sctp_listeners;
public:
    future<> listen(ipv4_addr addr) {
            listen_options lo;
            lo.proto = transport::TCP;
            lo.reuse_address = true;
            _tcp_listeners.push_back(engine().listen(make_ipv4_address(addr), lo));
            do_accepts(_tcp_listeners);
        return make_ready_future<>();
    }

    // FIXME: We should properly tear down the service here.
    future<> stop() {
        return make_ready_future<>();
    }

    void do_accepts(std::vector<server_socket>& listeners) {
        int which = listeners.size() - 1;
        listeners[which].accept().then([this, &listeners] (connected_socket fd, socket_address addr) mutable {
            auto conn = new connection(*this, std::move(fd), addr);
            conn->process().then_wrapped([conn] (auto&& f) {
                delete conn;
                try {
                    f.get();
                } catch (std::exception& ex) {
                    std::cout << "request error " << ex.what() << "\n";
                }
            });
            do_accepts(listeners);
        }).then_wrapped([] (auto&& f) {
            try {
                f.get();
            } catch (std::exception& ex) {
                std::cout << "accept failed: " << ex.what() << "\n";
            }
        });
    }
    class connection {
        connected_socket _fd;
        input_stream<char> _read_buf;
        output_stream<char> _write_buf;
    public:
        connection(tcp_server& server, connected_socket&& fd, socket_address addr)
            : _fd(std::move(fd))
            , _read_buf(_fd.input())
            , _write_buf(_fd.output()) {}
        future<> process() {
             return read();
        }
        future<> read() {
            if (_read_buf.eof()) {
                return make_ready_future();
            }
            // Expect 4 bytes cmd from client
            return _read_buf.read().then([this] (temporary_buffer<char> buf) {
                if (buf.size() == 0) {
                    return make_ready_future();
                }
                auto cmd = std::string(buf.get(), buf.size());
                    return _write_buf.write(cmd).then([this] {
                        return _write_buf.flush();
                    }).then([this] {
                        return this->read();
                    });
            });
        }

    };
};

namespace bpo = boost::program_options;

int main(int ac, char** av) {
    app_template app;
    app.add_options()
        ("port", bpo::value<uint16_t>()->default_value(1234), "TCP server port") // add port
        ("tcp", bpo::value<std::string>()->default_value("yes"), "tcp listen") // add tcp
        ("sctp", bpo::value<std::string>()->default_value("no"), "sctp listen"); // enable sctp



    return app.run_deprecated(ac, av, [&] {
        auto&& config = app.configuration();
        uint16_t port = config["port"].as<uint16_t>(); // assign the port value from the app_template
        auto server = new distributed<tcp_server>; // run distributed object
        // The distributed template manages a sharded service,
        // by creating a copy of the service on each shard, providing mechanisms
        // to communicate with each shard's copy, and a way to stop the service.


        //Starts Service by constructing an instance on every logical core with a copy of args passed to the constructor.
        server->start().then([server = std::move(server), port] () mutable {
            engine().at_exit([server] {
                return server->stop();
            });
            server->invoke_on_all(&tcp_server::listen, ipv4_addr{port});
            // Invoke a method on all Service instances in parallel.
        }).then([port] {
            std::cout << "Seastar TCP server listening on port " << port << " ...\n";
        });
    });
}
