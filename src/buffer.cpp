#include "buffer.hpp"

#include <stdexcept>

#include <cassert>

namespace h4net {

Buffer::Buffer(): lastMsgSize(0)
{
}

void Buffer::Push(const Bytes& input, Size size)
{
    Size pos = 0;
    const Size inputSize = std::min(size, input.size());
    while (inputSize > pos) {
        if (lastMsgSize) {
            assert(!msgQueue.empty());
            auto& lastMsg = msgQueue.back();
            assert(lastMsgSize > lastMsg.size());
            const auto nextPos = pos + std::min(lastMsgSize - lastMsg.size(), inputSize - pos);
            lastMsg.insert(lastMsg.end(), input.begin() + pos, input.begin() + nextPos);
            pos = nextPos;
            lastMsgSize = GetSize(lastMsg);
            if (lastMsgSize == lastMsg.size())
                lastMsgSize = 0;
        } else {
            lastMsgSize = GetSize(input, pos);
            if (!lastMsgSize)
                throw std::runtime_error("Invalid input");
            const auto nextPos = pos + std::min(lastMsgSize, inputSize - pos);
            msgQueue.emplace(input.begin() + pos, input.begin() + nextPos);
            pos = nextPos;
            const auto& lastMsg = msgQueue.back();
            if (lastMsgSize == lastMsg.size())
                lastMsgSize = 0;
        }
    }
}

Buffer::Bytes Buffer::Pop()
{
    Bytes output;
    if (msgQueue.empty() || (msgQueue.size() == 1 && lastMsgSize))
        return output;
    msgQueue.front().swap(output);
    msgQueue.pop();
    return output;
}

void Buffer::Clear()
{
    lastMsgSize = 0;
    std::queue<Bytes> newMsgQueue;
    msgQueue.swap(newMsgQueue);
}

// static
Buffer::Size Buffer::GetSize(const Bytes& msg, Size pos)
{
    if (msg.size() <= pos)
        return 1;
    switch (msg[pos]) {
        case 0x01: // HCI Command packet
        case 0x03: // HCI Synchronous Data packet
            return 4 + (msg.size() - pos < 4 ? 0 : msg[pos + 3]);
        case 0x02: // HCI ACL Data packet
            return 5 + (msg.size() - pos < 5 ? 0 : msg[pos + 4] << 8 | msg[pos + 3]);
        case 0x04: // HCI Event packet
            return 3 + (msg.size() - pos < 3 ? 0 : msg[pos + 2]);
        case 0x05: // HCI ISO Data packet
            return 5 + (msg.size() - pos < 5 ? 0 : (msg[pos + 4] & 0x3f) << 8 | msg[pos + 3]);
        default:
            return 0;
    }
}

} // namespace h4net
