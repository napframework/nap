#pragma once

#include <QObject>

#define ENTITY_NAME "Entity"

#define RUN_Q_APPLICATION 	\
    int argc = 0; 			\
    char* argv[] = {"app"}; \
    QCoreApplication app(argc, const_cast<char**>(argv));

/**
 * Used to test if signals are received
 */
class SigCapture
{
public:
	template <typename S, typename F>
	SigCapture(S* sender, F f) {
		QObject::connect(sender, f, [this]() {
			mCount++;
		});
	}

	int count() { return mCount; }
private:
	int mCount = 0;
};

/**
 * Shorthand to grab a file from the test resources dir
 * @param filename Relative filename
 * @return The transformed/usable filename
 */
QString getResource(const QString& filename);