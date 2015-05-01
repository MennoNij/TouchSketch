#include "myproject.h"

#include<string>

#include "MyApplicationManager.h"
#include <LDF/VisComponent.h>
#include <LDF/LargeDisplayEvent.h>
#include <LDFToolkit/ConstantProvider.h>
#include <LDFToolkit/FrameTimers/WindowsFrameTimer.h>

#include <QTimer>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QTabletEvent>
#include <QString>

//Input device pointers/cursors
#define PRI_INDICATOR 0
#define SEC_INDICATOR 1


MyProject::MyProject(QWidget *parent) : QGLWidget(parent), width(800), height(600), fullScreen(false)
{
	setAttribute(Qt::WA_NoSystemBackground);

	// This timer provides constant update of the framebuffer so that after an
	// interaction, for example, there is no need to call updateGL() (what can
	// slow down the application)
	QTimer* timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), SLOT(update()));
	timer->start();

	setMouseTracking(true);

	// Initializing the event types
	ConstantProvider* constants = ConstantProvider::getInstance();
	pressType = constants->getEventTypeIdentifier("PRESS");
	dragType = constants->getEventTypeIdentifier("DRAG");
	releaseType = constants->getEventTypeIdentifier("RELEASE");
}

MyProject::~MyProject()
{
	delete manager;
}


void MyProject::initializeGL()
{
	// Background color
	glClearColor(1.0, 1.0, 1.0, 1.0);
	// OpenGL setup and optimization
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_DEPTH_TEST);        // we don't need depth buffering
	glDisable(GL_CULL_FACE);         // we don't need backface culling right now
	glDisable(GL_LIGHTING);          // we don't need lighting right now
	glDisable(GL_POLYGON_SMOOTH);    // we don't need anti-aliasing right now
	glDisable(GL_NORMALIZE);         // we don't use lights so we don't need normals
	glDisable(GL_DITHER);            // we don't need dithering

	// Initializing SBSDK2
	sbsdk2 = new CSBSDK2();
	HDC dc = GetDC(this->winId());
	HWND hwnd = WindowFromDC(dc);
	sbsdkWnd = new CSBSDKWnd(hwnd);	
	sbsdk2->SBSDKAttach(*sbsdkWnd);
	sbsdk2->SetEventHandler(this);
	sbsdk2->SBSDKSendMouseEvents(*sbsdkWnd, SB_MEF_NEVER, -1);

	// The manager HAS to be created in here to have a GL context available!
	// If not, initializations that rely on OpenGL (e.g., display lists) are
	// not done properly.
	manager = new MyApplicationManager(new WindowsFrameTimer(10));
}

void MyProject::resizeGL(int w, int h)
{
	width = w;
	height = h;
	glViewport(0, 0, (GLsizei) w, (GLsizei) h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, width, 0.0, height, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	manager->resize(width, height);
}

void MyProject::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT);
	manager->renderAll(false);
}

void MyProject::keyPressEvent(QKeyEvent *e)
{
	switch (e->key()) {
		case Qt::Key_Escape:
			manager->writeCommandLog();
			// Closes the application
			exit(0);
			break;
		case Qt::Key_F1:
			// Toggles full-screen (multi-monitor)
			fullScreen = !fullScreen;
			if (fullScreen) setupMultiMonitorFullScreen();
			else restoreFromMultiMonitorFullScreen();
			break;
		default:
			QString val = e->text();
			std::string s = val.toStdString();
			int mod = e->modifiers();
			manager->processKeyPress(s, mod);
			break;
	}
}

//MOUSE INPUT EVENTS

void MyProject::mousePressEvent(QMouseEvent *event)
{
	int button = 0;
	if(event->button() != Qt::LeftButton)
		button = 1;
	//std::cout << "\nEVENT BUTTON:"<<event->button()<<"\n";
	VisComponent* component = manager->getComponentAt(event->x(), height - event->y());
	if (component != NULL)
		component->moveNodeToFront();
	LargeDisplayEvent* evt = new LargeDisplayEvent(pressType, PRI_INDICATOR, event->x(), height - event->y());
	manager->processEvent(evt, button, 1.0f, component);
	delete evt;
}

void MyProject::mouseMoveEvent(QMouseEvent *event)
{
	//std::cout<<"move: "<<event->x()<<" "<<event->y()<<"\n";
	LargeDisplayEvent* evt = new LargeDisplayEvent(dragType, PRI_INDICATOR, event->x(), height - event->y());
	manager->processEvent(evt);
	delete evt;
}

void MyProject::mouseReleaseEvent(QMouseEvent *event)
{
	int button = 0;
	if(event->button() != Qt::LeftButton)
		button = 1;
	//std::cout << "\nEVENT BUTTON:"<<event->button()<<"\n";
	VisComponent* component = manager->getComponentAt(event->x(), height - event->y());
	if (component != NULL)
		manager->addToUpdateList(component->getId());
	LargeDisplayEvent* evt = new LargeDisplayEvent(releaseType, PRI_INDICATOR, event->x(), height - event->y());
	manager->processEvent(evt, button);
	delete evt;
}

//TABLET INPUT EVENTS

