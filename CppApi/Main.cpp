#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <functional>

using namespace boost::asio;
using namespace boost::asio::ip;

// Type alias for request handlers
using RequestHandler = std::function<std::string(const std::string&)>;

// Function to handle a simple GET request for /
std::string handle_root(const std::string& /*body*/) {
    std::string body_content = "Hello world this is my first Api with Boost.Asio ";
    std::string response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: " + std::to_string(body_content.length()) + "\r\n"
        "\r\n"
        + body_content;
    return response;
}
// Function to handle a GET request for /about
std::string handle_about(const std::string& /*body*/) {
    return
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 17\r\n"
        "\r\n"
        "This is the about page.";
}

// Function to handle a GET request for /api/data (example with potential body handling)
std::string handle_api_data(const std::string& /*body*/) {
    // In a real application, you might process the body for POST requests
    return
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: 21\r\n"
        "\r\n"
        "{\"message\": \"Data\"}";
}

// Function to handle 404 Not Found
std::string handle_not_found(const std::string& /*body*/) {
    return
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 13\r\n"
        "\r\n"
        "Not Found\r\n";
}

void handle_request(tcp::socket socket, const std::map<std::string, RequestHandler>& routes) {
    try {
        boost::asio::streambuf buffer;
        boost::asio::read_until(socket, buffer, "\r\n\r\n"); // Read until end of headers

        std::istream request_stream(&buffer);
        std::string line;
        std::getline(request_stream, line);

        std::istringstream request_line_stream(line);
        std::string method, path, http_version;
        request_line_stream >> method >> path >> http_version;

        // Extract the body (if any)
        std::stringstream body_stream;
        while (std::getline(request_stream, line) && line != "\r") {
            // Read headers (you could parse them here if needed)
        }
        // Read the remaining content as the body (simplified for this example)
        body_stream << request_stream.rdbuf();
        std::string body = body_stream.str();

        // Look up the path in the routes map
        auto it = routes.find(path);
        std::string response;
        if (it != routes.end()) {
            response = it->second(body); // Call the handler function
        }
        else {
            response = handle_not_found(body);
        }

        boost::asio::write(socket, boost::asio::buffer(response));
    }
    catch (std::exception& e) {
        std::cerr << "Exception in handle_request: " << e.what() << std::endl;
    }
}

int main() {
    try {
        io_context io_context;
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 8080));

        // Define the routes and their corresponding handlers
        std::map<std::string, RequestHandler> routes;
        routes["/"] = handle_root;
        routes["/about"] = handle_about;
        routes["/api/data"] = handle_api_data;

        std::cout << "Server listening on port 8080..." << std::endl;

        for (;;) {
            tcp::socket socket = acceptor.accept();
            std::thread(handle_request, std::move(socket), routes).detach(); // Pass the routes map
        }
    }
    catch (std::exception& e) {
        std::cerr << "Exception in main: " << e.what() << std::endl;
    }

    return 0;
}