#include <utility>

#ifndef AFINA_NETWORK_ST_NONBLOCKING_CONNECTION_H
#define AFINA_NETWORK_ST_NONBLOCKING_CONNECTION_H

#include <cstring>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <spdlog/logger.h>
#include <afina/Storage.h>
#include "protocol/Parser.h"
#include <afina/execute/Command.h>

namespace Afina {
namespace Network {
namespace STnonblock {

class Connection {
public:
    Connection(int s, std::shared_ptr<spdlog::logger> logger, std::shared_ptr<Afina::Storage> ps)
    : _socket(s)
    , _logger(logger)
    , pStorage(ps)
    {
        std::memset(&_event, 0, sizeof(struct epoll_event));

    }

    inline bool isAlive() const { return _connection_alive; }

    void Start();

    ~Connection();

protected:
    void OnError();
    void OnClose();
    void DoRead();
    void DoWrite();

private:
    friend class ServerImpl;
    // Logger instance
    std::shared_ptr<spdlog::logger> _logger;
    std::shared_ptr<Afina::Storage> pStorage;

    bool _connection_alive;

    std::string _results;

    int _socket;
    struct epoll_event _event;

    Protocol::Parser parser;
    std::size_t arg_remains;
    std::string argument_for_command;
    std::unique_ptr<Execute::Command> command_to_execute;
};

} // namespace STnonblock
} // namespace Network
} // namespace Afina

#endif // AFINA_NETWORK_ST_NONBLOCKING_CONNECTION_H
