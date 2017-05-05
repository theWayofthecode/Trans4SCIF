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

#ifndef _MMAPMEM_H_
#define _MMAPMEM_H_

#include <cstddef>
#include <scif.h>
#include <sys/types.h>
#include "trans4scif_config.h"
#include "util.h"

namespace t4s {

class Mmapmem {
 private:
  void * addr_;
  std::size_t const len_;
  off_t const off_;

 public:
  explicit Mmapmem(scif_epd_t epd, off_t off, std::size_t len, int prot_flags);

  //   Copy is prohibited.
  Mmapmem(const Mmapmem &m) = delete;
  Mmapmem &operator=(const Mmapmem &m) = delete;

  Mmapmem(Mmapmem &&m);
  Mmapmem &operator=(Mmapmem &&m) = delete;

  ~Mmapmem();

  void *addr() const { return addr_; }

  off_t off() const { return off_; }

  std::size_t size() const { return len_; }
};

}

#endif