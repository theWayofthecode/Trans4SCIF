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

#include "util.h"

namespace t4s {

void scaled_sleep(int v, int s, int m) {
  if (v < s) {
    std::this_thread::sleep_for(std::chrono::microseconds(10));
  } else if (v < m) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  } else {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

}