#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/format.hpp>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <utility>

using boost::asio::ip::tcp;
using namespace std;

#define MAX_CONNECTION 5
#define MAX_NP_SERVER 12
#define DOMAIN "cs.nycu.edu.tw"

struct Environment {
    string REQUEST_METHOD = "";
    string REQUEST_URI = "";
    string PATH_INFO = "";
    string QUERY_STRING = "";
    string SERVER_PROTOCOL = "";
    string HTTP_HOST = "";
    string SERVER_ADDR = "";
    string SERVER_PORT = "";
    string REMOTE_ADDR = "";
    string REMOTE_PORT = "";
};

struct ConnectionInfo {
    string host = "";
    string port = "";
    string file = "";
};

boost::asio::io_context io_context;

vector<ConnectionInfo> connections(MAX_CONNECTION);

const string HTTP_OK = "HTTP/1.1 200 OK\r\n";

const string contentType = "Content-Type: text/html\r\n\r\n";

string getPanel() {
    const string formMethod = "GET";
    const string formAction = "console.cgi";
    string contentHead = R"(
        <!DOCTYPE html>
        <html lang="en">
          <head>
            <title>NP Project 3 Panel</title>
            <link
              rel="stylesheet"
              href="https://cdn.jsdelivr.net/npm/bootstrap@4.5.3/dist/css/bootstrap.min.css"
              integrity="sha384-TX8t27EcRE3e/ihU7zmQxVncDAy5uIKz4rEkgIXeMed4M0jlfIDPvg6uqKI2xXr2"
              crossorigin="anonymous"
            />
            <link
              href="https://fonts.googleapis.com/css?family=Source+Code+Pro"
              rel="stylesheet"
            />
            <link
              rel="icon"
              type="image/png"
              href="https://cdn4.iconfinder.com/data/icons/iconsimple-setting-time/512/dashboard-512.png"
            />
            <style>
              * {
                font-family: 'Source Code Pro', monospace;
              }
            </style>
          </head>
          <body class="bg-secondary pt-5">
    )";
    string contentBodyFront = (boost::format(R"(
            <form action="%1%" method="%2%">
              <table class="table mx-auto bg-light" style="width: inherit">
                <thead class="thead-dark">
                  <tr>
                    <th scope="col">#</th>
                    <th scope="col">Host</th>
                    <th scope="col">Port</th>
                    <th scope="col">Input File</th>
                  </tr>
                </thead>
                <tbody>
    )") % formAction % formMethod).str();
    string contentBodyMiddle;
    string hostMenu;
    for (int i = 0; i < MAX_NP_SERVER; i++) {
        hostMenu += (boost::format(R"(
                          <option value="nplinux%1%.%2%">nplinux%1%</option>
        )") % (i + 1) % DOMAIN).str();
    }
    string testCaseMenu;
    for (int i = 0; i < 5; i++) {
        testCaseMenu += (boost::format(R"(
                        <option value="t%1%.txt">t%1%.txt</option>
        )") % (i + 1)).str();
    }
    for (int i = 0; i < MAX_CONNECTION; i++) {
        contentBodyMiddle += (boost::format(R"(
                  <tr>
                    <th scope="row" class="align-middle">Session %1%</th>
                    <td>
                      <div class="input-group">
                        <select name="h%2%" class="custom-select">
                          <option></option>%3%
                        </select>
                        <div class="input-group-append">
                          <span class="input-group-text">.%4%</span>
                        </div>
                      </div>
                    </td>
                    <td>
                      <input name="p%2%" type="text" class="form-control" size="5" />
                    </td>
                    <td>
                      <select name="f%2%" class="custom-select">
                        <option></option>
                        %5%
                      </select>
                    </td>
                  </tr>
        )") % (i + 1) % i % hostMenu % DOMAIN % testCaseMenu).str();
    }
    string contentBodyEnd = R"(
                  <tr>
                    <td colspan="3"></td>
                    <td>
                      <button type="submit" class="btn btn-info btn-block">Run</button>
                    </td>
                  </tr>
                </tbody>
              </table>
            </form>
          </body>
        </html>
    )";
    return contentType + contentHead + contentBodyFront + contentBodyMiddle + contentBodyEnd;
}

string createPanel() {
    string contentHead = R"(
        <!DOCTYPE html>
        <html lang="en">
          <head>
            <meta charset="UTF-8" />
            <title>NP Project 3 Console</title>
            <link
              rel="stylesheet"
              href="https://cdn.jsdelivr.net/npm/bootstrap@4.5.3/dist/css/bootstrap.min.css"
              integrity="sha384-TX8t27EcRE3e/ihU7zmQxVncDAy5uIKz4rEkgIXeMed4M0jlfIDPvg6uqKI2xXr2"
              crossorigin="anonymous"
            />
            <link
              href="https://fonts.googleapis.com/css?family=Source+Code+Pro"
              rel="stylesheet"
            />
            <link
              rel="icon"
              type="image/png"
              href="https://cdn0.iconfinder.com/data/icons/small-n-flat/24/678068-terminal-512.png"
            />
            <style>
              * {
                font-family: 'Source Code Pro', monospace;
                font-size: 1rem !important;
              }
              body {
                background-color: #232731;
              }
              pre {
                color: #D8DEE9;
              }
              b {
                color: #a3be8c;
              }
              th {
                color: #81A1C1;
              }
            </style>
          </head>
    )";
    string contentBodyFront = R"(
          <body>
            <table class="table table-dark table-bordered">
              <thead>
                <tr>
    )";
    for (int i = 0; i < MAX_CONNECTION; i++) {
        if (connections[i].host == "") {
            break;
        }
        contentBodyFront += (boost::format(R"(
                  <th scope="col">%1%:%2%</th>
        )") % connections[i].host % connections[i].port).str();
    }
    string contentBodyMiddle = R"(
                </tr>
              </thead>
              <tbody>
                <tr>
    )";
    for (int i = 0; i < MAX_CONNECTION; i++) {
        if (connections[i].host == "") {
            break;
        }
        contentBodyMiddle += (boost::format(R"(
                  <td><pre id="s%1%" class="mb-0"></pre></td>
        )") % i).str();
    }
    string contentBodyEnd = R"(
                <tr>
              </tbody>
            </table>
          </body>
        </html>
    )";
    return contentType + contentHead + contentBodyFront + contentBodyMiddle + contentBodyEnd;
}

