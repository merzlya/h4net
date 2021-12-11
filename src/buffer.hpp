#ifndef H4NET_BUFFER_HPP
#define H4NET_BUFFER_HPP

#include <queue>
#include <vector>
#include <limits>

#include <cstdint>

namespace h4net {

class Buffer
{
public:
    using Byte = uint8_t;
    using Bytes = std::vector<Byte>;
    using Size = Bytes::size_type;

    Buffer();

    void Push(const Bytes&, Size = std::numeric_limits<Size>::max());
    Bytes Pop();

    void Clear();

private:
    std::queue<Bytes> msgQueue;
    Size lastMsgSize;

    static Size GetSize(const Bytes&, Size pos = 0);
};

} // namespace h4net

#endif
