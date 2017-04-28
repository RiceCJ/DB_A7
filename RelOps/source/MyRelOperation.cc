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
#include <stdio.h>

MyRelOperation::MyRelOperation(SQLStatement *sql, map<string, MyDB_TableReaderWriterPtr> tables,
                               MyDB_BufferManagerPtr bufMgr, MyDB_CatalogPtr myCatalog, string dirname) {
    this->sql = sql;
    this->tables= tables;
    this->bufMgr = bufMgr;
    this->myCatalog = myCatalog;
    this->dirname = dirname;
}


MyDB_TableReaderWriterPtr MyRelOperation::joinTable() {

    //TODO:
    return nullptr;

}

void MyRelOperation::run() {
    SFWQuery query = sql->getQuery();
    vector<pair<string, string>> tableToProcess = query.getTables();
    string tablename = tableToProcess.front().first;
    MyDB_TableReaderWriterPtr inputTableReaderWriter = nullptr;
    if(tableToProcess.size()==1){
//        cout<< "Only one table here!"<<endl;
        inputTableReaderWriter = tables[tablename];
//        cout<< "input table load successfully! " <<endl;

    }else{
        inputTableReaderWriter = joinTable();
    }

    //prepare for output table
    MyDB_SchemaPtr mySchemaOut = make_shared<MyDB_Schema>();
    vector<ExprTreePtr> valueToSelect = query.getSelects();
    vector<string> projections;
//    cout<< "get "<< valueToSelect.size() << " selects."<<endl;
    vector <pair <MyDB_AggType, string>> aggsToCompute;
    vector<string> groups;
    bool hasAgg=false;
    //append select values to output schema
    for(auto v:valueToSelect){
        mySchemaOut->appendAtt(v->getAttrs(myCatalog,tablename));
        projections.push_back(v->toString());
//        cout<<v->toString()<<endl;
        // todo: add check groupings
        if(v->getType() == MyDB_ExpType::sumExp){
            hasAgg = true;
            aggsToCompute.push_back(make_pair(MyDB_AggType::aggSum, v->toString().substr(3)));
        }else if(v->getType() == MyDB_ExpType::avgExp){
            hasAgg = true;
            aggsToCompute.push_back(make_pair(MyDB_AggType::aggAvg, v->toString().substr(3)));
        }else{
            groups.push_back(v->toString());
        }
    }

//    cout<< "total attrs:" << mySchemaOut->getAtts().size() <<endl;


    MyDB_TablePtr myTableOut = make_shared<MyDB_Table>("outTable",dirname+"/outTable.bin", mySchemaOut);
    MyDB_TableReaderWriterPtr outTableReaderWriter = make_shared<MyDB_TableReaderWriter>(myTableOut,bufMgr);

    vector<string> selPredicates;
    for(auto disjunction:query.getAllDisjunctions()){
        selPredicates.push_back(disjunction->toString());
//        cout<<disjunction->toString()<<endl;
    }
    string selectPredicates = concatenatePredicates(selPredicates);
//    cout<<selectPredicates<<endl;

    // todo: add aggregate
    if(hasAgg){
        Aggregate aggOp (inputTableReaderWriter, outTableReaderWriter, aggsToCompute, groups, selectPredicates);
        aggOp.run();
    }else{
        RegularSelection regSelection(inputTableReaderWriter, outTableReaderWriter,selectPredicates, projections);
        regSelection.run();
    }


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

    int err = remove((dirname+"/outTable.bin").c_str());
    if(err == -1){
        cout<<"Fail to remove garbage file for outtable"<<endl;
    }
    else{
        cout<<"Successfully remove garbage file for outtable"<<endl;
    }
}

string MyRelOperation::concatenatePredicates(vector<string> predicates) {
    int size = predicates.size();
    if(size==1) return predicates.front();
    string ret= predicates[0];
    for(int i=1;i<size;i++){
        ret= "&& (" + ret + ", "+ predicates[i] + ")";
    }
//    cout << "predicates: " << ret << endl;
    return ret;
}
