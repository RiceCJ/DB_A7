//
// Created by Liu Fang on 4/27/17.
//

#ifndef DBMS_TEST_MYRELOPERATION_H
#define DBMS_TEST_MYRELOPERATION_H

#include "ParserTypes.h"
#include "MyDB_TableReaderWriter.h"
#include "MyDB_Catalog.h"

class MyRelOperation {

public:

    MyRelOperation( SQLStatement *sql, map<string, MyDB_TableReaderWriterPtr> tables, MyDB_BufferManagerPtr bufMgr
                    , MyDB_CatalogPtr myCatalog, string dirname);

    void run();

private:

    SQLStatement *sql;
    map <string, MyDB_TableReaderWriterPtr> tables;
    MyDB_BufferManagerPtr bufMgr;
    MyDB_CatalogPtr myCatalog;
    string dirname;

    MyDB_TableReaderWriterPtr joinTable();
    string concatenatePredicates(vector<string> predicates);
};
#endif //DBMS_TEST_MYRELOPERATION_H
