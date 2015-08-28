#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class DeltasIslandsApp : public App {
public:
  void setup() override;
  void mouseDown( MouseEvent event ) override;
  void update() override;
  void draw() override;
};

void DeltasIslandsApp::setup()
{
}

void DeltasIslandsApp::mouseDown( MouseEvent event )
{
}

void DeltasIslandsApp::update()
{
}

void DeltasIslandsApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
}

CINDER_APP( DeltasIslandsApp, RendererGl, [] (App::Settings *settings) {
  settings->setWindowSize(1280, 720);
} )
