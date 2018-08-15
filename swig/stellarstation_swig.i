/* -*- c++ -*- */

#define STELLARSTATION_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "stellarstation_swig_doc.i"

%{
#include "stellarstation/api_source.h"
%}


%include "stellarstation/api_source.h"
GR_SWIG_BLOCK_MAGIC2(stellarstation, api_source);
