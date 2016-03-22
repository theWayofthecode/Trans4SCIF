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

#define PORT 6798

template<typename Node>
std::array<std::unique_ptr<Node>, 2> MakeConnectedNodes() {
  std::array<std::unique_ptr<Node>, 2> sn_pair;
  std::thread t_acc([&sn_pair]() {
    sn_pair[0].reset(new Node(PORT));
  });
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  try {
    sn_pair[1].reset(new Node(0, PORT));
  } catch (std::system_error e) {
    std::cerr << "Warning: MakeConnectedNodes: " << e.what() << __FILE__LINE__ << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    sn_pair[1].reset(new Node(0, PORT));
  }
  t_acc.join();
  return sn_pair;
}