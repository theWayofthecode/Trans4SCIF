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
#include <cstdlib>
#include <system_error>
#include <scif.h>
#include <iostream>
#include <cstring>
#include <cassert>

#include "mmapmem.h"

namespace t4s {

Mmapmem::Mmapmem(scif_epd_t epd, off_t off, std::size_t len, int prot_flags) {
  if ((addr_ = scif_mmap(nullptr, len, prot_flags, 0, epd, off)) == SCIF_MMAP_FAILED)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);
  len_ = len;
  off_ = off;
}

Mmapmem::Mmapmem(Mmapmem &&m) :
      addr_(m.addr_),
      off_(m.off_),
      len_(m.len_) {
  m.addr_ = nullptr;
  m.off_ = 0;
  m.len_ = 0;
}

Mmapmem &Mmapmem::operator=(Mmapmem &&m) {
  this->~Mmapmem();
  addr_ = m.addr_;
  off_ = m.off_;
  len_ = m.len_;
  m.addr_ = nullptr;
  m.off_ = 0;
  m.len_ = 0;
  return *this;
}

Mmapmem::~Mmapmem() {
  //TODO:
//  if (addr_ != nullptr) {
//    if (scif_munmap(addr_, len_) == -1) {
//      std::system_error e(errno, std::system_category(), __FILE__LINE__);
//      std::cerr << "Warning: scif_munmap: " << e.what() << __FILE__LINE__ << std::endl;
//    }
//  }
}

}
