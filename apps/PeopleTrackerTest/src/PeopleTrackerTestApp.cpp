#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/Rand.h"
#include "cinder/Log.h"

#include "entityx/Entity.h"
#include "soso/BehaviorSystem.h"
#include "soso/ExpiresSystem.h"
#include "soso/Expires.h"

#include "JsonReceiverUDP.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace soso;

class PeopleTrackerTestApp : public App
{
public:
  PeopleTrackerTestApp();
  void setup() override;
  void update() override;
  void draw() override;

  void keyDown(KeyEvent event) override;

  void createTestEntities();

private:
  entityx::EventManager  _events;
  entityx::EntityManager _entities;
  entityx::SystemManager _systems;
  ci::Timer              _frame_timer;
  sansumbrella::JsonReceiverUDP _json_receiver;
};

PeopleTrackerTestApp::PeopleTrackerTestApp()
: _entities(_events),
  _systems(_entities, _events),
  _json_receiver(io_service())
{}

void PeopleTrackerTestApp::setup()
{
  _systems.add<BehaviorSystem>(_entities);
  _systems.add<ExpiresSystem>();
  _systems.configure();

  createTestEntities();

  _json_receiver.connect("", 21234);
  /*
  auto server_address = asio::ip::address_v4::from_string("230.0.0.110");
  auto listener_address = asio::ip::address_v4::from_string(System::getIpAddress());
  _json_receiver.connect_multicast(listener_address, server_address, 21234);
  */
  _json_receiver.getSignalJsonReceived().connect([this] (const JsonTree &json) {
    CI_LOG_I("Got some JSON: " << json.serialize());
  });
}

void PeopleTrackerTestApp::keyDown(cinder::app::KeyEvent event)
{
  _json_receiver.connect("", 21234);
  /*
  auto server_address = asio::ip::address_v4::from_string("230.0.0.110");
  auto listener_address = asio::ip::address_v4::from_string(System::getIpAddress());
  _json_receiver.connect_multicast(listener_address, server_address, 21234);
  */
}

void PeopleTrackerTestApp::createTestEntities()
{
  for (auto i = 0; i < 72; i += 1)
  {
    auto e = _entities.create();
    e.assign<Expires>(randFloat(1.0f, 100.0f));
  }
}

void PeopleTrackerTestApp::update()
{
  auto dt = _frame_timer.getSeconds();
  if (dt < std::numeric_limits<double>::epsilon() || dt > 0.1)
  {
    dt = 1.0 / 60.0;
  }
  _frame_timer.start();

  _systems.update<BehaviorSystem>(dt);
  _systems.update<ExpiresSystem>(dt);
}

void PeopleTrackerTestApp::draw()
{
  gl::clear();

  auto position = vec2(10);
  for (auto e : _entities.entities_with_components<Expires>())
  {
    auto ec = e.component<Expires>();
    auto index = to_string(e.id().index());
    auto time_remaining = to_string(ec->time);
    gl::drawString(index + " expires in " + time_remaining + " seconds.", position);

    position += vec2(0, 20);
    if (position.y > getWindowHeight())
    {
      position.y = 10;
      position.x += 210;
    }
  }
}

CINDER_APP( PeopleTrackerTestApp, RendererGl )
