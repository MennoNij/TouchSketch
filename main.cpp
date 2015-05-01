#define NOMINMAX 

#include <QtGui/QApplication>
#include "MyProject.h"

int main(int argc, char *argv[])
{
	//add a bootstrap config file

	//multisampling
	QGLFormat fmt;
	fmt.setSampleBuffers(true);
	fmt.setSamples(16);
	QGLFormat::setDefaultFormat(fmt);

	QApplication a(argc, argv);
	MyProject w;
	w.show();
	a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));
	return a.exec();
}
