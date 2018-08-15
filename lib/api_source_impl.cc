/* -*- c++ -*- */
/* 
 * Copyright 2018 Infostellar, Inc..
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "api_source_impl.h"

#include <iostream>
#include <memory>

#include <grpc/grpc.h>
#include <grpc/support/alloc.h>
#include <grpc/support/log.h>
#include <grpcpp/channel.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

using stellarstation::api::v1::StellarStationService;
using stellarstation::api::v1::SatelliteStreamRequest;
using stellarstation::api::v1::SatelliteStreamResponse;
using stellarstation::api::v1::ReceiveTelemetryResponse;

namespace gr {
  namespace stellarstation {

    api_source::sptr
    api_source::make(const char *key_path)
    {
      return gnuradio::get_initial_sptr
        (new api_source_impl(key_path));
    }

    /*
     * The private constructor
     */
    api_source_impl::api_source_impl(const char *key_path)
      : gr::block("api_source",
              gr::io_signature::make(0, 0, 0),
              gr::io_signature::make(0, 0, 0)),
        port_(pmt::mp("out")),
        thread_(NULL),
        key_path_(key_path)
    {
        message_port_register_out(port_);
    }

    bool api_source_impl::start() {
      // TODO: Make this a function
      std::ifstream json_key_file(key_path_);
      std::stringstream key_stream;
      key_stream << json_key_file.rdbuf();
      grpc::string json_key(key_stream.str());

      auto call_creds = grpc::ServiceAccountJWTAccessCredentials(json_key);
      // TODO: Don't hardcode tls file and make this a function
      std::ifstream root_cert_key_file("/home/rei/IdeaProjects/stellarstation-api/examples/fakeserver/src/main/resources/tls.crt");
      std::stringstream root_cert_key_stream;
      root_cert_key_stream << root_cert_key_file.rdbuf();
      grpc::string root_cert(root_cert_key_stream.str());

      grpc::SslCredentialsOptions opts;
      opts.pem_root_certs = root_cert;

      auto channel_creds = grpc::SslCredentials(opts);
      auto composite_creds = grpc::CompositeChannelCredentials(channel_creds, call_creds);
      // TODO: Don't hardcode the server to talk to
      auto channel = grpc::CreateChannel("localhost:8080", composite_creds);
      stub_  = StellarStationService::NewStub(channel);

      client_reader_writer_ = stub_->OpenSatelliteStream(&context_);

      SatelliteStreamRequest request;
      // TODO: Don't hardcode this
      request.set_satellite_id("5");
      client_reader_writer_->Write(request);

      thread_ = new std::thread(std::bind(&api_source_impl::readloop, this));
      return true;
    }

    bool api_source_impl::stop() {
      // TODO: End things here
      thread_->join();
      std::cout << "Stopped successfully" << std::endl;
      return true;
    }

    void api_source_impl::readloop() {
      std::cout << "Started loop " << key_path_ << std::endl;

      SatelliteStreamResponse response;
      while (client_reader_writer_->Read(&response)) {
        std::cout << "STREAM ID" << response.stream_id() << std::endl;
        if (response.has_receive_telemetry_response()) {
            const ReceiveTelemetryResponse &telem_resp = response.receive_telemetry_response();
            std::cout << "data: ";
            for (int i=0; i<100; i++) {
              std::cout << (int)telem_resp.telemetry().data()[i];
            }
            std::cout << std::endl;
            std::cout << "data length: " << telem_resp.telemetry().data().size() << std::endl;
            std::cout << "framing: " << telem_resp.telemetry().framing() << std::endl;
            std::cout << "dl frequency: " << telem_resp.telemetry().downlink_frequency_hz() << std::endl;
        }
      }
    }

    /*
     * Our virtual destructor.
     */
    api_source_impl::~api_source_impl()
    {
    }

  } /* namespace stellarstation */
} /* namespace gr */
