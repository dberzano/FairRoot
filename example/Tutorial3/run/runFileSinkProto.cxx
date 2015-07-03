/********************************************************************************
 *    Copyright (C) 2014 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH    *
 *                                                                              *
 *              This software is distributed under the terms of the             * 
 *         GNU Lesser General Public Licence version 3 (LGPL) version 3,        *  
 *                  copied verbatim in the file "LICENSE"                       *
 ********************************************************************************/
/**
 * runFileSinkProto.cxx
 *
 * @since 2013-01-21
 * @author A. Rybalchenko
 */

#include <iostream>

#include "boost/program_options.hpp"

#include "FairMQLogger.h"
#include "FairMQFileSink.h"

#ifdef NANOMSG
#include "nanomsg/FairMQTransportFactoryNN.h"
#else
#include "zeromq/FairMQTransportFactoryZMQ.h"
#endif

#include "FairTestDetectorHit.h"
#include "FairTestDetectorPayload.pb.h"

using namespace std;

typedef FairTestDetectorHit THit;                    // class to serialize/deserialize
typedef TestDetectorProto::HitPayload TProtoPayload; // protobuf payload
typedef FairMQFileSink<THit, TProtoPayload> TSink;

typedef struct DeviceOptions
{
    DeviceOptions() :
        id(), ioThreads(0),
        inputSocketType(), inputBufSize(0), inputMethod(), inputAddress() {}

    string id;
    int ioThreads;
    string inputSocketType;
    int inputBufSize;
    string inputMethod;
    string inputAddress;
} DeviceOptions_t;

inline bool parse_cmd_line(int _argc, char* _argv[], DeviceOptions* _options)
{
    if (_options == NULL)
        throw std::runtime_error("Internal error: options' container is empty.");

    namespace bpo = boost::program_options;
    bpo::options_description desc("Options");
    desc.add_options()
        ("id", bpo::value<string>()->required(), "Device ID")
        ("io-threads", bpo::value<int>()->default_value(1), "Number of I/O threads")
        ("input-socket-type", bpo::value<string>()->required(), "Input socket type: sub/pull")
        ("input-buff-size", bpo::value<int>()->required(), "Input buffer size in number of messages (ZeroMQ)/bytes(nanomsg)")
        ("input-method", bpo::value<string>()->required(), "Input method: bind/connect")
        ("input-address", bpo::value<string>()->required(), "Input address, e.g.: \"tcp://*:5555\"")
        ("help", "Print help messages");

    bpo::variables_map vm;
    bpo::store(bpo::parse_command_line(_argc, _argv, desc), vm);

    if ( vm.count("help") )
    {
        LOG(INFO) << "FairMQ File Sink" << endl << desc;
        return false;
    }

    bpo::notify(vm);

    if ( vm.count("id") )
        _options->id = vm["id"].as<string>();

    if ( vm.count("io-threads") )
        _options->ioThreads = vm["io-threads"].as<int>();

    if ( vm.count("input-socket-type") )
        _options->inputSocketType = vm["input-socket-type"].as<string>();

    if ( vm.count("input-buff-size") )
        _options->inputBufSize = vm["input-buff-size"].as<int>();

    if ( vm.count("input-method") )
        _options->inputMethod = vm["input-method"].as<string>();

    if ( vm.count("input-address") )
        _options->inputAddress = vm["input-address"].as<string>();

    return true;
}

int main(int argc, char** argv)
{
    TSink filesink;
    filesink.CatchSignals();

    DeviceOptions_t options;
    try
    {
        if (!parse_cmd_line(argc, argv, &options))
            return 0;
    }
    catch (exception& e)
    {
        LOG(ERROR) << e.what();
        return 1;
    }

    LOG(INFO) << "PID: " << getpid();

#ifdef NANOMSG
    FairMQTransportFactory* transportFactory = new FairMQTransportFactoryNN();
#else
    FairMQTransportFactory* transportFactory = new FairMQTransportFactoryZMQ();
#endif

    filesink.SetTransport(transportFactory);

    FairMQChannel channel(options.inputSocketType, options.inputMethod, options.inputAddress);
    channel.UpdateSndBufSize(options.inputBufSize);
    channel.UpdateRcvBufSize(options.inputBufSize);
    channel.UpdateRateLogging(1);

    filesink.fChannels["data-in"].push_back(channel);

    filesink.SetProperty(TSink::Id, options.id);
    filesink.SetProperty(TSink::NumIoThreads, options.ioThreads);

    filesink.ChangeState(TSink::INIT_DEVICE);
    filesink.WaitForEndOfState(TSink::INIT_DEVICE);

    filesink.ChangeState(TSink::INIT_TASK);
    filesink.WaitForEndOfState(TSink::INIT_TASK);

    filesink.InitOutputFile(options.id);

    filesink.ChangeState(TSink::RUN);
    filesink.InteractiveStateLoop();


    return 0;
}