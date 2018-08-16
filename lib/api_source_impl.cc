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
    api_source::make(const char *key_path, const char *root_cert_path)
    {
      return gnuradio::get_initial_sptr
        (new api_source_impl(key_path, root_cert_path));
    }

    /*
     * The private constructor
     */
    api_source_impl::api_source_impl(const char *key_path, const char *root_cert_path)
      : gr::block("api_source",
              gr::io_signature::make(0, 0, 0),
              gr::io_signature::make(0, 0, 0)),
        port_(pmt::mp("out")),
        thread_(NULL),
        key_path_(key_path),
        root_cert_path_(root_cert_path)
    {
        message_port_register_out(port_);
    }

    bool api_source_impl::start() {
      grpc::string json_key(read_file_into_string(key_path_));

      auto call_creds = grpc::ServiceAccountJWTAccessCredentials(json_key);
      grpc::string root_cert(read_file_into_string(root_cert_path_));

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
      std::cout << "Trying to stop" << std::endl;

      // Signal the server it's time to stop
      client_reader_writer_->WritesDone();

      // Wait for the server to end the stream.
      thread_->join();

      grpc::Status status = client_reader_writer_->Finish();
      if (!status.ok()) std::cout << "rpc failed." << std::endl;

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

            pmt::pmt_t dict_pmt(pmt::make_dict());
            dict_pmt = pmt::dict_add(dict_pmt,
                                     pmt::intern("Downlink Frequency Hz"),
                                     pmt::from_uint64(telem_resp.telemetry().downlink_frequency_hz()));
            dict_pmt = pmt::dict_add(dict_pmt,
                                     pmt::intern("Framing"),
                                     pmt::from_uint64(telem_resp.telemetry().framing()));
            dict_pmt = pmt::dict_add(dict_pmt,
                                     pmt::intern("Frame header"),
                                     pmt::make_blob(&telem_resp.telemetry().frame_header()[0],
                                                    telem_resp.telemetry().frame_header().size()));

            pmt::pmt_t vec_pmt(pmt::make_blob(&telem_resp.telemetry().data()[0],
                               telem_resp.telemetry().data().size()));
            pmt::pmt_t pdu(pmt::cons(dict_pmt, vec_pmt));

            message_port_pub(port_, pdu);
        }
      }
    }

    grpc::string api_source_impl::read_file_into_string(const char *filename) {
      std::ifstream file(filename);
      std::stringstream stream;
      stream << file.rdbuf();
      return stream.str();
    }

    /*
     * Our virtual destructor.
     */
    api_source_impl::~api_source_impl()
    {
    }

  } /* namespace stellarstation */
} /* namespace gr */
