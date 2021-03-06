/* This file is part of Into.
 * Copyright (C) Intopii 2013.
 * All rights reserved.
 *
 * Licensees holding a commercial Into license may use this file in
 * accordance with the commercial license agreement. Please see
 * LICENSE.commercial for commercial licensing terms.
 *
 * Alternatively, this file may be used under the terms of the GNU
 * Affero General Public License version 3 as published by the Free
 * Software Foundation. In addition, Intopii gives you special rights
 * to use Into as a part of open source software projects. Please
 * refer to LICENSE.AGPL3 for details.
 */

#ifndef _PIICONFUSIONMATRIX_H
#define _PIICONFUSIONMATRIX_H

#include <PiiMatrix.h>
#include <QStringList>
#include <PiiMathDefs.h>
#include "PiiClassificationGlobal.h"
#include <cstdlib>
#include <cstdio>

/**
 * Confusion matrix is a handy tool for inspecting classification
 * results. A confusion matrix is essentially a two-dimensional
 * histogram that is indexed by the real class index (rows) and the
 * classification result (columns). Entries on the diagonal represent
 * correct classifications. The example below shows a perfect
 * classification result:
 *
 * ~~~(c++)
 *        |    O           |
 *        |    r    A      |
 *        |    a    p      |
 *        |    n    p    C |
 *        |    g    l    a |
 *        |    e    e    t | Error
 * -------+----------------+-------
 * Orange |    5    .    . |   0.0
 *  Apple |    .    4    . |   0.0
 *    Cat |    .    .    9 |   0.0
 * -------+----------------+-------
 *  Mixup |  0.0  0.0  0.0 |  %
 *
 * Number of classes       : 3
 * Total number of entries : 18
 * Correctly classified    : 18
 * Incorrectly classified  : 0
 * Total error             : 0 %
 * ~~~
 *
 */
class PiiConfusionMatrix : public PiiMatrix<int>
// No PII_CLASSIFICATION_EXPORT here. MSVC fails if this class is exported.
{
public:
  /**
   * Create a confusion matrix for the given amount of classes.
   */
  PiiConfusionMatrix(int classes = 0);

  /**
   * Copy a confusion matrix.
   */
  PiiConfusionMatrix(const PiiConfusionMatrix& mat);

  /**
   * Use an existing matrix as a confusion matrix. If `mat` is not
   * square, only a square portion of its top left corner will be
   * used.
   */
  PiiConfusionMatrix(const PiiMatrix<int>& mat);

  /**
   * Copy the contents of another matrix.
   */
  PiiConfusionMatrix& operator= (const PiiConfusionMatrix& other);

  /**
   * Add a classification result to the confusion matrix. This
   * function is equal to `mat(correctClass,
   * classification)++`, but it ensures that the indices do not
   * exceed matrix dimensions. If they do, the matrix will be
   * extended. If either index is negative, the function does nothing.
   */
  void addEntry(int correctClass, int classification);

  /**
   * Get the error percentage for the whole confusion matrix.
   *
   * @return a floating-point number within [0,1]
   */
  double error() const;
  /**
   * Get the error percentage for a row in the matrix.
   *
   * @return a floating-point number within [0,1]
   */
  double error(int row) const;
  /**
   * Get the mixup percentage for a column. The mixup percentage
   * tells the fraction of correctly classified samples among all
   * samples classified to a certain class.
   *
   * @return a floating-point number within [0,1]
   */
  double mixup(int column) const;

#ifndef PII_NO_QT
  /**
   * Print the confusion matrix to an output stream. The class names
   * will be taken from `classNames`, and the indicated amount of
   * space will be reserved for each matrix column.
   *
   * @param out the output stream
   *
   * @param classNames list of classes
   *
   * @param space column width-1 (maximum length of a matrix entry
   * represented as a decimal number)
   *
   * ~~~(c++)
   * PiiConfusionMatrix mat(2);
   * // ... fill data ...
   * mat.print(std::cout, QStringList() << "Good" << "Poor");
   * ~~~
   */
  template <class Stream> void print(Stream& out, const QStringList& classNames = QStringList(), int space = 4) const;
#endif
};

