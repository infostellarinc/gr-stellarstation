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

#ifndef INCLUDED_STELLARSTATION_PDU_TO_STREAM_IMPL_H
#define INCLUDED_STELLARSTATION_PDU_TO_STREAM_IMPL_H

#include <stellarstation/pdu_to_stream.h>

namespace gr {
namespace stellarstation {

class pdu_to_stream_impl : public pdu_to_stream {
 private:
  pmt::pmt_t current_pdu_;
  size_t itemsize_;
  int current_buffer_position_;

 public:
  pdu_to_stream_impl(size_t itemsize);
  ~pdu_to_stream_impl();

  int work(int noutput_items, gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);
};

}  // namespace stellarstation
}  // namespace gr

#endif /* INCLUDED_STELLARSTATION_PDU_TO_STREAM_IMPL_H */
