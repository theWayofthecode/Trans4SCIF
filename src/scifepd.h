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
#include <iostream>
#include "constants.h"

namespace t4s {

class ScifEpd {
 private:
  scif_epd_t epd;
 public:
  ScifEpd() {
    if ((epd = scif_open()) == SCIF_OPEN_FAILED) {
      throw std::system_error(errno, std::system_category(), __FILE__LINE__);
    }
  }

  /* Copy and move is not supported */
  ScifEpd(const ScifEpd &e) = delete;

  ScifEpd &operator=(const ScifEpd &e) = delete;

  ScifEpd(ScifEpd &&e) = delete;

  ScifEpd &operator=(ScifEpd &&e) = delete;

  ~ScifEpd() {
    if (scif_close(epd) == -1) {
      std::system_error e(errno, std::system_category(), __FILE__LINE__);
      std::cerr << "Warning: scif_close: " << e.what() << std::endl;
    }
  }

//   TODO: Maybe implement move assignment and constructor

  void set_epd_t(scif_epd_t e) {
    if (scif_close(epd) == -1) {
      throw std::system_error(errno, std::system_category(), __FILE__LINE__);
    }
    epd = e;
  }

  scif_epd_t get_epd_t() const {
    return epd;
  }

};
}
#endif
