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
                    , MyDB_CatalogPtr myCatalog);

    void run();

private:

    SQLStatement *sql;
    map <string, MyDB_TableReaderWriterPtr> tables;
    MyDB_BufferManagerPtr bufMgr;
    MyDB_CatalogPtr myCatalog;

    string parseStringPredicate(vector<string> allPredicates);
    MyDB_TableReaderWriterPtr joinTable();
};
#endif //DBMS_TEST_MYRELOPERATION_H