#ifndef PII_NO_QT
template <class Stream> void PiiConfusionMatrix::print(Stream& out, const QStringList& classNames, int space) const
{
  char bfr[32];
  int max=0;
  space = qBound(2, space, 20);

  //Find the maximum length for a class name
  for (int i=0; i<classNames.size(); i++)
    {
      int len = classNames[i].size();
      if (len > max)
        max = len;
    }
  if (classNames.size() < rows() && max < 8)
    max = 8;
  int hMax = max;
  if (hMax < 5) hMax = 5;

  //Print names vertically
  for (int i=0; i<max; i++)
    {
      for (int j=0; j<hMax; j++)
        out << ' ';
      out << " |";
      for (int j=0; j<columns(); j++)
        {
          QString name = j < classNames.size() ? classNames[j] : QString("class%1").arg(j,3,int(10),QChar('0'));
          int len = name.size();
          for (int k=0; k<space; k++)
            out << ' ';
          if (len >= max-i)
            out << name[len-max+i].toLatin1();
          else
            out << ' ';
        }
      out << " |";
      if (i==max-1)
        out << " Error";
      out << "\n";
    }
  //Print line
  for (int i=0; i<hMax+columns()*(space+1)+11; i++)
    {
      if (i!=hMax+1 && i!=hMax+columns()*(space+1)+3)
        out << '-';
      else
        out << "+";
    }
  out << "\n";

  int correct = 0, wrong = 0;
  //Print rows
  for (int i=0; i<rows(); i++)
    {
      const int* ptr = row(i);
      QString name = i < classNames.size() ? classNames[i] : QString("class%1").arg(i,3,10,QChar('0'));
      //Class names first
      int remainder = hMax-name.size();
      for (int j=0; j<remainder; j++)
        out << ' ';
      out << qPrintable(name);
      out << " |";
      //Then data
      for (int j=0; j<rows(); j++,ptr++)
        {
          if (i!=j)
            wrong += *ptr;
          else
            correct += *ptr;

          if (!*ptr)
            strcpy(bfr,".");
          else
            sprintf(bfr,"%d",*ptr);
          for (int k=0; k<space-(int)std::strlen(bfr)+1; k++)
            out << ' ';
          out << bfr;
        }
      //Finally, error percentage
      double dError = error(i);
      if (!Pii::isNan(dError))
        sprintf(bfr, "%5.1f", dError*100);
      else
        strcpy(bfr, "  N/A");
      out << " | " << bfr << "\n";
    }
  //Print line
  for (int i=0; i<hMax+columns()*(space+1)+11; i++)
    {
      if (i!=hMax+1 && i!=hMax+columns()*(space+1)+3)
        out << '-';
      else
        out << "+";
    }
  out << "\n";
  //Mixup percentages
  for (int i=0; i<hMax-5; i++)
    out << ' ';
  out << "Mixup |";
  for (int i=0; i<columns(); i++)
    {
      double dMixup = mixup(i);
      if (Pii::isNan(dMixup))
        strcpy(bfr, "N/A");
      else if (dMixup != 1.0)
        sprintf(bfr, "%.1f" , dMixup*100);
      else
        strcpy(bfr, "100");
      for (int k=0; k<space-(int)std::strlen(bfr)+1; k++)
        out << ' ';
      out << bfr;
    }
  out << " |  %" << "\n";

  out << "\n"
      << "Number of classes       : " << columns() << "\n"
      << "Total number of entries : " << (wrong+correct) << "\n"
      << "Correctly classified    : " << correct << "\n"
      << "Incorrectly classified  : " << wrong << "\n"
      << "Total error             : " << (double(wrong*100)/double(wrong+correct)) << " %\n";
}
#endif

#endif //_PIICONFUSIONMATRIX_H
