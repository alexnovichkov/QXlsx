########################################
# QXlsx.pri
########################################

QT += core
QT += gui-private

# TODO: Define your C++ version. c++14, c++17, etc.
CONFIG += c++17

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# For DLL visibility
#DEFINES += QXlsx_SHAREDLIB QXlsx_EXPORTS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

isEmpty(QXLSX_PARENTPATH) {
    message( 'QXLSX_PARENTPATH is empty. use default value.' )
    QXLSX_PARENTPATH = $$PWD/../
} else {
    message( 'QXLSX_PARENTPATH is not empty.' )
}
message( $${QXLSX_PARENTPATH} )

isEmpty(QXLSX_HEADERPATH) {
    message( 'QXLSX_HEADERPATH is empty. use default value.' )
    QXLSX_HEADERPATH = $${QXLSX_PARENTPATH}header/
} else {
    message( 'QXLSX_HEADERPATH is not empty.' )
}
message( $${QXLSX_HEADERPATH} )

isEmpty(QXLSX_SOURCEPATH) {
    message( 'QXLSX_SOURCEPATH is empty. use default value.' )
    QXLSX_SOURCEPATH = $${QXLSX_PARENTPATH}source/
} else {
    message( 'QXLSX_SOURCEPATH is not empty.' )
}
message( $${QXLSX_SOURCEPATH} )

INCLUDEPATH += $$PWD
INCLUDEPATH += $${QXLSX_PARENTPATH}
INCLUDEPATH += $${QXLSX_HEADERPATH}


########################################
# source code 

HEADERS += \
$${QXLSX_HEADERPATH}xlsxheaderfooter.h \
$${QXLSX_HEADERPATH}xlsxpagesetup.h \
$${QXLSX_HEADERPATH}xlsxpagemargins.h \
$${QXLSX_HEADERPATH}xlsxsheetprotection.h \
$${QXLSX_HEADERPATH}xlsxautofilter.h \
$${QXLSX_HEADERPATH}xlsxsheetview.h \
$${QXLSX_HEADERPATH}xlsxlegend.h \
$${QXLSX_HEADERPATH}xlsxlayout.h \
$${QXLSX_HEADERPATH}xlsxseries.h \
$${QXLSX_HEADERPATH}xlsxeffect.h \
$${QXLSX_HEADERPATH}xlsxlabel.h \
$${QXLSX_HEADERPATH}xlsxfont.h \
$${QXLSX_HEADERPATH}xlsxtext.h \
$${QXLSX_HEADERPATH}xlsxtitle.h \
$${QXLSX_HEADERPATH}xlsxfillformat.h \
$${QXLSX_HEADERPATH}xlsxmain.h \
$${QXLSX_HEADERPATH}xlsxshapeformat.h \
$${QXLSX_HEADERPATH}xlsxaxis.h \
$${QXLSX_HEADERPATH}xlsxmarkerformat.h \
$${QXLSX_HEADERPATH}xlsxlineformat.h \
$${QXLSX_HEADERPATH}xlsxabstractooxmlfile.h \
$${QXLSX_HEADERPATH}xlsxabstractooxmlfile_p.h \
$${QXLSX_HEADERPATH}xlsxabstractsheet.h \
$${QXLSX_HEADERPATH}xlsxabstractsheet_p.h \
$${QXLSX_HEADERPATH}xlsxcell.h \
$${QXLSX_HEADERPATH}xlsxcellformula.h \
$${QXLSX_HEADERPATH}xlsxcellrange.h \
$${QXLSX_HEADERPATH}xlsxcellreference.h \
$${QXLSX_HEADERPATH}xlsxchart.h \
$${QXLSX_HEADERPATH}xlsxchartsheet.h \
$${QXLSX_HEADERPATH}xlsxchartsheet_p.h \
$${QXLSX_HEADERPATH}xlsxcolor.h \
$${QXLSX_HEADERPATH}xlsxconditionalformatting.h \
$${QXLSX_HEADERPATH}xlsxconditionalformatting_p.h \
$${QXLSX_HEADERPATH}xlsxcontenttypes_p.h \
$${QXLSX_HEADERPATH}xlsxdatavalidation.h \
$${QXLSX_HEADERPATH}xlsxdatetype.h \
$${QXLSX_HEADERPATH}xlsxdocpropsapp_p.h \
$${QXLSX_HEADERPATH}xlsxdocpropscore_p.h \
$${QXLSX_HEADERPATH}xlsxdocument.h \
$${QXLSX_HEADERPATH}xlsxdrawinganchor_p.h \
$${QXLSX_HEADERPATH}xlsxdrawing_p.h \
$${QXLSX_HEADERPATH}xlsxformat.h \
$${QXLSX_HEADERPATH}xlsxformat_p.h \
$${QXLSX_HEADERPATH}xlsxglobal.h \
$${QXLSX_HEADERPATH}xlsxmediafile_p.h \
$${QXLSX_HEADERPATH}xlsxnumformatparser_p.h \
$${QXLSX_HEADERPATH}xlsxrelationships_p.h \
$${QXLSX_HEADERPATH}xlsxrichstring.h \
$${QXLSX_HEADERPATH}xlsxrichstring_p.h \
$${QXLSX_HEADERPATH}xlsxsharedstrings_p.h \
$${QXLSX_HEADERPATH}xlsxsimpleooxmlfile_p.h \
$${QXLSX_HEADERPATH}xlsxstyles_p.h \
$${QXLSX_HEADERPATH}xlsxtheme_p.h \
$${QXLSX_HEADERPATH}xlsxutility_p.h \
$${QXLSX_HEADERPATH}xlsxworkbook.h \
$${QXLSX_HEADERPATH}xlsxworkbook_p.h \
$${QXLSX_HEADERPATH}xlsxworksheet.h \
$${QXLSX_HEADERPATH}xlsxworksheet_p.h \
$${QXLSX_HEADERPATH}xlsxzipreader_p.h \
$${QXLSX_HEADERPATH}xlsxzipwriter_p.h

