#ifndef H4NET_APP_HPP
#define H4NET_APP_HPP

#include <vector>
#include <memory>

#include <cstdint>

#include <asio.hpp>

#include "buffer.hpp"

namespace h4net {

class App final
{
public:
    App(const std::string&, unsigned int, uint16_t, size_t = 1024);

    App(const App&) = delete;
    App& operator=(const App&) = delete;

    int Run();

public:
    const std::string uartPath;
    const unsigned int uartBaudRate;
    h4net::Buffer ctrlBuff;
    std::vector<uint8_t> fromHostData;
    std::vector<uint8_t> fromCtrlData;
    std::vector<uint8_t> toCtrlData;
    asio::io_context loop;
    asio::signal_set signals;
    asio::serial_port uart;
    asio::ip::tcp::acceptor tcpServer;
    asio::ip::tcp::socket tcpSocket;

    void Start();
    void Stop();

    void ResetTcpServer();
    void HandleSignal(const asio::error_code&, int);
    void AcceptTcpConn(const std::error_code&);
    void FinishReadFromHost(const std::error_code&, size_t);
    void FinishWriteToHost(const std::error_code&, size_t);
    void FinishReadFromCtrl(const std::error_code&, size_t);
    void FinishWriteToCtrl(const std::error_code&, size_t);
    void OpenCtrl();
    void CloseCtrl();
};

} // namespace h4net

#endif