void MyProject::tabletEvent(QTabletEvent* e)
{
	//Distinguish dragging from clicking
	if (e->type() == QEvent::TabletPress)
	{
		//std::cout<<"PEN ID: "<<e->uniqueId()<<"\n";
		//equal to a button press
		int button = 0;
		if (e->pointerType() != QTabletEvent::Pen) //QTabletEvent::Eraser
			button = 1;

		VisComponent* component = manager->getComponentAt(e->x(), height - e->y());
		if (component != NULL)
			component->moveNodeToFront();
		LargeDisplayEvent* evt = new LargeDisplayEvent(pressType, SEC_INDICATOR, e->x(), height - e->y());
		manager->processEvent(evt, button, e->pressure(), component);
		delete evt;
	}
	else if (e->type() == QEvent::TabletMove)
	{
			//Equal to dragging
			LargeDisplayEvent* evt = new LargeDisplayEvent(dragType, SEC_INDICATOR, e->x(), height - e->y());
			manager->processEvent(evt, -1, e->pressure());
			delete evt;
	}
	else if (e->type() == QEvent::TabletRelease)
	{
		//equal to a button release
		int button = 0;
		if (e->pointerType() != QTabletEvent::Pen)
			button = 1;
		
		VisComponent* component = manager->getComponentAt(e->x(), height - e->y());
		if (component != NULL)
			manager->addToUpdateList(component->getId());
		LargeDisplayEvent* evt = new LargeDisplayEvent(releaseType, SEC_INDICATOR, e->x(), height - e->y());
		manager->processEvent(evt, button);
		delete evt;	
	}

}


bool MyProject::winEvent(MSG* m, long* result)
{
	if (m) {
		int imsg = m->message;
		if (imsg == CSBSDK2::SBSDK_NEW_MESSAGE) {
			sbsdk2->SBSDKProcessData();
			return false;
		}
	}
	return false;
}

//SMARTBOARD INPUT EVENTS

void MyProject::OnXYDown(int x, int y, int z, int iPointerID)
{
	unsigned int userID = translateSmartIPointerId(iPointerID);
	VisComponent* component = manager->getComponentAt(x, height - y - 1);
	if (component == NULL) return;
	component->moveNodeToFront();
	LargeDisplayEvent* evt = new LargeDisplayEvent(
		pressType, userID, x, height - y - 1);
	manager->processEvent(evt, 0, 1.0f, component);
	delete evt;
}

void MyProject::OnXYMove(int x, int y, int z, int iPointerID)
{
	unsigned int userID = translateSmartIPointerId(iPointerID);
	LargeDisplayEvent* evt = new LargeDisplayEvent(dragType, userID, x, height - y - 1);
	manager->processEvent(evt);
	delete evt;
}

void MyProject::OnXYUp(int x, int y, int z, int iPointerID)
{
	VisComponent* component = manager->getComponentAt(x, height - y - 1);
	if (component == NULL) return;
	manager->addToUpdateList(component->getId());
	LargeDisplayEvent* evt = new LargeDisplayEvent(
		releaseType, translateSmartIPointerId(iPointerID), x, height - y - 1);
	manager->processEvent(evt, 0);
	delete evt;
}


unsigned int MyProject::translateSmartIPointerId(int iPointerId)
{
	return (iPointerId & 256) >> 8;
}


void MyProject::setupMultiMonitorFullScreen()
{
	// Obtaining a handler for the window of the current context
	// (in English: getting a reference to the current OpenGL screen)
	HDC dc = GetDC(this->winId());
	HWND hwnd = WindowFromDC(dc);

	// Getting rid of the window title bar and, consequently, the system tray
	long windowStyle = GetWindowLong(hwnd, GWL_STYLE);
	SetWindowLong(hwnd, GWL_STYLE, windowStyle - (windowStyle & (WS_CAPTION)));

	// Getting the size of the virtual screen (i.e., the dimensions of the
	// screen made of the several monitors)
	int xPosVirtual = GetSystemMetrics(SM_XVIRTUALSCREEN)
		- GetSystemMetrics(SM_CXFRAME) + 1;
	int yPosVirtual = GetSystemMetrics(SM_YVIRTUALSCREEN)
		- GetSystemMetrics(SM_CYFRAME) + 1;
	int widthVirtual = GetSystemMetrics(SM_CXVIRTUALSCREEN)
		+ GetSystemMetrics(SM_CXFRAME) + GetSystemMetrics(SM_CXFRAME) - 2;
	int heightVirtual = GetSystemMetrics(SM_CYVIRTUALSCREEN)
		+ GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CYFRAME) - 2;
	// Placing the window with the bigger dimensions to cover the whole
	// multi-monitor screen
	SetWindowPos(hwnd, NULL, xPosVirtual, yPosVirtual, widthVirtual,
		heightVirtual, SWP_FRAMECHANGED);
}

void MyProject::restoreFromMultiMonitorFullScreen()
{
	// Obtaining a handler for the window of the current context
	// (in English: getting a reference to the current OpenGL screen)
	HDC dc = GetDC(this->winId());
	HWND hwnd = WindowFromDC(dc);
	// Recovering the window title
	long windowstyle = GetWindowLong(hwnd, GWL_STYLE);
	SetWindowLong(hwnd, GWL_STYLE, windowstyle | WS_CAPTION);
	// Placing the window in the upper left corner with default dimensions
	SetWindowPos(hwnd, NULL, 0, 0, 800, 600, SWP_FRAMECHANGED);
}