SOURCES += \
$${QXLSX_SOURCEPATH}xlsxheaderfooter.cpp \
$${QXLSX_SOURCEPATH}xlsxpagesetup.cpp \
$${QXLSX_SOURCEPATH}xlsxpagemargins.cpp \
$${QXLSX_SOURCEPATH}xlsxsheetprotection.cpp \
$${QXLSX_SOURCEPATH}xlsxautofilter.cpp \
$${QXLSX_SOURCEPATH}xlsxsheetview.cpp \
$${QXLSX_SOURCEPATH}xlsxlegend.cpp \
$${QXLSX_SOURCEPATH}xlsxlayout.cpp \
$${QXLSX_SOURCEPATH}xlsxseries.cpp \
$${QXLSX_SOURCEPATH}xlsxeffect.cpp \
$${QXLSX_SOURCEPATH}xlsxlabel.cpp \
$${QXLSX_SOURCEPATH}xlsxfont.cpp \
$${QXLSX_SOURCEPATH}xlsxtext.cpp \
$${QXLSX_SOURCEPATH}xlsxtitle.cpp \
$${QXLSX_SOURCEPATH}xlsxfillformat.cpp \
$${QXLSX_SOURCEPATH}xlsxmain.cpp \
$${QXLSX_SOURCEPATH}xlsxshapeformat.cpp \
$${QXLSX_SOURCEPATH}xlsxaxis.cpp \
$${QXLSX_SOURCEPATH}xlsxmarkerformat.cpp \
$${QXLSX_SOURCEPATH}xlsxlineformat.cpp \
$${QXLSX_SOURCEPATH}xlsxabstractooxmlfile.cpp \
$${QXLSX_SOURCEPATH}xlsxabstractsheet.cpp \
$${QXLSX_SOURCEPATH}xlsxcell.cpp \
$${QXLSX_SOURCEPATH}xlsxcellformula.cpp \
$${QXLSX_SOURCEPATH}xlsxcellrange.cpp \
$${QXLSX_SOURCEPATH}xlsxcellreference.cpp \
$${QXLSX_SOURCEPATH}xlsxchart.cpp \
$${QXLSX_SOURCEPATH}xlsxchartsheet.cpp \
$${QXLSX_SOURCEPATH}xlsxcolor.cpp \
$${QXLSX_SOURCEPATH}xlsxconditionalformatting.cpp \
$${QXLSX_SOURCEPATH}xlsxcontenttypes.cpp \
$${QXLSX_SOURCEPATH}xlsxdatavalidation.cpp \
$${QXLSX_SOURCEPATH}xlsxdatetype.cpp \
$${QXLSX_SOURCEPATH}xlsxdocpropsapp.cpp \
$${QXLSX_SOURCEPATH}xlsxdocpropscore.cpp \
$${QXLSX_SOURCEPATH}xlsxdocument.cpp \
$${QXLSX_SOURCEPATH}xlsxdrawing.cpp \
$${QXLSX_SOURCEPATH}xlsxdrawinganchor.cpp \
$${QXLSX_SOURCEPATH}xlsxformat.cpp \
$${QXLSX_SOURCEPATH}xlsxmediafile.cpp \
$${QXLSX_SOURCEPATH}xlsxnumformatparser.cpp \
$${QXLSX_SOURCEPATH}xlsxrelationships.cpp \
$${QXLSX_SOURCEPATH}xlsxrichstring.cpp \
$${QXLSX_SOURCEPATH}xlsxsharedstrings.cpp \
$${QXLSX_SOURCEPATH}xlsxsimpleooxmlfile.cpp \
$${QXLSX_SOURCEPATH}xlsxstyles.cpp \
$${QXLSX_SOURCEPATH}xlsxtheme.cpp \
$${QXLSX_SOURCEPATH}xlsxutility.cpp \
$${QXLSX_SOURCEPATH}xlsxworkbook.cpp \
$${QXLSX_SOURCEPATH}xlsxworksheet.cpp \
$${QXLSX_SOURCEPATH}xlsxzipreader.cpp \
$${QXLSX_SOURCEPATH}xlsxzipwriter.cpp


