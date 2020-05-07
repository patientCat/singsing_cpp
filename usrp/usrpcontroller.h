#ifndef USRPCONTROLLER_H
#define USRPCONTROLLER_H

#include "container/circularbufferthreadsafe.h"
#include "singletonthreadsafe.h"
#include "synchronization/countdownlatch.h"

#include <string>
#include <complex>

#include <uhd.h>
#include <uhd/stream.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <boost/noncopyable.hpp>
#include <memory>

//   首先这里需要提供系列的设置
class UsrpController
    : public boost::noncopyable {
public:
    UsrpController();
    /*UsrpObj(const std::string name);*/
    void create(const std::string &args = "");
    void setClockRate(float rate);
    void checkLockedSensor();
    void setAntenna(const std::string &ant = "TX/RX") {
        setRxAntenna(ant);
        setTxAntenna(ant);
    }
    void setRxAntenna(const std::string &ant);
    void setTxAntenna(const std::string &ant);
    void setClockSource(const std::string &source = "internal");
    void setRxSampleRate(float sampRate);
    void setTxSampleRate(float smapRate);
    void setRxCenterFreq(float centerFreq);
    void setTxCenterFreq(float centerFreq);
    void setRxGain(float gain = 20);
    void setTxGain(float gain = 20);
    // fc64 代表float，fc32代表float
    // void SetRxStreamer(const std::string &cpuFormat = "fc32", const std::string &wireFormat = "sc16");
    // usrp最大的带宽设置为40M，最大采样速率为40M
    typedef std::function<uhd::sensor_value_t(const std::string &)> get_sensor_fn_t;


    void setRxBandwidth(float band);
    void setTxBandwidth(float band);

    // 想想怎么设计这里的send和receive
    // send 好弄, send这里应该也是需要一个循环队列来做
    void setTxBufferSize(size_t bufferSize);
    void send(std::complex<float> *buff, size_t n);
    void sendAsync(std::complex<float> *buff, size_t n);

    // 这里是用来设计连续接收的
    void setRxBufferSize(size_t bufferSize);
    void receiveContinue();
    // 偷看这么多元素，但是不取走
    void peek(std::vector<std::complex<float> > &toReceive);
    void retrieve(int len);
    // 会读走这些元素
    void read(std::vector<std::complex<float>> &toReceive);


    // 接收n个symbol，一个symbol是一个复数。
    void receiveNSymbol(std::complex<float> *buff, size_t n);
private:
    void startSendThread();
    void sendThreadRoutine();
    void printInfo();

    bool check_locked_sensor(std::vector<std::string> sensor_names,
                             const char *sensor_name,
                             get_sensor_fn_t get_sensor_fn,
                             double setup_time);

    uhd::usrp::multi_usrp::sptr usrp_;
    uhd::rx_streamer::sptr rxStream_;
    uhd::tx_streamer::sptr txStream_;

    std::unique_ptr<CirularBufferThreadSafe<std::complex<float>>> receiveBuffer_;
    std::unique_ptr<CirularBufferThreadSafe<std::complex<float>>> sendBuffer_;

    std::unique_ptr<std::thread> sendThread_;
    int frameSize_;
    CountDownLatch latch_{1};
};



template class SingletonThreadSafe<UsrpController>;
#define gUsrpController SingletonThreadSafe<UsrpController>::getInstance()

#endif // USRPCONTROLLER_H
