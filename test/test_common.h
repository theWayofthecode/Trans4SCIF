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

#include <memory>
#include <array>
#include <thread>
#include <chrono>
#include <stdexcept>
#include <system_error>
#include <iostream>
#include "scifnode.h"

constexpr uint16_t PORT{6798};

template<typename Node>
std::array<std::unique_ptr<Node>, 2> MakeConnectedNodes() {
  std::array<std::unique_ptr<Node>, 2> sn_pair;
  // Listen
  std::thread t([&sn_pair]() { sn_pair[0].reset(new Node(PORT)); });

  // Connect
  uint16_t self_node_id = -1;
  scif_get_nodeIDs(nullptr, 0, &self_node_id);
  for (int i = 0; i < 10; ++i) {
    try {
      sn_pair[1].reset(new Node(self_node_id, PORT));
      break;
    } catch (std::system_error e) {
      if (i == 10)
        throw e;
      std::cerr << "Warning: MakeConnectedNodes: " << e.what() << __FILE__LINE__ << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }
  t.join();
  return sn_pair;
}