########################################
# custom setting for compiler & system

win32-g++ {
    message("compiling for windows g++. mingw or msys or cygwin.")
    INCLUDEPATH += $${QXLSX_HEADERPATH}win32-gcc 
	CONFIG(debug, debug|release) {
	} else {
	}	
}
win32-msvc2013 {
    message("Compiling for Visual Studio 2013")
    INCLUDEPATH += $${QXLSX_HEADERPATH}msvc2013 
	CONFIG(debug, debug|release) {
	} else {
	}		
}
win32-msvc2015 {
    message("Compiling for Visual Studio 2015")
    INCLUDEPATH += $${QXLSX_HEADERPATH}msvc2015 
	CONFIG(debug, debug|release) {
	} else {
	}		
}
win32-msvc2017 {
    message("Compiling for Visual Studio 2017")
    INCLUDEPATH += $${QXLSX_HEADERPATH}msvc2017 
	CONFIG(debug, debug|release) {
	} else {
	}		
}
win32-msvc2019 {
    message("Compiling for Visual Studio 2019")
    INCLUDEPATH += $${QXLSX_HEADERPATH}msvc2019
        CONFIG(debug, debug|release) {
        } else {
        }
}
unix {
   !contains(QT_ARCH, x86_64){
       LIB=lib32
       message("compiling for 32bit linux/unix system")
    } else {
       LIB=lib64
       message("compiling for 64bit linux/unix system")
   }

   INCLUDEPATH += $${QXLSX_HEADERPATH}unix-gcc

    # target.path = /usr/lib
    # INSTALLS += target
	
	CONFIG(debug, debug|release) {
	} else {
	}		
}
mac {
    message("compiling for mac os")
    INCLUDEPATH += $${QXLSX_HEADERPATH}mac
	CONFIG(debug, debug|release) {
	} else {
	}		
}

