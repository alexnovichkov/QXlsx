// richtext.cpp

#include <QtGlobal>
#include <QtCore>

#include "xlsxdocument.h"
#include "xlsxrichstring.h"
#include "xlsxworkbook.h"
#include "xlsxformat.h"

int richtext()
{
    QXlsx::Document xlsx;

    QXlsx::Format blue;
    blue.setFontColor(Qt::blue);
    QXlsx::Format red;
    red.setFontColor(Qt::red);
    red.setFontSize(15);
    QXlsx::Format bold;
    bold.setFontBold(true);

    QXlsx::RichString rich;
    rich.addFragment("Hello ", blue);
    rich.addFragment("Qt ", red);
    rich.addFragment("Xlsx", bold);
    xlsx.write("B2", rich);

    xlsx.workbook()->setHtmlToRichStringEnabled(true);
    xlsx.write("B4", "<b>Hello</b> <font color=\"red\">Qt</font> <i>Xlsx</i>");

    xlsx.write("B6", "<font color=\"red\"><b><u><i>Qt Xlsx</i></u></b></font>");

    xlsx.saveAs("richtext1.xlsx");

    QXlsx::Document("richtext1.xlsx");
    xlsx.saveAs("richtext2.xlsx");

    return 0;
}
