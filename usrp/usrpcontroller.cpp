#include "usrpcontroller.h"
#include "loghelper.h"

#include <iostream>
#include <vector>
#include <uhd/types/tune_request.hpp>
#include <uhd/utils/thread.hpp>
#include <uhd/utils/safe_main.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/exception.hpp>
#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <fstream>
#include <csignal>
#include <complex>
#include <thread>
#include <chrono>
#include <mutex>
#include <functional>

using namespace std;

//UsrpObj::UsrpObj(const std::string name)
//{
//	usrp_ = uhd::usrp::multi_usrp::make(name);
//}

UsrpController::UsrpController() {

}

void UsrpController::create(const std::string &args) {
    std::cout << std::endl;
    std::cout << boost::format("Creating the usrp device with: %s...") % args << std::endl;
    try {
        usrp_ = uhd::usrp::multi_usrp::make(args);
    } catch (uhd::key_error &e) {
        cout << boost::format("请插入USRP或者重启程序 %s\n") % e.what();
    }
    printInfo();

    uhd::stream_args_t stream_args("fc32", "sc16");
    std::vector<size_t> channel_nums;
    channel_nums.push_back(boost::lexical_cast<size_t>(0));
    stream_args.channels = channel_nums;
    rxStream_ = usrp_->get_rx_stream(stream_args);
    txStream_ = usrp_->get_tx_stream(stream_args);


    const int defaultBufferSize = 300000;
    setTxBufferSize(defaultBufferSize);
    setRxBufferSize(defaultBufferSize);
}

void UsrpController::setClockRate(float rate) {
    std::cout << boost::format("Setting clock rate: %f Msps...") % (rate / 1e6) << std::endl;
    usrp_->set_master_clock_rate(rate);
    std::cout << boost::format("Actual clock rate: %f Msps...") % (usrp_->get_master_clock_rate() / 1e6) << std::endl << std::endl;
}

void UsrpController::checkLockedSensor() {
    double setup_time = 1;
    std::this_thread::sleep_for(
        std::chrono::milliseconds(int64_t(1000 * setup_time))
    );

    // set the clock source and clock rate
    check_locked_sensor(
        usrp_->get_rx_sensor_names(0),
        "lo_locked",
    [&](const std::string & sensor_name) {
        return usrp_->get_rx_sensor(sensor_name);
    },
    setup_time
    );

}

void UsrpController::setRxAntenna(const string &ant) {
    usrp_->set_tx_antenna(ant);
}

void UsrpController::setTxAntenna(const string &ant) {

    usrp_->set_rx_antenna(ant);
}

void UsrpController::setClockSource(const std::string &source) {
    usrp_->set_clock_source(source);
}

void UsrpController::setRxSampleRate(float sampRate) {
    // set the sample rate
    std::cout << boost::format("Setting RX Rate: %f Msps...") % (sampRate / 1e6) << std::endl;
    usrp_->set_rx_rate(sampRate);
    std::cout << boost::format("Actual RX Rate: %f Msps...") % (usrp_->get_rx_rate() / 1e6) << std::endl << std::endl;
}

void UsrpController::setTxSampleRate(float sampRate) {
    std::cout << boost::format("Setting TX Rate: %f Msps...") % (sampRate / 1e6) << std::endl;
    usrp_->set_tx_rate(sampRate);
    std::cout << boost::format("Actual TX Rate: %f Msps...") % (usrp_->get_tx_rate() / 1e6) << std::endl << std::endl;
}

void UsrpController::setRxCenterFreq(float centerFreq) {
    std::cout << boost::format("Setting RX Freq: %f MHz...") % (centerFreq / 1e6) << std::endl;
    uhd::tune_request_t tune_request(centerFreq);
    usrp_->set_rx_freq(tune_request);
    std::cout << boost::format("Actual RX Freq: %f MHz...") % (usrp_->get_rx_freq() / 1e6) << std::endl << std::endl;
}

void UsrpController::setTxCenterFreq(float centerFreq) {
    std::cout << boost::format("Setting TX Freq: %f MHz...") % (centerFreq / 1e6) << std::endl;
    uhd::tune_request_t tune_request(centerFreq);
    usrp_->set_tx_freq(tune_request);
    std::cout << boost::format("Actual TX Freq: %f MHz...") % (usrp_->get_tx_freq() / 1e6) << std::endl << std::endl;
}

void UsrpController::setTxGain(float gain) {
    std::cout << boost::format("Setting TX Gain: %f dB...") % gain << std::endl;
    usrp_->set_tx_gain(gain);
    std::cout << boost::format("Actual TX Gain: %f dB...") % usrp_->get_rx_gain() << std::endl << std::endl;
}

void UsrpController::setRxGain(float gain) {
    std::cout << boost::format("Setting RX Gain: %f dB...") % gain << std::endl;
    usrp_->set_rx_gain(gain);
    std::cout << boost::format("Actual RX Gain: %f dB...") % usrp_->get_rx_gain() << std::endl << std::endl;
}

