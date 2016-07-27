/*
	Copyright (c) 2015-2016 CERN
	
	This software is distributed under the terms of the 
	GNU Lesser General Public Licence version 3 (LGPLv3),
	copied verbatim in the file "LICENSE".
	In applying this licence, CERN does not waive 
	the privileges and immunities granted to it by virtue of its status 
	as an Intergovernmental Organization or submit itself to any jurisdiction.
	
	Author: Aram Santogidis <aram.santogidis@cern.ch>
*/


#ifndef _CIRCBUF_CIRCBUF_H_
#define _CIRCBUF_CIRCBUF_H_

#include <cstddef>
#include <vector>
#include <memory>
#include "virt_circbuf.h"
#include "rmawindow.h"

namespace t4s {

class Circbuf: public VirtCircbuf {
 private:
  RMAWindow win_;
  uint8_t *base_mem_;

 public:
  Circbuf(RMAWindow w) : VirtCircbuf(w.get_off(), w.get_len()),
                         win_{std::move(w)},
                         base_mem_{static_cast<uint8_t *>(win_.get_mem())} { }

  // Copy and move is prohibited.
  Circbuf(const Circbuf &w) = delete;
  Circbuf &operator=(const Circbuf &w) = delete;
  Circbuf(Circbuf &&w) = delete;
  Circbuf &operator=(Circbuf &&w) = delete;
  
  std::size_t Write(const uint8_t *src, std::size_t len);

  std::size_t Read(uint8_t *dest, std::size_t len);

  void WrResetChunkHead();

  uint64_t RdReadResetChunkHead();

  uint64_t WrReadChunkHead();

};

}
#endif