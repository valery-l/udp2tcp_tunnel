#pragma once

namespace network
{

typedef function<void (const void*, size_t)>    on_receive_f;
typedef function<void (error_code const&)>      on_error_f  ;

struct endpoint
{
    endpoint(string address = "", size_t port = 0)
        : addr  (address.empty() ? ip::address_v4::any() : ip::address_v4::from_string(address))
        , port  (port)
    {}

    endpoint(ip::address_v4 address, size_t port = 0)
        : addr  (address)
        , port  (port   )
    {}

    endpoint(const char* bytes)
        : addr  (*reinterpret_cast<const uint32_t*>(bytes))
        , port  (*reinterpret_cast<const uint16_t*>(bytes + sizeof(uint32_t)))
    {
    }

    operator tcp::endpoint() const { return tcp::endpoint(addr, port); }
    operator udp::endpoint() const { return udp::endpoint(addr, port); }

    /////
    static size_t serialize_size()
    {
        return sizeof(uint32_t) + sizeof(uint16_t);
    }

    template<class iter>
    void serialize(iter it) const
    {
        const uint32_t saddr = static_cast<uint32_t>(addr.to_ulong());
        const uint16_t sport = static_cast<uint16_t>(port);

        const char* addr_buf = reinterpret_cast<const char*>(&saddr);
        const char* port_buf = reinterpret_cast<const char*>(&sport);

        std::copy(addr_buf, addr_buf + sizeof(saddr), it);
        std::copy(port_buf, port_buf + sizeof(sport), it);
    }

    ip::address_v4  addr;
    size_t          port;
};

inline std::ostream& operator<<(std::ostream& o, endpoint const& point)
{
    return o << point.addr.to_string() << ":" << point.port;
}

inline bool operator<(endpoint const& lhs, endpoint const& rhs)
{
    return  lhs.addr < rhs.addr ||
               (lhs.addr == rhs.addr &&
                lhs.port  < rhs.port);
}


/////////////////////////////////////////////////////////////

} // namespace network
