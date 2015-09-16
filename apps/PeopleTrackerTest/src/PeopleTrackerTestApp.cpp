#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/Rand.h"
#include "cinder/Log.h"

#include "entityx/Entity.h"
#include "soso/BehaviorSystem.h"
#include "soso/ExpiresSystem.h"
#include "soso/Expires.h"
#include "soso/Transform.h"

#include "JSonListenerUDP.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace soso;

struct Person
{
  Person() = default;
  Person(uint32_t id): _id(id) {}

  uint32_t _id = 0;
};

struct Position
{
  Position() = default;

  std::vector<ci::vec2> _positions;
};

entityx::Entity create_or_update_point(entityx::EntityManager &entities, uint32_t id, const ci::vec2 &pos)
{
  entityx::Entity entity;
  entityx::ComponentHandle<Person> person;
  for (auto e: entities.entities_with_components(person))
  {
    if (person->_id == id)
    {
      entity = e;
      break;
    }
  }

  if (! entity)
  {
    entity = entities.create();
    entity.assign<Person>( id );
    entity.assign<Position>();
    entity.assign<Expires>( 1.0f );
  }

  auto position = entity.component<Position>();
  position->_positions.push_back(pos);
  entity.component<Expires>()->time = 1.0f;

  if (position->_positions.size() > 120)
  {
    position->_positions.erase(position->_positions.begin());
  }

  return entity;
}

class PeopleTrackerTestApp : public App
{
public:
  PeopleTrackerTestApp();
  void setup() override;
  void update() override;
  void draw() override;

  void createTestEntities();

private:
  entityx::EventManager         _events;
  entityx::EntityManager        _entities;
  entityx::SystemManager        _systems;
  ci::Timer                     _frame_timer;
  sansumbrella::JSonListenerUDP _json_receiver;
};

PeopleTrackerTestApp::PeopleTrackerTestApp()
: _entities(_events),
  _systems(_entities, _events),
  _json_receiver(io_service(), 21234)
{}

void PeopleTrackerTestApp::setup()
{
  _systems.add<BehaviorSystem>(_entities);
  _systems.add<ExpiresSystem>();
  _systems.configure();

  _json_receiver.get_signal_json_received().connect([this] (const JsonTree &json) {
    if (json.hasChild("tracks"))
    {
      auto &tracks = json.getChild("tracks");
      for (auto &t: tracks)
      {
        auto id = t.getValueForKey<int>("id", 0);
        auto pos = vec2(t.getValueForKey<float>("x", 0.0f), t.getValueForKey<float>("y", 0.0f));
        create_or_update_point(_entities, id, pos);

        console() << id << ": " << pos << endl;
      }
    }
  });
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
  gl::ScopedModelMatrix mat;
  gl::translate(getWindowCenter());
  auto scaling = vec2(50.0f);

  entityx::ComponentHandle<Position> position;
  entityx::ComponentHandle<Person>   person;
  for (auto __unused e: _entities.entities_with_components(position, person))
  {
    gl::begin(GL_LINE_STRIP);
    for (auto &p: position->_positions)
    {
      gl::vertex(p * scaling);
    }
    gl::end();
    gl::drawStrokedCircle(vec2(position->_positions.back() * scaling), 2.0f);
    gl::drawString(to_string(person->_id), position->_positions.back() * scaling);
  }
}

CINDER_APP( PeopleTrackerTestApp, RendererGl )
