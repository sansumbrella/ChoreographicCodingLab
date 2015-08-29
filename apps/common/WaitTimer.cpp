//
//  WaitTimer.cpp
//
//  Created by David Wicks on 8/29/15.
//
//

#include "WaitTimer.h"
#include "cinder/Timer.h"

using namespace sansumbrella;

WaitTimer::WaitTimer(const std::function<void ()> &fn, double delay)
: _fn( fn ),
  _delay( delay )
{

}

void WaitTimer::cancel()
{
  _timer.stop();
}

void WaitTimer::update(asio::io_service &service)
{

}

std::shared_ptr<WaitTimer> WaitTimer::create(const std::function<void ()> &fn, double delay, asio::io_service &service)
{
  auto timer = std::shared_ptr<WaitTimer>(new WaitTimer(fn, delay));
  timer->update(service);

  return timer;
}
