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



#ifndef _SCIFEPD_SCIFEPD_H_
#define _SCIFEPD_SCIFEPD_H_

#include <scif.h>
#include <system_error>

namespace t4s {

class ScifEpd {
 private:
  scif_epd_t epd_;
  void throwIfFailed() {
    if (epd_== SCIF_OPEN_FAILED)
      throw std::system_error(errno, std::system_category(), "On ScifEpd initialization.");
  }

 public:
  ScifEpd() : epd_{scif_open()} { throwIfFailed(); }

  explicit ScifEpd(scif_epd_t e) : epd_{e} { throwIfFailed(); }

  // Move constructor and assignment
  ScifEpd(ScifEpd &&e) : epd_{e.epd_} {
    throwIfFailed();
    e.epd_ = SCIF_OPEN_FAILED;
  }

  ScifEpd &operator=(ScifEpd &&e) {
    scif_close(epd_);
    epd_ = e.epd_;
    throwIfFailed();
    e.epd_ = SCIF_OPEN_FAILED;
    return *this;
  }

  // Copy is prohibited
  ScifEpd(const ScifEpd &e) = delete;
  ScifEpd &operator=(const ScifEpd &e) = delete;


  ~ScifEpd() {
    scif_close(epd_);
  }

  scif_epd_t get() const { return epd_; }

};
}
#endif
