#include "testApp.h"

int main() {
  ofSetCurrentRenderer(ofGLProgrammableRenderer::TYPE);
  ofSetupOpenGL(1024, 768, OF_WINDOW);
	ofRunApp(new testApp());
}
