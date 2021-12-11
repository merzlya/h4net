#include "app.hpp"

#include <functional>
#include <chrono>

using namespace std::placeholders;
using namespace std::literals::chrono_literals;

namespace h4net {

App::App(const std::string& uartPath, unsigned int uartBaudRate, uint16_t tcpSrvPort, size_t bufferSize):
    uartPath(uartPath),
    uartBaudRate(uartBaudRate),
    fromHostData(bufferSize, 0),
    fromCtrlData(bufferSize, 0),
    loop(1),
    signals(loop),
    uart(loop),
    tcpServer(loop, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), tcpSrvPort)),
    tcpSocket(loop)
{
}

void App::Start()
{
    signals.add(SIGINT);
    signals.add(SIGTERM);
    signals.async_wait(std::bind(&App::HandleSignal, this, _1, _2));

    tcpServer.async_accept(tcpSocket, std::bind(&App::AcceptTcpConn, this, _1));
}

void App::Stop()
{
    tcpSocket.close();
    tcpServer.close();
    signals.cancel();
    signals.clear();
    CloseCtrl();

    asio::post(loop, std::bind(&asio::io_context::stop, &loop));
}

void App::OpenCtrl()
{
    uart.open(uartPath);
    uart.set_option(asio::serial_port::flow_control(asio::serial_port::flow_control::type::hardware));
    uart.set_option(asio::serial_port::baud_rate(uartBaudRate));
    uart.set_option(asio::serial_port::parity(asio::serial_port::parity::type::none));
    uart.set_option(asio::serial_port::stop_bits(asio::serial_port::stop_bits::type::one));
    uart.set_option(asio::serial_port::character_size(8));
    FinishWriteToHost(std::error_code(), 0);
}

void App::CloseCtrl()
{
    uart.close();
}

void App::HandleSignal(const asio::error_code& ec, int)
{
    if (ec == asio::error::operation_aborted)
        return;
    if (ec)
        throw std::runtime_error("Failed to handle signal [" + ec.message() + "]");
    asio::post(loop, std::bind(&App::Stop, this));
}

void App::ResetTcpServer()
{
    CloseCtrl();
    tcpSocket.close();
    ctrlBuff.Clear();
    tcpServer.async_accept(tcpSocket, std::bind(&App::AcceptTcpConn, this, _1));
}

void App::AcceptTcpConn(const std::error_code& ec)
{
    if (ec == asio::error::operation_aborted)
        return;
    if (ec)
        throw std::runtime_error("TCP server failed to accept connection [" + ec.message() + "]");
    tcpSocket.async_read_some(asio::buffer(fromHostData), std::bind(&App::FinishReadFromHost, this, _1, _2));
    OpenCtrl();
}

void App::FinishReadFromHost(const std::error_code& ec, size_t size)
{
    if (ec == asio::error::operation_aborted)
        return;
    if (ec || !size)
        return ResetTcpServer();
    ctrlBuff.Push(fromHostData, size);
    toCtrlData = ctrlBuff.Pop();
    if (!toCtrlData.empty())
        asio::async_write(uart, asio::buffer(toCtrlData), std::bind(&App::FinishWriteToCtrl, this, _1, _2));
    else
        tcpSocket.async_read_some(asio::buffer(fromHostData), std::bind(&App::FinishReadFromHost, this, _1, _2));
}

void App::FinishWriteToHost(const std::error_code& ec, size_t)
{
    if (ec == asio::error::operation_aborted)
        return;
    if (ec)
        return ResetTcpServer();
    uart.async_read_some(asio::buffer(fromCtrlData), std::bind(&App::FinishReadFromCtrl, this, _1, _2));
}

void App::FinishReadFromCtrl(const std::error_code& ec, size_t size)
{
    if (!tcpSocket.is_open() || ec == asio::error::operation_aborted)
        return;
    if (ec)
        return asio::post(loop, std::bind(&App::Stop, this));
    asio::async_write(tcpSocket, asio::buffer(fromCtrlData, size), std::bind(&App::FinishWriteToHost, this, _1, _2));
}

void App::FinishWriteToCtrl(const std::error_code& ec, size_t)
{
    if (!tcpSocket.is_open() || ec == asio::error::operation_aborted)
        return;
    if (ec)
        return asio::post(loop, std::bind(&App::Stop, this));
    toCtrlData = ctrlBuff.Pop();
    if (!toCtrlData.empty())
        asio::async_write(uart, asio::buffer(toCtrlData), std::bind(&App::FinishWriteToCtrl, this, _1, _2));
    else
        tcpSocket.async_read_some(asio::buffer(fromHostData), std::bind(&App::FinishReadFromHost, this, _1, _2));
}

int App::Run()
{
    Start();
    loop.run();
    loop.restart();
    return 0;
}

} // namespace h4net
