// test.cpp

#include <QtGlobal>
#include <QCoreApplication>
#include <QtCore>
#include <QVector>
#include <QVariant>
#include <QDebug> 
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDataStream>
#include <QByteArray>
#include <QString>

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
using namespace std;

#include "xlsxdocument.h"

#ifdef Q_OS_WIN
// #include "colorprintf.h" // https://github.com/VittGam/colorprintf
#endif

int test()
{
    qDebug() << "[debug] current path : " << QDir::currentPath();

    QFile fileNames(":/xlsx_files/dir2.txt");
    if ( !fileNames.open(QIODevice::ReadOnly | QIODevice::Text) )
    {
        qDebug() << "[debug] failed to load dir2.txt";
        return (-1);
    }

    while (!fileNames.atEnd()) {
        QString line = fileNames.readLine().simplified();

        QString currentFilename = QString(":/xlsx_files/%1").arg(line);

        {
            using namespace QXlsx;
            Document output2(currentFilename);
            if (output2.isLoaded()) {
                if (output2.saveAs(line))
                    qDebug() << "[debug] xlsx is saved. " << line;
                else
                    qCritical() << "[debug] failed to save. " << line;
            }
            else
                qCritical() << "[debug] failed to load. " << line;
        }
    }

    return 0;
}