class Client : public std::enable_shared_from_this<Client> {
  public:
    Client(int index, boost::asio::io_context &io_context, shared_ptr<tcp::socket> socket)
        : userIdx_(index), socket_(io_context), webSocket_(socket), resolver_(io_context) {}

    void start() {
        file_.open(("./test_case/" + connections[userIdx_].file), ios::in); // Open file
        doResolve();
    }

  private:
    void doResolve() {
        auto self(shared_from_this());
        resolver_.async_resolve(
            connections[userIdx_].host,
            connections[userIdx_].port,
            [this, self](boost::system::error_code ec, tcp::resolver::iterator it) {
                if (!ec) {
                    doConnect(it);
                }
            });
    }

    void doConnect(tcp::resolver::iterator it) {
        auto self(shared_from_this());
        socket_.async_connect(
            *it,
            [this, self](boost::system::error_code ec) {
                if (!ec) {
                    doRead();
                }
            });
    }

    void doRead() {
        auto self(shared_from_this());
        memset(data_, '\0', max_length); // Clear read data from last time
        socket_.async_read_some(
            boost::asio::buffer(data_, max_length),
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    string content(data_);
                    string output = outputShell(content);
                    writeToWeb(output);

                    // Clear read data
                    memset(data_, '\0', max_length);

                    if (content.find("% ") != string::npos) {
                        writeToNPServer();
                    }
                    else {
                        doRead();
                    }
                }
            });
    }

    void writeToNPServer() {
        auto self(shared_from_this());
        string command = getCommand();
        boost::asio::async_write(
            socket_,
            boost::asio::buffer(command.c_str(), command.length()),
            [this, self](boost::system::error_code ec, std::size_t /*length*/) {
                if (!ec) {
                    doRead();
                }
            });
    }

    void writeToWeb(string content) {
        auto self(shared_from_this());
        boost::asio::async_write(
            *webSocket_,
            boost::asio::buffer(content.c_str(), content.size()),
            [this, self](boost::system::error_code ec, std::size_t /*length*/) {
                if (ec) {
                    cout << "Error writing: " << ec.message() << endl;
                }
            });
    }

    string getCommand() {
        string command;
        if (file_.is_open()) {
            getline(file_, command);
            if (command.find("exit") != string::npos) {
                file_.close();
            }
            command += "\n";
            string output = outputCommand(command);
            writeToWeb(output);
        }
        return command;
    }

    string htmlEscape(string content) {
        boost::replace_all(content, "&", "&amp;");
        boost::replace_all(content, "\"", "&quot;");
        boost::replace_all(content, "\'", "&apos;");
        boost::replace_all(content, "<", "&lt;");
        boost::replace_all(content, ">", "&gt;");
        boost::replace_all(content, "\n", "&NewLine;");
        boost::replace_all(content, "\r", "");
        boost::replace_all(content, " ", "&nbsp;");
        return content;
    }

    string outputShell(string content) {
        string contentEsc = htmlEscape(content);
        return (boost::format("<script>document.getElementById('s%1%').innerHTML += '%2%';</script>") % userIdx_ % contentEsc).str();
    }

    string outputCommand(string content) {
        string contentEsc = htmlEscape(content);
        return (boost::format("<script>document.getElementById('s%1%').innerHTML += '<b>%2%</b>';</script>") % userIdx_ % contentEsc).str();
    }

    int userIdx_;
    tcp::socket socket_;
    shared_ptr<tcp::socket> webSocket_; // Web server socket
    tcp::resolver resolver_;
    fstream file_;
    enum { max_length = 1024 };
    char data_[max_length];
};


