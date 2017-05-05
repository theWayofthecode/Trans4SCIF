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


#ifndef _RMAWINDOW_H_
#define _RMAWINDOW_H_

#include <cstddef>
#include <scif.h>
#include <sys/types.h>
#include "trans4scif_config.h"
#include "util.h"

namespace t4s {

class RMAWindow {
 private:
  scif_epd_t const epd_;
  void *mem_;
  off_t off_;
  std::size_t const len_;

 public:
  //  Open a window in the registered memory space of the process.
  //  len will be rounded up to PAGE_SIZE boundary if necessary.
  //  The prot_flags is formed by OR'ing SCIF_PROT_READ and SCIF_PROT_WRITE
  RMAWindow(scif_epd_t epd, std::size_t len, int prot_flags);

  //   Copy is prohibited.
  RMAWindow(const RMAWindow &w) = delete;
  RMAWindow &operator=(const RMAWindow &w) = delete;

  RMAWindow(RMAWindow &&w);
  RMAWindow &operator=(RMAWindow &&w) = delete;

  ~RMAWindow();

  void *mem() const { return mem_; }

  off_t off() const { return off_; }

  std::size_t size() const { return len_; }

  bool isInWindow(void *p) const { return (p >= mem_) && (p < mem_+len_); }
};

}

#endif