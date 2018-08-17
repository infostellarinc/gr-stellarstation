#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
# Copyright 2018 Infostellar, Inc.
# 
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
# 
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this software; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
# 

from gnuradio import gr
from gnuradio import blocks
import pmt
from stellarstation import stellarstation_swig

class bitstream_source(gr.hier_block2):
    """
    This hierarchical block contains a block for streaming bitstream data from a satellite
    using the Stellarstation API.
    """
    def __init__(self, satellite_id, stream_id, key_path, root_cert_path="", api_url="api.stellarstation.com"):
        """
        :param satellite_id: Satellite ID to connect to
        :param stream_id: Stream ID to connect to. Can be an empty string.
        :param key_path: Path to JSON API Key
        :param root_cert_path: Path to root certificate for development server.
        Leave blank when connecting to the real API
        :param api_url: API URL to connect to.
        """
        gr.hier_block2.__init__(self,
                                "iq_source",
                                gr.io_signature(0, 0, 0),  # Input signature
                                gr.io_signature(1, 1, gr.sizeof_char))  # Output signature

        # Define blocks and connect them
        api_source = stellarstation_swig.api_source(satellite_id, stream_id, key_path, root_cert_path, api_url)
        pdu_filter = blocks.pdu_filter(pmt.intern("FRAMING"), pmt.from_uint64(0))  # Parse only packets with Bitstream Framing
        pdu_to_stream = stellarstation_swig.pdu_to_stream(gr.sizeof_char)

        self.msg_connect((api_source, 'out'), (pdu_filter, 'pdus'))
        self.msg_connect((pdu_filter, 'pdus'), (pdu_to_stream, 'pdu'))
        self.connect((pdu_to_stream, 0), (self, 0))
