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

#include <stellarstation/api.h>
#include <gnuradio/sync_block.h>

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
    class STELLARSTATION_API api_source : virtual public gr::block
    {
     public:
      typedef boost::shared_ptr<api_source> sptr;

      static sptr make();
    };

  } // namespace stellarstation
} // namespace gr

#endif /* INCLUDED_STELLARSTATION_API_SOURCE_H */

