#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <vector> // メッセージ履歴を保存するためのベクター

using boost::asio::ip::tcp;

// メッセージ履歴を保存するグローバル変数
std::vector<std::string> message_history;

// メッセージ履歴を表示する関数
void display_message_history() {
    std::cout << "\n--- Message History ---" << std::endl;
    for (const auto& message : message_history) {
        std::cout << message << std::endl;
    }
    std::cout << "------------------------\n" << std::endl;
}

void handle_client(tcp::socket socket) {
    try {
        for (;;) {
            char data[1024];
            std::memset(data, 0, sizeof(data));

            // クライアントからのメッセージを受信
            size_t length = socket.read_some(boost::asio::buffer(data));
            std::string client_message = std::string(data, length);

            // 受信したメッセージを履歴に追加
            message_history.push_back("Client: " + client_message);

            // 履歴を表示
            display_message_history();

            // サーバーからの返信（固定メッセージを送信）
            std::string server_response = "Message received: " + client_message;
            message_history.push_back("Server: " + server_response); // サーバーの返信も履歴に追加
            boost::asio::write(socket, boost::asio::buffer(server_response));
        }
    }
    catch (std::exception& e) {
        std::cerr << "Connection closed: " << e.what() << std::endl;
    }
}

int main() {
    try {
        boost::asio::io_context io_context;

        // サーバーソケットの準備
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 12345));
        std::cout << "Server is running. Waiting for a connection..." << std::endl;

        // クライアント接続を待つ
        tcp::socket socket(io_context);
        acceptor.accept(socket);
        std::cout << "Client connected!" << std::endl;

        // クライアントとの通信を処理
        handle_client(std::move(socket));
    }
    catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}