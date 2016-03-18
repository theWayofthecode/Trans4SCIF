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

#include "rmawindow.hpp"

namespace t4s {

RMAWindow::RMAWindow(scif_epd_t epd, std::size_t len, int prot_flags) {
  this->epd_ = epd;
  this->len_ = ROUND_TO_BOUNDARY(len, PAGE_SIZE);
  int err = posix_memalign(&mem_, PAGE_SIZE, this->len_);
  if (err) {
    throw std::system_error(err, std::system_category(), __FILE__LINE__);
  }
  bzero(mem_, this->len_);
  off_ = scif_register(epd, mem_, this->len_, 0, prot_flags, 0);
  if (off_ == SCIF_REGISTER_FAILED) {
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);
  }
}

RMAWindow::RMAWindow(RMAWindow &&w)
    : epd_{w.epd_},
      mem_{w.mem_},
      off_{w.off_},
      len_{w.len_} {
  w.epd_ = -1;
  w.mem_ = nullptr;
  w.off_ = 0;
  w.len_ = 0;
}

RMAWindow &RMAWindow::operator=(RMAWindow &&w) {
  epd_ = w.epd_;
  mem_ = w.mem_;
  off_ = w.off_;
  len_ = w.len_;
  w.epd_ = -1;
  w.mem_ = nullptr;
  w.off_ = 0;
  w.len_ = 0;
  return *this;
}

RMAWindow::~RMAWindow() {
  if (mem_) {
    free(mem_);
  }
  if (epd_ != -1) {
    if (scif_unregister(epd_, off_, len_) == -1) {
      std::system_error e(errno, std::system_category(), __FILE__LINE__);
      std::cerr << "Warning: scif_unregister: " << e.what() << __FILE__LINE__ << std::endl;
    }
  }
}

}