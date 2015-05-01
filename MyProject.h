#ifndef MYPROJECT_H
#define MYPROJECT_H

#include <qgl.h>
#include <Smart/SBSDK2.h>
#include <iostream>

class MyApplicationManager;
class Canvas;
class InputHandler;

class MyProject : public QGLWidget, public CSBSDK2EventHandler
{
	Q_OBJECT

	// Attributes
protected:
	unsigned int width;
	unsigned int height;
	bool fullScreen;

	// SBSDK (SmartBoard SDK)
	CSBSDK2* sbsdk2;
	CSBSDKWnd* sbsdkWnd;

	MyApplicationManager* manager;
	Canvas* canvas;

	// EVENT TYPES
	unsigned int pressType;
	unsigned int dragType;
	unsigned int releaseType;


	// Methods
public:
	MyProject(QWidget *parent = 0);
	~MyProject();


protected:
	// Qt OpenGL methods
	void initializeGL();
	void resizeGL(int w, int h);
	void paintGL();
	
	// Qt mouse and keyboard event handlers
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void keyPressEvent(QKeyEvent *e);
	void tabletEvent(QTabletEvent* e); 

	// SBSDK event handlers
	bool winEvent(MSG* m, long* result);
	void OnXYDown(int x, int y, int z, int iPointerID);
	void OnXYMove(int x, int y, int z, int iPointerID);
	void OnXYUp(int x, int y, int z, int iPointerID);


	unsigned int translateSmartIPointerId(int iPointerId);
	void setupMultiMonitorFullScreen();
	void restoreFromMultiMonitorFullScreen();
};

#endif // MYPROJECT_H
