#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <iostream>

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;

int main()
{
    try
    {
        net::io_context ioc;

        tcp::acceptor acceptor{ ioc, tcp::endpoint(tcp::v4(), 8080) };
        tcp::socket socket{ ioc };

        std::cout << "listening on port 8080...\n";

        acceptor.accept(socket);
        std::cout << "client connected!\n";

        websocket::stream<tcp::socket> ws{ std::move(socket) };
        ws.accept();

        beast::flat_buffer buffer;

        while (true)
        {
            try
            {
                ws.read(buffer);
                std::string data = beast::buffers_to_string(buffer.data());
                buffer.consume(buffer.size());

                std::cout << "client1 pos: " << data << "\r" << std::flush;

                ws.text(true);
                ws.write(net::buffer(data));
            }
            catch (const beast::system_error& se)
            {
                if (se.code() == websocket::error::closed)
                {
                    std::cout << "client disconnected.\n";
                    break;
                }
                throw;
            }
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}