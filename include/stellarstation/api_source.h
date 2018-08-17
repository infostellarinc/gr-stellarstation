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

#ifndef INCLUDED_STELLARSTATION_API_SOURCE_H
#define INCLUDED_STELLARSTATION_API_SOURCE_H

#include <gnuradio/sync_block.h>
#include <stellarstation/api.h>

namespace gr {
namespace stellarstation {

/*!
 * The Stellarstation API Source block is responsible for connecting to the
 * Stellarstation API, receiving data, and propagating the received packets
 * as PMTs for downstream blocks to consume. For users who are only expecting
 * to receive one type of data from the satellite (IQ data, bitstream), you
 * should use the dedicated hierarchical blocks (based on this block) for your
 * purpose e.g. Stellarstation IQ Source, Stellarstation bitstream source.
 * \ingroup stellarstation
 *
 */
class STELLARSTATION_API api_source : virtual public gr::block {
 public:
  typedef boost::shared_ptr<api_source> sptr;

  /**
   * The Stellarstation API Source block is responsible for connecting to the
   * Stellarstation API, receiving data, and propagating the received packets
   * as PMTs for downstream blocks to consume. For users who are only expecting
   * to receive one type of data from the satellite (IQ data, bitstream), you
   * should use the dedicated hierarchical blocks (based on this block) for your
   * purpose e.g. Stellarstation IQ Source, Stellarstation bitstream source.
   *
   * @param satellite_id Satellite ID to connect to
   * @param stream_id Stream ID to connect to. Can be an empty string.
   * @param key_path Path to JSON API Key
   * @param root_cert_path Path to root certificate for development server.
   * Leave blank for connecting to the real API
   * @param api_url API URL to connect to.
   */
  static sptr make(const char *satellite_id, const char *stream_id,
                   const char *key_path, const char *root_cert_path,
                   const char *api_url);
};

}  // namespace stellarstation
}  // namespace gr

#endif /* INCLUDED_STELLARSTATION_API_SOURCE_H */
