#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <unordered_set>
#include <memory>
#include<deque>
#include<string>

using boost::asio::ip::tcp;
using namespace std;

class ChatSession : public enable_shared_from_this<ChatSession> {
public:
    ChatSession(tcp::socket socket, unordered_set<shared_ptr<ChatSession>>& sessions)
        : socket_(move(socket)), sessions_(sessions) {
    }

    void start() {
        sessions_.insert(shared_from_this());
        do_read();
    }

    void deliver(const string& message) {
        auto self(shared_from_this());
        boost::asio::post(socket_.get_executor(), [this, self, message]() {
            bool write_in_progress = !write_msgs_.empty();
            write_msgs_.push_back(message);
            if (!write_in_progress) {
                do_write();
            }
            });
    }

private:
    void do_read() {
        auto self(shared_from_this());
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
            [this, self](boost::system::error_code ec, size_t length) {
                if (!ec) {
                    string message(data_, length);
                    broadcast(message);
                    do_read();
                }
                else {
                    sessions_.erase(shared_from_this());
                }
            });
    }

    void do_write() {
        auto self(shared_from_this());
        boost::asio::async_write(socket_, boost::asio::buffer(write_msgs_.front()),
            [this, self](boost::system::error_code ec, size_t /*length*/) {
                if (!ec) {
                    write_msgs_.pop_front();
                    if (!write_msgs_.empty()) {
                        do_write();
                    }
                }
                else {
                    sessions_.erase(shared_from_this());
                }
            });
    }

    void broadcast(const string& message) {
        for (auto& session : sessions_) {
            session->deliver(message);
        }
    }

    tcp::socket socket_;
    unordered_set<shared_ptr<ChatSession>>& sessions_;
    enum { max_length = 1024 };
    char data_[max_length];
    deque<string> write_msgs_;
};

class ChatServer {
public:
    ChatServer(boost::asio::io_context& io_context, const tcp::endpoint& endpoint)
        : acceptor_(io_context, endpoint) {
        do_accept();
    }

private:
    void do_accept() {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket) {
                if (!ec) {
                    make_shared<ChatSession>(move(socket), sessions_)->start();
                }
                do_accept();
            });
    }

    tcp::acceptor acceptor_;
    unordered_set<shared_ptr<ChatSession>> sessions_;
};

int main() {
    try {
        boost::asio::io_context io_context;
        tcp::endpoint endpoint(tcp::v4(), 12345);
        ChatServer server(io_context, endpoint);
        io_context.run();
    }
    catch (exception& e) {
        cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}