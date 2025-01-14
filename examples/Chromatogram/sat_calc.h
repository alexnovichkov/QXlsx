#ifndef SAT_CALC_H
#define SAT_CALC_H
#include <QtCore>
#include <QString>
#include <QTextStream>
#include <iostream>
#include <QColor>

class sat_calc
{
private:
    QString chromatogram;
    double precision;
    QString report_file;
    QString sheet_name;
    int output_line_count;


public:
    void generate_report (void);
    void get_chromatogram(const QString & chromatogram);
    void get_report_file (const QString & report_file) ;
    void get_sheet_name  (const QString & sheet_name)  ;
    void get_precision   (const double    precision)   ;

    sat_calc();
    ~sat_calc();
};

#endif // SAT_CALC_H