class session : public std::enable_shared_from_this<session> {
  public:
    session(tcp::socket socket) : socket_(std::move(socket)) {}

    void start() {
        doRead();
    }

  private:
    void doRead() {
        auto self(shared_from_this());
        socket_.async_read_some(
            boost::asio::buffer(data_, max_length),
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    parseHTTPRequest();
                    doWrite(HTTP_OK);
                    if (envVars.PATH_INFO == "/panel.cgi") {
                        string panel = getPanel();
                        doWrite(panel);
                    }
                    else if (envVars.PATH_INFO == "/console.cgi") {
                        parseQueryString();
                        string console = createPanel();
                        doWrite(console);
                        makeConnection(io_context);
                    }
                    else {
                        cout << "Invalid path: " << envVars.PATH_INFO << endl;
                    }
                }
            });
    }

    void doWrite(string httpContent) {
        auto self(shared_from_this());
        boost::asio::async_write(
            socket_,
            boost::asio::buffer(httpContent.c_str(), httpContent.size()),
            [this, self](boost::system::error_code ec, std::size_t /*length*/) {
                if (ec) {
                    cout << "Error writing: " << ec.message() << endl;
                }
            });
    }

    void parseHTTPRequest() {
        stringstream ss(data_);
        ss >> envVars.REQUEST_METHOD >> envVars.REQUEST_URI >> envVars.SERVER_PROTOCOL;

        string temp;
        ss >> temp;
        if (temp == "Host:") {
            ss >> envVars.HTTP_HOST;
        }

        // Extract query string
        size_t pos = envVars.REQUEST_URI.find("?");
        if (pos != string::npos) {
            envVars.QUERY_STRING = envVars.REQUEST_URI.substr(pos + 1);
            envVars.PATH_INFO = envVars.REQUEST_URI.substr(0, pos);
        }
        else {
            envVars.PATH_INFO = envVars.REQUEST_URI;
        }

        envVars.SERVER_ADDR = socket_.local_endpoint().address().to_string();
        envVars.SERVER_PORT = to_string(socket_.local_endpoint().port());
        envVars.REMOTE_ADDR = socket_.remote_endpoint().address().to_string();
        envVars.REMOTE_PORT = to_string(socket_.remote_endpoint().port());
    }

    void parseQueryString() {
        vector<string> tmp;
        boost::split(tmp, envVars.QUERY_STRING, boost::is_any_of("&"));
        for (unsigned long int i = 0; i < tmp.size(); i++) {
            vector<string> tmp2;
            boost::split(tmp2, tmp[i], boost::is_any_of("="));
            if (tmp2.size() == 2) {
                if (tmp2[0][0] == 'h') {
                    connections[i / 3].host = tmp2[1];
                }
                else if (tmp2[0][0] == 'p') {
                    connections[i / 3].port = tmp2[1];
                }
                else if (tmp2[0][0] == 'f') {
                    connections[i / 3].file = tmp2[1];
                }
            }
        }
    }

    void makeConnection(boost::asio::io_context &io_context) {
        shared_ptr<tcp::socket> webSocket = make_shared<tcp::socket>(std::move(socket_));
        for (int idx = 0; idx < MAX_CONNECTION; idx++) {
            if (connections[idx].host == "") {
                return;
            }

            std::make_shared<Client>(idx, io_context, webSocket)->start();
        }
    }

    tcp::socket socket_;
    enum { max_length = 1024 };
    char data_[max_length];
    Environment envVars;
};

class server {
  public:
    server(boost::asio::io_context &io_context, short port)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
        do_accept();
    }

  private:
    void do_accept() {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket) {
                if (!ec) {
                    std::make_shared<session>(std::move(socket))->start();
                }

                do_accept();
            });
    }

    tcp::acceptor acceptor_;
};

int main(int argc, char *argv[]) {
    try {
        if (argc != 2) {
            std::cerr << "Usage: async_tcp_echo_server <port>\n";
            return 1;
        }

        server s(io_context, std::atoi(argv[1]));

        io_context.run();
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}