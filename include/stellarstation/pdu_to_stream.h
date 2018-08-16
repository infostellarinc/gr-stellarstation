/* -*- c++ -*- */
/* 
 * Copyright 2018 Infostellar, Inc.
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


#ifndef INCLUDED_STELLARSTATION_PDU_TO_STREAM_H
#define INCLUDED_STELLARSTATION_PDU_TO_STREAM_H

#include <stellarstation/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
  namespace stellarstation {

    /*!
     * This block converts a PDU into a stream. It is similar to the
     * PDU to Tagged Stream block, however, it does not have a limit
     * on the size of the PDU. (Stellarstation API usually returns ~1MB
     * packets, while PDU to Tagged Stream is limited to ~32k bytes.) The
     * design is mostly based on
     * http://lists.ettus.com/pipermail/usrp-users_lists.ettus.com/2016-November/050626.html
     * \ingroup stellarstation
     *
     */
    class STELLARSTATION_API pdu_to_stream : virtual public gr::sync_block
    {
     public:
      typedef boost::shared_ptr<pdu_to_stream> sptr;
      static sptr make(size_t itemsize);
    };

  } // namespace stellarstation
} // namespace gr

#endif /* INCLUDED_STELLARSTATION_PDU_TO_STREAM_H */

