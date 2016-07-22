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
#include <system_error>
#include "scifnode.h"
#include <future>

#define PORT 6798

template<typename Node, typename Listener, typename Connecter>
std::array<Node, 2> MakeConnectedNodes(Listener listen, Connecter connect) {
  std::array<Node, 2> sn_pair;
  std::promise<void> p;

  auto sn0_fut = std::async(std::launch::async, [&listen, &p]() -> Node {
    p.set_value();
    return Node(listen(PORT));
  });

// Wait for scif_accept()
  p.get_future().wait();
  // TODO: Whati is this sleep for?
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  uint16_t self_node_id = -1;
  scif_get_nodeIDs(nullptr, 0, &self_node_id);
  try {
    sn_pair[1] = Node(connect(self_node_id, PORT));
  } catch (std::system_error e) {
    std::cerr << "Warning: MakeConnectedNodes: " << e.what() << __FILE__LINE__ << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    sn_pair[1] = Node(connect(self_node_id, PORT));
  }
  sn_pair[0] = sn0_fut.get();
  return sn_pair;
}