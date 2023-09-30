# QXlsx

[![Conan Center](https://shields.io/conan/v/qxlsx)](https://conan.io/center/qxlsx)

> *Read this in other languages: [English](README.md), :kr: [Korean](README.ko.md), :ru: [Russian](README.RU.md)*

<p align="center"><img src="https://raw.githubusercontent.com/QtExcel/QXlsx/master/markdown.data/QXlsx-Desktop.png"></p>

- QXlsx is an excel files (`*.xlsx`) reader/writer library.
  - Because QtXlsxWriter is no longer supported, I created a new project that is based on QtXlsx. (2017-)
- Development language of QXlsx is C++ (with Qt.)

## About this fork

This fork was intended to add some features not yet implemented in the original QXlsx.

Right now there is an ongoing work to fully implement charts, including full documentation.

This fork requires C++17 as it uses `std::optional`.

### The fork concept

THe ECMA-376 specifies that many parts of xlsx charts can be optional, i.e. if not set, they shouldn't appear in the xlsx file.
The original QXlsx uses enums and maps to implement it: the feature is not set, if the map  has no corresponding key.

This fork utilises another approach. 

For simple properties (bool, int, enums) it uses `std::optional`: methods that return a chart feature return `std::optional`. You can test the feature with `std::optional::has_value()`.

For complex properties (f.e. the shape fill or the title text) classes that implement these properties are shallow-copyable and have `isValid()` method.
If the feature is set, `isValid()` will return true.

I try to get rid of the raw pointers. And definitely non-scoped enums shall be replaced with scoped ones. Some parts of the library still need serious overhaul. 

See [what is done up to this moment](ToDo.md).

You can find examples of charts here:

- [Simple charts](examples/Charts/chart.cpp)
- [Bar charts](examples/Charts/barchart.cpp)
- [Chart gridlines, titles and legends](examples/Charts/chartextended.cpp)
- [Combined charts, multiple axes](examples/CombinedChart/main.cpp)

## How to use

- See [examples](examples/README.md)
- See [wiki](https://github.com/QtExcel/QXlsx/wiki) - AlexNovichkov: largely outdated! Needs rewriting
- See [FAQ](https://github.com/QtExcel/QXlsx/wiki/FAQ) - AlexNovichkov: largely outdated! Needs rewriting

## How to set up (Installation)

- See [how to setup QXlsx project (qmake)](HowToSetProject.md)	
- See [how to setup QXlsx project (cmake)](HowToSetProject-cmake.md)	

## Github Actions

[![Android](https://github.com/QtExcel/QXlsx/actions/workflows/android.yml/badge.svg)](https://github.com/QtExcel/QXlsx/actions/workflows/android.yml) [![IOS](https://github.com/QtExcel/QXlsx/actions/workflows/ios.yml/badge.svg)](https://github.com/QtExcel/QXlsx/actions/workflows/ios.yml) [![MacOS](https://github.com/QtExcel/QXlsx/actions/workflows/macos.yml/badge.svg)](https://github.com/QtExcel/QXlsx/actions/workflows/macos.yml) [![Ubuntu](https://github.com/QtExcel/QXlsx/actions/workflows/ubuntu.yml/badge.svg)](https://github.com/QtExcel/QXlsx/actions/workflows/ubuntu.yml) [![Windows](https://github.com/QtExcel/QXlsx/actions/workflows/windows.yml/badge.svg)](https://github.com/QtExcel/QXlsx/actions/workflows/windows.yml) [![CMake](https://github.com/QtExcel/QXlsx/actions/workflows/cmake.yml/badge.svg)](https://github.com/QtExcel/QXlsx/actions/workflows/cmake.yml) [![cmake-ubuntu](https://github.com/QtExcel/QXlsx/actions/workflows/cmake-ubuntu.yml/badge.svg)](https://github.com/QtExcel/QXlsx/actions/workflows/cmake-ubuntu.yml)

- See [tested environments](TestEnv.md) for more information.

## Contributions
- See [contributors](https://github.com/QtExcel/QXlsx/graphs/contributors).

## License and links
- QXlsx is under MIT license. [https://github.com/QtExcel/QXlsx](https://github.com/QtExcel/QXlsx)
- Thank you for creating the following amazing projects. :+1:
  - Qt is under LGPL v3 license or Commercial license. [https://www.qt.io/](https://www.qt.io/) 
  - QtXlsxWriter is under MIT license. :+1: [https://github.com/dbzhang800/QtXlsxWriter](https://github.com/dbzhang800/QtXlsxWriter)
  - Qt-Table-Printer is under BSD 3-Clause license. [https://github.com/T0ny0/Qt-Table-Printer](https://github.com/T0ny0/Qt-Table-Printer) 
  - recurse is under MIT license. [https://github.com/pkoretic/recurse](https://github.com/pkoretic/recurse)
  - libfort is under MIT license. [https://github.com/seleznevae/libfort](https://github.com/seleznevae/libfort)
  - colorprintf is under MIT license. [https://github.com/VittGam/colorprintf](https://github.com/VittGam/colorprintf)
  - HelloActions-Qt is under MIT license. [https://github.com/jaredtao/HelloActions-Qt](https://github.com/jaredtao/HelloActions-Qt)  

## :email: Contact
- Leave me a issue. [https://github.com/QtExcel/QXlsx/issues](https://github.com/QtExcel/QXlsx/issues)
	- If you are interested in participating in the project, please contact us by issue.
	- My native language is not English and my English is not fluent. Please, use EASY English. :-)
- If you would like to translate README.md into your native language, please contact me.
	- You can either raise an issue or use a pull request. (such as README.ko.md)
- For issues on this fork contact me [here](https://github.com/alexnovichkov/QXlsx/issues)
	
## Similar projects

### :star: <b>Qxlnt</b> [https://github.com/QtExcel/Qxlnt](https://github.com/QtExcel/Qxlnt)

<p align="center"><img src="https://github.com/QtExcel/Qxlnt/raw/master/markdown-data/Concept-QXlnt.jpg"></p>

- Qxlnt is a helper project that allows xlnt to be used in Qt.
- xlnt is a excellent C++ library for using xlsx Excel files.
- I was looking for a way to make it easy to use in Qt. Of course, cmake is compatible with Qt, but it is not convenient to use. So I created Qxlnt.

### :star: <b>Qlibxlsxwriter</b> [https://github.com/QtExcel/Qlibxlsxwriter](https://github.com/QtExcel/Qlibxlsxwriter)

<p align="center"><img src="https://github.com/QtExcel/Qlibxlsxwriter/raw/master/markdown.data/logo.png"></p>

- Qlibxlsxwriter is a helper project that allows libxlsxwriter to be used in Qt.
- libxlsxwriter is a C library for creating Excel XLSX files.

### :star: <b>QSimpleXlsxWriter</b> [https://github.com/QtExcel/QSimpleXlsxWriter](https://github.com/QtExcel/QSimpleXlsxWriter)

- Use SimpleXlsxWriter in Qt.
- SimpleXlsxWriter is C++ library for creating XLSX files for MS Excel 2007 and above.
