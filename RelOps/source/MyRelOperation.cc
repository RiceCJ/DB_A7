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


MyDB_TableReaderWriterPtr MyRelOperation::joinTable() {

    SFWQuery query = sql->getQuery();
    vector <pair <string, string>> tablesToProcess = query.getTables();
    string leftTbName = tablesToProcess[1].first;
    string rightTbName = tablesToProcess[0].first;
    string leftAlias = tablesToProcess[1].second;
    string rightAlias = tablesToProcess[0].second;
    cout << "left table: " << leftTbName << "\n";
    cout << "right table: " << rightTbName << "\n";

    MyDB_TableReaderWriterPtr leftTableReaderWriter = tables[leftTbName];
    MyDB_TableReaderWriterPtr rightTableReaderWriter = tables[rightTbName];
    leftTableReaderWriter->loadFromTextFile("./"+leftTbName+".tbl");
    rightTableReaderWriter->loadFromTextFile("./"+rightTbName+".tbl");

    MyDB_SchemaPtr mySchemaOut = make_shared <MyDB_Schema> ();
    vector <string> projections;
    MyDB_SchemaPtr leftSchema = leftTableReaderWriter->getTable()->getSchema();
    vector <pair <string, MyDB_AttTypePtr>> leftAtts = leftSchema -> getAtts();
    for (auto &p : leftAtts){
        mySchemaOut->appendAtt(p);
        projections.push_back("["+p.first+"]");
    }
    MyDB_SchemaPtr rightSchema = rightTableReaderWriter->getTable()->getSchema();
    vector <pair <string, MyDB_AttTypePtr>> rightAtts = rightSchema -> getAtts();
    for (auto &p : rightAtts){
        mySchemaOut->appendAtt(p);
        projections.push_back("["+p.first+"]");
    }

    MyDB_TablePtr myTableOut = make_shared <MyDB_Table> ("tableOut", "tableOut.bin", mySchemaOut);
    MyDB_TableReaderWriterPtr outputTableReadWriter = make_shared <MyDB_TableReaderWriter> (myTableOut, bufMgr);

    vector <ExprTreePtr> allDisjunctions = query.getAllDisjunctions();
    vector <pair <string, string>> hashAtts;
    vector <string> finalPredicates;
    vector <string> leftPredicates;
    vector <string> rightPredicates;

    for (auto v : allDisjunctions){
        pair<bool, string> isJoinPair = v->IsJoinPredicate();
        if (isJoinPair.first){
            string delimiter = "|";
            string atts = isJoinPair.second;
            int pos = atts.find(delimiter);
            string leftAtt = atts.substr(0, pos);
            string rightAtt = atts.substr(pos+1, atts.size()-pos-1);
            finalPredicates.push_back(v->toString());
            hashAtts.push_back(make_pair(leftAtt, rightAtt));
            cout << v->toString() << "\n";
            cout << "atts: " << atts << "\n";
            cout << "left att: " << leftAtt << "\n";
            cout << "right att: " << rightAtt << "\n";
        }
        else{
            if (isJoinPair.second == leftAlias){
                leftPredicates.push_back(v->toString());
            }
            else{
                rightPredicates.push_back(v->toString());
            }

        }
    }

    string finalSelectionPredicate = concatenatePredicates(finalPredicates);
    string leftSelectionPredicate = concatenatePredicates(leftPredicates);
    string rightSelectionPredicate = concatenatePredicates(rightPredicates);
    cout << "final sel: " << finalSelectionPredicate << "\n";
    cout << "left sel: " << leftSelectionPredicate << "\n";
    cout << "right sel: " << rightSelectionPredicate << "\n";


    ScanJoin myOp (leftTableReaderWriter, rightTableReaderWriter, outputTableReadWriter,
                   finalSelectionPredicate, projections, hashAtts, leftSelectionPredicate, rightSelectionPredicate);
    myOp.run();
    return outputTableReadWriter;

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
