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

api_source::sptr api_source::make(std::string satellite_id,
                                  std::string stream_id, std::string key_path,
                                  std::string root_cert_path,
                                  std::string api_url) {
  return gnuradio::get_initial_sptr(new api_source_impl(
      satellite_id, std::string(stream_id), key_path, root_cert_path, api_url));
}

/*
 * The private constructor
 */
api_source_impl::api_source_impl(std::string satellite_id,
                                 std::string stream_id, std::string key_path,
                                 std::string root_cert_path,
                                 std::string api_url)
    : gr::block("api_source", gr::io_signature::make(0, 0, 0),
                gr::io_signature::make(0, 0, 0)),
      port_(pmt::mp("out")),
      thread_(NULL),
      satellite_id_(satellite_id),
      stream_id_(stream_id),
      key_path_(key_path),
      root_cert_path_(root_cert_path),
      api_url_(api_url) {
  message_port_register_out(port_);
}

bool api_source_impl::start() {
  grpc::string json_key(read_file_into_string(key_path_));
  auto call_creds = grpc::ServiceAccountJWTAccessCredentials(json_key);

  grpc::SslCredentialsOptions opts;
  if (!root_cert_path_.empty()) {
    grpc::string root_cert(read_file_into_string(root_cert_path_));
    opts.pem_root_certs = root_cert;
  }

  auto channel_creds = grpc::SslCredentials(opts);
  auto composite_creds =
      grpc::CompositeChannelCredentials(channel_creds, call_creds);
  auto channel = grpc::CreateChannel(api_url_, composite_creds);
  stub_ = StellarStationService::NewStub(channel);

  client_reader_writer_ = stub_->OpenSatelliteStream(&context_);

  SatelliteStreamRequest request;
  request.set_satellite_id(satellite_id_);
  request.set_stream_id(stream_id_);
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
  if (!status.ok()) {
    GR_LOG_ERROR(d_logger,
                 boost::format("rpc failed. error code:  %1%\n%2%\n%3%") %
                     status.error_code() % status.error_message() %
                     status.error_details());
  }

  std::cout << "Stopped successfully" << std::endl;
  return true;
}

void api_source_impl::readloop() {
  SatelliteStreamResponse response;
  while (client_reader_writer_->Read(&response)) {
    GR_LOG_DEBUG(
        d_logger,
        boost::format("Packet received. Stream ID: %1%") % response.stream_id())
    if (response.has_receive_telemetry_response()) {
      const ReceiveTelemetryResponse &telem_resp =
          response.receive_telemetry_response();

      pmt::pmt_t dict_pmt(pmt::make_dict());
      dict_pmt = pmt::dict_add(
          dict_pmt, pmt::intern("DOWNLINK_FREQUENCY_HZ"),
          pmt::from_uint64(telem_resp.telemetry().downlink_frequency_hz()));
      dict_pmt =
          pmt::dict_add(dict_pmt, pmt::intern("FRAMING"),
                        pmt::from_uint64(telem_resp.telemetry().framing()));
      dict_pmt = pmt::dict_add(
          dict_pmt, pmt::intern("FRAME_HEADER"),
          pmt::make_blob(&telem_resp.telemetry().frame_header()[0],
                         telem_resp.telemetry().frame_header().size()));

      pmt::pmt_t vec_pmt(pmt::make_blob(&telem_resp.telemetry().data()[0],
                                        telem_resp.telemetry().data().size()));
      pmt::pmt_t pdu(pmt::cons(dict_pmt, vec_pmt));

      message_port_pub(port_, pdu);
    }
  }
}

grpc::string api_source_impl::read_file_into_string(std::string filename) {
  std::ifstream file(filename);
  std::stringstream stream;
  stream << file.rdbuf();
  return stream.str();
}

/*
 * Our virtual destructor.
 */
api_source_impl::~api_source_impl() {}

} /* namespace stellarstation */
} /* namespace gr */
