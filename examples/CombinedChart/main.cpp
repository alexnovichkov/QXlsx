#include <QtGlobal>
#include <QCoreApplication>
#include <QtCore>
#include <QVariant>
#include <QDir>
#include <QDebug>

#include <iostream>
using namespace std;

#include "xlsxdocument.h"
#include "xlsxcellrange.h"
#include "xlsxchart.h"
using namespace QXlsx;

int main()
{
    QXlsx::Document xlsx;
    auto sheet = xlsx.activeWorksheet();

    for (int i=1; i<10; ++i) {
        sheet->write(1, i+1, "Pos " + QString::number(i));
    }

    sheet->write(2, 1, "Set 1");
    sheet->write(3, 1, "Set 2");

    for (int i=1; i<10; ++i) {
        sheet->write(2, i+1, i*i*i);   //A2:A10
        sheet->write(3, i+1, i*i);    //B2:B10
    }
    /**
    An example of a chart with series of different types.
    */

    Chart *chart1 = sheet->insertChart(4, 3, QSize(600, 600));
    chart1->setLegend(Legend::Position::Bottom);
    chart1->setTitle("Combined chart");

    //The first series - bar series
    chart1->addSubchart(Chart::Type::Bar);
    chart1->addSeries(CellRange(1,1,2,10), nullptr, true, true, false, 0);

    //The second series - line chart
    chart1->addSubchart(Chart::Type::Line, chart1->subchartAxes(0));
    chart1->addSeries(CellRange(1,1,1,10), CellRange(3,1,3,10), nullptr, true, 1);

    /**
    An example of a chart with series of different types. The first series is placed
    on the left axis, the second one is on the right axis.
    */

    Chart *chart2 = sheet->insertChart(4, 15, QSize(600, 600));
    chart2->setDefaultLegend(); //default legend is positioned on the right outside the chart
    chart2->setTitle("Combined chart with three axes");
    chart2->setAutoTitleDeleted(true);

    // add axes
    chart2->addAxis(Axis::Type::Category, Axis::Position::Bottom, "bottom axis");
    chart2->addAxis(Axis::Type::Value, Axis::Position::Left, "left axis");
    chart2->addAxis(Axis::Type::Value, Axis::Position::Right, "right axis");
    auto bottomAxis = chart2->axis(0);
    auto leftAxis = chart2->axis(1);
    auto rightAxis = chart2->axis(2);

    //link axes together
    leftAxis->setCrossAxis(bottomAxis);
    bottomAxis->setCrossAxis(leftAxis);
    rightAxis->setCrossAxis(bottomAxis);
    //right now both left and right axes are plotted on the left. We need to properly
    //position the right axis.
    rightAxis->setCrossesType(Axis::CrossesType::Maximum); // Comment out this line to see what changes.

    //chart2->title().textProperties().textShape = TextShapeType::textChevron;

    //The first series - bar series
    auto bottomAxisId = bottomAxis->id();
    auto leftAxisId = leftAxis->id();
    auto rightAxisId = rightAxis->id();
    chart2->addSubchart(Chart::Type::Bar, {bottomAxisId, leftAxisId});
    chart2->addSeries(CellRange(1,1,2,10), nullptr, true, true, false, 0);

    //The second series - line chart
    chart2->addSubchart(Chart::Type::Line, {bottomAxisId, rightAxisId});
    chart2->addSeries(CellRange(1,1,1,10), CellRange(3,1,3,10), nullptr, true, 1);

    xlsx.saveAs("combinedChart1.xlsx");

    Document xlsx2("combinedChart1.xlsx");
    if (xlsx2.isLoaded()) {
        xlsx2.saveAs("combinedChart2.xlsx");
    }

    return 0;
}
