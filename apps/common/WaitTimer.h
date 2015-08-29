//
//  WaitTimer.h
//
//  Created by David Wicks on 8/29/15.
//
//

#pragma once
#include "asio/asio.hpp"

namespace sansumbrella
{

///
///
class WaitTimer : public std::enable_shared_from_this<WaitTimer>
{
public:
  static std::shared_ptr<WaitTimer> create(const std::function<void ()> &fn, double delay, asio::io_service &service);
  void cancel();

private:
  WaitTimer(const std::function<void ()> &fn, double delay);
  ci::Timer               _timer;
  double                  _delay;
  std::function<void ()>  _fn;

  void update(asio::io_service &service);
};

} // namespace sansumbrella