//void UsrpObj::SetRxStreamer(const std::string & cpuFormat, const std::string & wireFormat)
//{
//	uhd::stream_args_t stream_args(cpuFormat, wireFormat);
//	rxStream_.reset();
//	rxStream_.swap(usrp_->get_rx_stream(stream_args));
//}

void UsrpController::setRxBandwidth(float band) {
    std::cout << boost::format("Setting RX Bandwidth: %f MHz...") % (band / 1e6) << std::endl;
    usrp_->set_rx_bandwidth(band);
    std::cout << boost::format("Actual RX Bandwidth: %f MHz...") % (usrp_->get_rx_bandwidth() / 1e6) << std::endl << std::endl;
}

void UsrpController::setTxBandwidth(float band) {
    std::cout << boost::format("Setting TX Bandwidth: %f MHz...") % (band / 1e6) << std::endl;
    usrp_->set_tx_bandwidth(band);
    std::cout << boost::format("Actual TX Bandwidth: %f MHz...") % (usrp_->get_tx_bandwidth() / 1e6) << std::endl << std::endl;
}

void UsrpController::setTxBufferSize(size_t bufferSize) {
    if(sendBuffer_ && sendBuffer_->capacity() == bufferSize) {
        return;
    }
    sendBuffer_.reset(new CirularBufferThreadSafe<complex<float>>(bufferSize));
}

void UsrpController::send(std::complex<float> *buff, size_t n) {
    uhd::tx_metadata_t tx_md;
    tx_md.start_of_burst = true; //1
    tx_md.end_of_burst = false;
    tx_md.has_time_spec = false; //如果这里是true，那么后面就要指定时间，不然发射有问题，ue不能捕获

    int toSend = 0;
    int samples_per_buff = txStream_->get_max_num_samps();
    while(toSend != 0) {
        int nsend = txStream_->send(buff, samples_per_buff, tx_md);
        toSend -= nsend;
    }
    tx_md.end_of_burst = true;
}

void UsrpController::sendAsync(std::complex<float> *buff, size_t n) {
    static std::once_flag flag;
    std::call_once(flag, std::bind(&UsrpController::startSendThread, this));

    frameSize_ = n;
    latch_.countDown();
    sendBuffer_->put(buff, n);
}

void UsrpController::setRxBufferSize(size_t bufferSize) {
    if(receiveBuffer_ && receiveBuffer_->capacity() == bufferSize) {
        return;
    }
    receiveBuffer_.reset(new CirularBufferThreadSafe<complex<float>>(bufferSize));
}

// 循环接收，线程安全的调用
void UsrpController::receiveContinue() {
    uhd::stream_cmd_t stream_cmd(uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS);
    stream_cmd.stream_now = true;
    stream_cmd.time_spec = uhd::time_spec_t();
    //stream_cmd.time_spec = usrp_->get_time_now();


    rxStream_->issue_stream_cmd(stream_cmd);

    uhd::rx_metadata_t md;

    const int kSAMPLE_PER_BUFF = rxStream_->get_max_num_samps();
    cout << boost::format("sample_per_buff = %d\n") % kSAMPLE_PER_BUFF;
    size_t numRxSamps;
    const auto start = std::chrono::steady_clock::now();
    vector<complex<float>> compFloatVec(kSAMPLE_PER_BUFF);
    while(1) {
        numRxSamps = rxStream_->recv(compFloatVec.data(), kSAMPLE_PER_BUFF, md, 3.0, false);

        if (md.error_code == uhd::rx_metadata_t::ERROR_CODE_TIMEOUT) {
            printf("Timeout while streaming......\n");
            break;
        }
        if (md.error_code == uhd::rx_metadata_t::ERROR_CODE_OVERFLOW) {
            printf("Overflowing while stream......\n");
            continue;
        }
        if (md.error_code != uhd::rx_metadata_t::ERROR_CODE_NONE) {
            printf("Receive error: %s \n", md.strerror());
            continue;
        }

        receiveBuffer_->put(compFloatVec.data(), numRxSamps);
    }
}

void UsrpController::peek(std::vector<std::complex<float> > &toReceive) {
    receiveBuffer_->peek(toReceive.data(), toReceive.size());
}

void UsrpController::retrieve(int len) {
    receiveBuffer_->retrieve(len);
}

void UsrpController::read(std::vector<complex<float> > &toReceive) {
    receiveBuffer_->get(toReceive.data(), toReceive.size());
}

