#pragma once

#include <exception>
#include <string>

namespace envi_probe
{
class Exception
    : public std::exception
{
public:
    virtual ~Exception() noexcept = default;

    const char* what() const noexcept override;
    bool restart() const noexcept;

protected:
    Exception(std::string message, bool restart = false);

private:
    std::string m_message;
    bool m_restart;
};

class ConfigException
    : public Exception
{
public:
    ConfigException(std::string message);
};

class SensorException
    : public Exception
{
public:
    SensorException(std::string message);
};

class WifiException
    : public Exception
{
public:
    WifiException(std::string message);
};

class NetworkException
    : public Exception
{
public:
    NetworkException(std::string message);
};
}
