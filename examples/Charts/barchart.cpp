// barchart.cpp

#include <QtGlobal>
#include <QtCore>
#include <QDebug>

#include "xlsxdocument.h"
#include "xlsxcellrange.h"
#include "xlsxchart.h"

QXLSX_USE_NAMESPACE

/*
 * Test bar charts
 *
 */

int barChart()
{
    Document xlsx;
    auto sheet = xlsx.activeWorksheet();
    for (int i=1; i<10; ++i)
    {
        sheet->write(1, i+1, "Pos " + QString::number(i));
    }

    sheet->write(2, 1, "Set 1");
    sheet->write(3, 1, "Set 2");

    for (int i=1; i<10; ++i)
    {
        sheet->write(2, i+1, i*i*i);   //A2:A10
        sheet->write(3, i+1, i*i);    //B2:B10
    }

    // insert a new chart with top-left corner being in cell(5,4) and with size of (300px, 300px)
    Chart *barChart11 = sheet->insertChart(5, 4, QSize(300, 300));
    barChart11->addSubchart(Chart::Type::Bar);
    barChart11->setLegend(Legend::Position::Right);
    barChart11->setTitle("Row-based, no options");
    barChart11->addSeries(CellRange(1,1,3,10), nullptr, true, true, false);

    //demonstrates how to use Coordinate to set size in pixels
    Chart *barChart12 = sheet->insertChart(5, 10, Coordinate("300px"), Coordinate("300px"));
    barChart12->addSubchart(Chart::Type::Bar);
    barChart12->setLegend(Legend::Position::Right);
    barChart12->setTitle("Row-based, Clustered grouping");
    barChart12->addSeries(CellRange(1,1,3,10), nullptr, true, true, false);
    barChart12->setBarGrouping(0, Chart::BarGrouping::Clustered);

    //demonstrates how to use Coordinate to set size in mm
    Chart *barChart13 = sheet->insertChart(5, 16, Coordinate("80mm"), Coordinate("80mm"));
    barChart13->addSubchart(Chart::Type::Bar);
    barChart13->setLegend(Legend::Position::Right);
    barChart13->setTitle("Row-based, Stacked grouping");
    barChart13->addSeries(CellRange(1,1,3,10), nullptr, true, true, false);
    barChart13->setBarGrouping(0, Chart::BarGrouping::Stacked);

    Chart *barChart14 = sheet->insertChart(5, 22, QSize(300, 300));
    barChart14->addSubchart(Chart::Type::Bar);
    barChart14->setLegend(Legend::Position::Right);
    barChart14->setTitle("Row-based, PercentStacked grouping");
    barChart14->addSeries(CellRange(1,1,3,10), nullptr, true, true, false);
    barChart14->setBarGrouping(0, Chart::BarGrouping::PercentStacked);

    Chart *barChart21 = sheet->insertChart(25, 4, QSize(300, 300));
    barChart21->addSubchart(Chart::Type::Bar);
    barChart21->setLegend(Legend::Position::Right);
    barChart21->setTitle("Column-based, no options");
    barChart21->addSeries(CellRange(1,1,3,10), nullptr, true, true, true);

    Chart *barChart22 = sheet->insertChart(25, 10, QSize(300, 300));
    barChart22->addSubchart(Chart::Type::Bar);
    barChart22->setLegend(Legend::Position::Right);
    barChart22->setTitle("Column-based, Clustered grouping");
    barChart22->addSeries(CellRange(1,1,3,10), nullptr, true, true, true);
    barChart22->setBarGrouping(0, Chart::BarGrouping::Clustered);

    Chart *barChart23 = sheet->insertChart(25, 16, QSize(300, 300));
    barChart23->addSubchart(Chart::Type::Bar);
    barChart23->setLegend(Legend::Position::Right);
    barChart23->setTitle("Column-based, Stacked grouping");
    barChart23->addSeries(CellRange(1,1,3,10), nullptr, true, true, true);
    barChart23->setBarGrouping(0, Chart::BarGrouping::Stacked);

    Chart *barChart24 = sheet->insertChart(25, 22, QSize(300, 300));
    barChart24->addSubchart(Chart::Type::Bar);
    barChart24->setLegend(Legend::Position::Right);
    barChart24->setTitle("Column-based, PercentStacked grouping");
    barChart24->addSeries(CellRange(1,1,3,10), nullptr, true, true, true);
    barChart24->setBarGrouping(0, Chart::BarGrouping::PercentStacked);

    Chart *barChart31 = sheet->insertChart(45, 4, QSize(300, 300));
    barChart31->addSubchart(Chart::Type::Bar);
    barChart31->setLegend(Legend::Position::Right);
    barChart31->setTitle("Row-based, default bar direction");
    barChart31->addSeries(CellRange(1,1,3,10), nullptr, true, true, false);

    Chart *barChart32 = sheet->insertChart(45, 10, QSize(300, 300));
    barChart32->addSubchart(Chart::Type::Bar);
    barChart32->setLegend(Legend::Position::Right);
    barChart32->setTitle("Row-based, horizontal");
    barChart32->addSeries(CellRange(1,1,3,10), nullptr, true, true, false);
    barChart32->setBarDirection(0, Chart::BarDirection::Bar);

    Chart *barChart33 = sheet->insertChart(45, 16, QSize(300, 300));
    barChart33->addSubchart(Chart::Type::Bar);
    barChart33->setLegend(Legend::Position::Right);
    barChart33->setTitle("Row-based, no gap");
    barChart33->addSeries(CellRange(1,1,3,10), nullptr, true, true, false);
    barChart33->setGapWidth(0, 0);

    Chart *barChart34 = sheet->insertChart(45, 22, QSize(300, 300));
    barChart34->addSubchart(Chart::Type::Bar);
    barChart34->setLegend(Legend::Position::Right);
    barChart34->setTitle("Row-based, 100% gap");
    barChart34->addSeries(CellRange(1,1,3,10), nullptr, true, true, false);
    barChart34->setGapWidth(0, 100);

    //3D charts
    Chart *barChart41 = sheet->insertChart(65, 4, QSize(300, 300));
    barChart41->addSubchart(Chart::Type::Bar3D);
    barChart41->setLegend(Legend::Position::Right);
    barChart41->setTitle("3D bar");
    barChart41->addSeries(CellRange(1,1,3,10), nullptr, true, true, false);

    Chart *barChart42 = sheet->insertChart(65, 10, QSize(300, 300));
    barChart42->addSubchart(Chart::Type::Bar3D);
    barChart42->setLegend(Legend::Position::Right);
    barChart42->setTitle("3D bar, horizontal");
    barChart42->addSeries(CellRange(1,1,3,10), nullptr, true, true, false);
    barChart42->setBarDirection(0, Chart::BarDirection::Bar);

    Chart *barChart43 = sheet->insertChart(65, 16, QSize(300, 300));
    barChart43->addSubchart(Chart::Type::Bar3D);
    barChart43->setLegend(Legend::Position::Right);
    barChart43->setTitle("3D bar, no gap");
    barChart43->addSeries(CellRange(1,1,3,10), nullptr, true, true, false);
    barChart43->setGapWidth(0, 0);

    Chart *barChart44 = sheet->insertChart(65, 221, QSize(300, 300));
    barChart44->addSubchart(Chart::Type::Bar3D);
    barChart44->setLegend(Legend::Position::Right);
    barChart44->setTitle("3D bar, no gap depth");
    barChart44->addSeries(CellRange(1,1,3,10), nullptr, true, true, false);
    barChart44->setGapDepth(0, 0);

    Chart *barChart51 = sheet->insertChart(85, 4, QSize(300, 300));
    barChart51->addSubchart(Chart::Type::Bar3D);
    barChart51->setLegend(Legend::Position::Right);
    barChart51->setTitle("3D bar, standard shape");
    barChart51->addSeries(CellRange(1,1,3,10), nullptr, true, true, false);

    Chart *barChart52 = sheet->insertChart(85, 10, QSize(300, 300));
    barChart52->addSubchart(Chart::Type::Bar3D);
    barChart52->setLegend(Legend::Position::Right);
    barChart52->setTitle("3D bar, cone shape");
    barChart52->addSeries(CellRange(1,1,3,10), nullptr, true, true, false);
    barChart52->setBarShape(0, Series::BarShape::Cone);

    Chart *barChart53 = sheet->insertChart(85, 16, QSize(300, 300));
    barChart53->addSubchart(Chart::Type::Bar3D);
    barChart53->setLegend(Legend::Position::Right);
    barChart53->setTitle("3D bar, pyramid shape");
    barChart53->addSeries(CellRange(1,1,3,10), nullptr, true, true, false);
    barChart53->setBarShape(0, Series::BarShape::Pyramid);

    Chart *barChart54 = sheet->insertChart(85, 22, QSize(300, 300));
    barChart54->addSubchart(Chart::Type::Bar3D);
    barChart54->setLegend(Legend::Position::Right);
    barChart54->setTitle("3D bar, only 1st series cylinder");
    barChart54->addSeries(CellRange(1,1,3,10), nullptr, true, true, false);
    barChart54->series(0)->setBarShape(Series::BarShape::Cylinder);

    xlsx.saveAs("barCharts1.xlsx");

    Document xlsx2("barCharts1.xlsx");
    if (xlsx2.isLoaded()) {
        xlsx2.saveAs("barCharts2.xlsx");
    }

    return 0;
}
