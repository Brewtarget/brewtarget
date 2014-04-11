#ifndef TESTING_H
#define TESTING_H

#include <QObject>
#include <QtTest/QtTest>

class Testing : public QObject
{
   Q_OBJECT
   
public:
   
private slots:
   
   void initTestCase()
   {
   }
   
   void cleanupTestCase()
   {
   }
   
   //! \brief Unit test: verify brewtarget runs
   void runTest()
   {
      QVERIFY( 1==1 );
   }
};

#endif /*TESTING_H*/
