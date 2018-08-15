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

#ifndef INCLUDED_STELLARSTATION_API_SOURCE_IMPL_H
#define INCLUDED_STELLARSTATION_API_SOURCE_IMPL_H

#include <stellarstation/api_source.h>

#include <thread>

#include "stellarstation.grpc.pb.h"

using stellarstation::api::v1::StellarStationService;
using stellarstation::api::v1::SatelliteStreamRequest;
using stellarstation::api::v1::SatelliteStreamResponse;

namespace gr {
  namespace stellarstation {

    class api_source_impl : public api_source
    {
     private:
      void readloop();

      std::thread *thread_;
      const pmt::pmt_t port_;
      const char *key_path_;
      grpc::ClientContext context_;
      std::unique_ptr<StellarStationService::Stub> stub_;
      std::shared_ptr<grpc::ClientReaderWriter<SatelliteStreamRequest, SatelliteStreamResponse> > client_reader_writer_;

     public:
      api_source_impl(const char *key_path);
      ~api_source_impl();

      bool start();
      bool stop();
    };

  } // namespace stellarstation
} // namespace gr

#endif /* INCLUDED_STELLARSTATION_API_SOURCE_IMPL_H */

