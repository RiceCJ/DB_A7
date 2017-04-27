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
        mySchemaOut->appendAtt(v->getAttrs(myCatalog,tablename));
        projections.push_back(v->toString());
        // todo: add check groupings
    }
    MyDB_TablePtr myTableOut = make_shared<MyDB_Table>("outTable","outTable.bin", mySchemaOut);
    MyDB_TableReaderWriterPtr outTableReaderWriter = make_shared<MyDB_TableReaderWriter>(myTableOut,bufMgr);

    vector<string> selPredicates;
    for(auto disjunction:query.getAllDisjunctions()){
        selPredicates.push_back(disjunction->toString());
    }
    string selectPredicates = concatenatePredicates(selPredicates);

    // todo: add aggregate
    RegularSelection regSelection(inputTableReaderWriter, outTableReaderWriter,selectPredicates, projections)
    regSelection.run();

    MyDB_RecordPtr rec = outTableReaderWriter->getEmptyRecord();
    MyDB_RecordIteratorAltPtr iter = outTableReaderWriter->getIteratorAlt();

    int size = 0;
    while(iter->advance()){
        iter->getCurrent(rec);
        if(size < 30){
            cout<<rec<<endl;
        }
        size++;
    }
    cout<<endl<<"Total size is: "<<size<<endl;
}

string MyRelOperation::concatenatePredicates(vector<string> predicates) {
    int size = predicates.size();
    if(size==1) return predicates.front();
    string ret= predicates[0];
    for(int i=1;i<size;i++){
        ret= "&& (" + ret + ", "+ predicates[i] + ")";
    }
    return ret;
}
