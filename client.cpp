#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;

struct loc
{
    float x = 0;
    float y = 0;
    float z = 0;
};

int main() {
    try
    {
        net::io_context ioc;
        tcp::resolver resolver{ ioc };
        websocket::stream<tcp::socket> ws{ ioc };

        auto const results = resolver.resolve("localhost", "8080");
        net::connect(ws.next_layer(), results.begin(), results.end());
        ws.handshake("localhost", "/");
        std::cout << "Connected to server!\n";

        std::thread listener([&ws]()
        {
            try
            {
                beast::flat_buffer buffer;
                while (true)
                {
                    ws.read(buffer);
                    std::string msg = beast::buffers_to_string(buffer.data());
                    buffer.consume(buffer.size());
                    std::cout << "\nserver: " << msg << std::flush;
                }
            }
            catch (std::exception const& e)
            {
                std::cerr << "\nlistener stopped: " << e.what() << std::endl;
            }
        });


        struct loc loc;

        while (true)
        {
            loc.x += 1.0f;
            loc.y += 2.0f;
            loc.z += 3.0f;

            std::ostringstream oss;
            oss << std::fixed << std::setprecision(2);
            oss << loc.x << ", " << loc.y << ", " << loc.z;

            std::string position = oss.str();
            ws.write(net::buffer(position));

            std::cout << "\nyou pos: " << position << "\r" << std::flush;

            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        listener.join();
        ws.close(websocket::close_code::normal);
    }
    catch (std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}