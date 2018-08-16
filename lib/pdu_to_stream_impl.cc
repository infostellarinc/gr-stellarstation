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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "pdu_to_stream_impl.h"

#include <algorithm>

namespace gr {
  namespace stellarstation {

    pdu_to_stream::sptr
    pdu_to_stream::make(size_t itemsize)
    {
      return gnuradio::get_initial_sptr
        (new pdu_to_stream_impl(itemsize));
    }

    /*
     * The private constructor
     */
    pdu_to_stream_impl::pdu_to_stream_impl(size_t itemsize)
      : gr::sync_block("pdu_to_stream",
              gr::io_signature::make(0, 0, 0),
              gr::io_signature::make(1, 1, itemsize)),
        current_pdu_(pmt::pmt_t()),
        itemsize_(itemsize),
        current_buffer_position_(0)
    {
      message_port_register_in(pmt::mp("pdu"));
    }

    /*
     * Our virtual destructor.
     */
    pdu_to_stream_impl::~pdu_to_stream_impl()
    {
    }

    int
    pdu_to_stream_impl::work (int noutput_items,
                               gr_vector_const_void_star &input_items,
                               gr_vector_void_star &output_items)
    {
      if(current_pdu_.get() == NULL) {
        // We are not currently processing a PDU
        pmt::pmt_t msg(delete_head_blocking(pmt::mp("pdu"), 100));
        if (msg.get() == NULL) {
          return 0;
        }

        if (!pmt::is_pair(msg)) {
          throw std::runtime_error("received a malformed pdu message");
        }

        current_pdu_ = msg;
      }

      pmt::pmt_t pmt_vect = pmt::cdr(current_pdu_);
      int total_pdu_items = pmt::blob_length(pmt_vect)/itemsize_;
      int available_items = total_pdu_items - current_buffer_position_;
      int items_to_transfer = std::min(available_items, noutput_items);

      size_t io(0);
      const uint8_t* ptr = (const uint8_t *) pmt::uniform_vector_elements(pmt_vect, io);
      uint8_t *out = (uint8_t *) output_items[0];

      memcpy(out, ptr + current_buffer_position_*itemsize_, items_to_transfer*itemsize_);

      current_buffer_position_ += items_to_transfer;

      if (current_buffer_position_ == total_pdu_items) {
        // PDU has been fully streamed out. We can now process another PDU.
        current_buffer_position_ = 0;
        current_pdu_ = pmt::pmt_t();
      }

      // Tell runtime system how many output items we produced.
      return items_to_transfer;
    }

  } /* namespace stellarstation */
} /* namespace gr */
