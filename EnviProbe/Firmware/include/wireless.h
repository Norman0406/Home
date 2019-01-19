#ifndef WIRELESS_H
#define WIRELESS_H

namespace envi_probe
{
class Configuration;

class Wireless
{
public:
    Wireless();

    void connect(Configuration& config);
};
}

#endif // WIRELESS_H