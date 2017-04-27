//
// Created by Liu Fang on 4/27/17.
//


#include "ParserTypes.h"
#include "MyDB_TableReaderWriter.h"
#include "MyDB_Catalog.h"
#include "ExprTree.h"
#include "MyDB_Record.h"
#include "RegularSelection.h"
#include "Aggregate.h"
#include "MyRelOperation.h"
#include "ScanJoin.h"

MyRelOperation::MyRelOperation(SQLStatement *sql, map<string, MyDB_TableReaderWriterPtr> tables,
                               MyDB_BufferManagerPtr bufMgr, MyDB_CatalogPtr myCatalog) {
    this->sql = sql;
    this->tables= tables;
    this->bufMgr = bufMgr;
    this->myCatalog = myCatalog;
}

void MyRelOperation::run() {
    SFWQuery query = sql->getQuery();
    vector<pair<string, string>> tableToProcess = query.getTables();
    string tablename = tableToProcess.front().first;
    MyDB_TableReaderWriterPtr inputTableReaderWriter = nullptr;
    if(tableToProcess.size()==1){
        string path = "./"+tablename+ ".tbl";
        inputTableReaderWriter->loadFromTextFile(path);
    }else{
        inputTableReaderWriter = joinTable();
    }

    //prepare for output table
    MyDB_SchemaPtr mySchemaOut = make_shared<MyDB_Schema>();
    vector<ExprTreePtr> valueToSelect = query.getSelects();
    vector<string> projections;

    //append select values to output schema
    for(auto v:valueToSelect){
        mySchemaOut->appendAtt(v->getAttr());
        projections.push_back(v->toString());
    }
}