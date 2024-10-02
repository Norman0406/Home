#include "exceptions.h"

namespace envi_probe {
Exception::Exception(std::string message, bool restart)
    : m_message{std::move(message)}, m_restart{restart} {}

const char* Exception::what() const noexcept { return m_message.c_str(); }

bool Exception::restart() const noexcept { return m_restart; }

ConfigException::ConfigException(std::string message)
    : Exception(std::move(message), true) {}

SensorException::SensorException(std::string message)
    : Exception(std::move(message), true) {}

WifiException::WifiException(std::string message)
    : Exception(std::move(message), true) {}

NetworkException::NetworkException(std::string message)
    : Exception(std::move(message), true) {}
}  // namespace envi_probe