// 注意这里接收的基础单位是一个symbol
void UsrpController::receiveNSymbol(std::complex<float> *hostBuff, size_t nSymbol) {
    uhd::stream_cmd_t stream_cmd(uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS);
    stream_cmd.num_samps = nSymbol;
    stream_cmd.stream_now = true;
    stream_cmd.time_spec = uhd::time_spec_t();
    //stream_cmd.time_spec = usrp_->get_time_now();

    uhd::stream_args_t stream_args("fc32", "sc16");
    std::vector<size_t> channel_nums;
    channel_nums.push_back(boost::lexical_cast<size_t>(0));
    stream_args.channels = channel_nums;
    uhd::rx_streamer::sptr rx_stream = usrp_->get_rx_stream(stream_args);

    rx_stream->issue_stream_cmd(stream_cmd);

    uhd::rx_metadata_t md;

    size_t samplePerBuff = 0;
    size_t toWrite = 0;
    size_t nSampsLeft = nSymbol;
    const int kSAMPLE_PER_BUFF = rx_stream->get_max_num_samps();
    cout << boost::format("sample_per_buff = %d\n") % kSAMPLE_PER_BUFF;
    size_t numRxSamps;
    const auto start = std::chrono::steady_clock::now();
    while (nSampsLeft != 0) {
        if (nSampsLeft >= kSAMPLE_PER_BUFF) {
            samplePerBuff = kSAMPLE_PER_BUFF;
        } else {
            samplePerBuff = nSampsLeft;
        }
        // std::cout << nSampsLeft << std::endl;
        numRxSamps = rx_stream->recv(hostBuff + toWrite, samplePerBuff, md, 3.0, false);

        if (md.error_code == uhd::rx_metadata_t::ERROR_CODE_TIMEOUT) {
            printf("Timeout while streaming......\n");
            break;
        }
        if (md.error_code == uhd::rx_metadata_t::ERROR_CODE_OVERFLOW) {
            printf("Overflowing while stream......\n");
            continue;
        }
        if (md.error_code != uhd::rx_metadata_t::ERROR_CODE_NONE) {
            printf("Receive error: %s \n", md.strerror());
            continue;
        }

        //std::copy(buff.begin(), buff.end(), hostBuff + toWrite);
        toWrite += numRxSamps;
        nSampsLeft -= numRxSamps;
    }
    const auto end = std::chrono::steady_clock::now();
    const auto time_diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    cout << boost::format("recv cost %d milliseconds") % time_diff.count() << endl;

    stream_cmd.stream_mode = uhd::stream_cmd_t::STREAM_MODE_STOP_CONTINUOUS;
    stream_cmd.stream_now = false;
    uhd::time_spec_t stop(0.5);
    stream_cmd.time_spec = stop;
    rx_stream->issue_stream_cmd(stream_cmd);  //to end continuous
    this_thread::sleep_for(chrono::seconds(1));
}

void UsrpController::startSendThread() {
    sendThread_.reset(new thread(std::bind(&UsrpController::sendThreadRoutine, this)));
}

void UsrpController::sendThreadRoutine() {
    uhd::tx_metadata_t tx_md;
    tx_md.start_of_burst = true; //1
    tx_md.end_of_burst = false;
    tx_md.has_time_spec = false; //如果这里是true，那么后面就要指定时间，不然发射有问题，ue不能捕获

    int toSend = 0;
    latch_.wait();
    const int samples_per_buff = frameSize_;
    int leftSend;
    vector<complex<float>> compFloatVec(samples_per_buff);
    printf("tx_buff_size = %d\n", samples_per_buff);
    while(1) {
        sendBuffer_->get(compFloatVec.data(), compFloatVec.size());
        leftSend = compFloatVec.size();
        printf("sendData.size() = %d\n", compFloatVec.size());
        write2bin("sendData.bin", compFloatVec);
        // printf();
        toSend = 0;
        while(leftSend != 0) {
            int nsend = txStream_->send(compFloatVec.data() + toSend, leftSend, tx_md);
            toSend += nsend;
            leftSend -= nsend;
            printf("nsend = %d\n", nsend);
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    tx_md.end_of_burst = true;
}

void UsrpController::printInfo() {
    std::cout << boost::format("Using Device: %s") % usrp_->get_pp_string() << std::endl;
}

bool UsrpController::check_locked_sensor(
    std::vector<std::string> sensor_names,
    const char *sensor_name,
    get_sensor_fn_t get_sensor_fn,
    double setup_time
) {
    if (std::find(sensor_names.begin(), sensor_names.end(), sensor_name) == sensor_names.end()) {
        return false;
    }

    auto setup_timeout =
        std::chrono::steady_clock::now()
        + std::chrono::milliseconds(int64_t(setup_time * 1000));
    bool lock_detected = false;

    std::cout << boost::format("Waiting for \"%s\": ") % sensor_name;
    std::cout.flush();

    while (true) {
        if (lock_detected and
                (std::chrono::steady_clock::now() > setup_timeout)) {
            std::cout << " locked." << std::endl;
            break;
        }
        if (get_sensor_fn(sensor_name).to_bool()) {
            std::cout << "+";
            std::cout.flush();
            lock_detected = true;
        } else {
            if (std::chrono::steady_clock::now() > setup_timeout) {
                std::cout << std::endl;
                throw std::runtime_error(str(
                                             boost::format("timed out waiting for consecutive locks on sensor \"%s\"")
                                             % sensor_name
                                         ));
            }
            std::cout << "_";
            std::cout.flush();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cout << std::endl;
    return true;
}
