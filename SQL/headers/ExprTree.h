
#ifndef SQL_EXPRESSIONS
#define SQL_EXPRESSIONS

#include "MyDB_AttType.h"
#include <string>
#include <vector>
#include <MyDB_Catalog.h>

// create a smart pointer for database tables
using namespace std;
class ExprTree;
typedef shared_ptr <ExprTree> ExprTreePtr;
enum MyDB_ExpAttType {boolType, doubleType, intType, stringType};
enum MyDB_ExpType{ boolExp,doubleExp,intExp,stringExp,identifierExp, minusExp,
	plusExp, timesExp, divideExp, gtExp, ltExp, neqExp, orExp, eqExp,
	notExp, sumExp, avgExp};

// this class encapsules a parsed SQL expression (such as "this.that > 34.5 AND 4 = 5")

// class ExprTree is a pure virtual class... the various classes that implement it are below
class ExprTree {

public:
	virtual string toString () = 0;
	virtual ~ExprTree () {}
	virtual pair<string, MyDB_AttTypePtr> getAttrs (MyDB_CatalogPtr catalog, string tbName) = 0;
	virtual MyDB_ExpAttType getAttType() = 0;
	virtual MyDB_ExpType getType() = 0;
};

class BoolLiteral : public ExprTree {

private:
	bool myVal;
public:
	
	BoolLiteral (bool fromMe) {
		myVal = fromMe;
	}

	string toString () {
		if (myVal) {
			return "bool[true]";
		} else {
			return "bool[false]";
		}
	}

	MyDB_ExpAttType getAttType() {
		return MyDB_ExpAttType :: boolType;
	}

	pair<string, MyDB_AttTypePtr> getAttrs (MyDB_CatalogPtr catalog, string tbName){
		return make_pair("", make_shared <MyDB_BoolAttType> ());
	}

	MyDB_ExpType getType(){
		return MyDB_ExpType::boolExp;
	}


};

class DoubleLiteral : public ExprTree {

private:
	double myVal;
public:

	DoubleLiteral (double fromMe) {
		myVal = fromMe;
	}

	string toString () {
		return "double[" + to_string (myVal) + "]";
	}

	MyDB_ExpAttType getAttType() {
		return MyDB_ExpAttType :: doubleType;
	}

	pair<string, MyDB_AttTypePtr> getAttrs (MyDB_CatalogPtr catalog, string tbName){
		return make_pair("",make_shared <MyDB_DoubleAttType>());
	}

	MyDB_ExpType getType(){
		return MyDB_ExpType::doubleExp;
	}


	~DoubleLiteral () {}
};

// this implement class ExprTree
class IntLiteral : public ExprTree {

private:
	int myVal;
public:

	IntLiteral (int fromMe) {
		myVal = fromMe;
	}

	string toString () {
		return "int[" + to_string (myVal) + "]";
	}

	MyDB_ExpAttType getAttType() {
		return MyDB_ExpAttType :: intType;
	}

	pair<string, MyDB_AttTypePtr> getAttrs (MyDB_CatalogPtr catalog, string tbName){
		return make_pair("",make_shared <MyDB_IntAttType>());
	}

	MyDB_ExpType getType(){
		return MyDB_ExpType::intExp;
	}

	~IntLiteral () {}
};

class StringLiteral : public ExprTree {

private:
	string myVal;
public:

	StringLiteral (char *fromMe) {
		fromMe[strlen (fromMe) - 1] = 0;
		myVal = string (fromMe + 1);
	}

	string toString () {
		return "string[" + myVal + "]";
	}

	MyDB_ExpAttType getAttType() {
		return MyDB_ExpAttType :: stringType;
	}

	pair<string, MyDB_AttTypePtr> getAttrs (MyDB_CatalogPtr catalog, string tbName){
		return make_pair("",make_shared <MyDB_StringAttType>());
	}

	MyDB_ExpType getType(){
		return MyDB_ExpType::stringExp;
	}


	~StringLiteral () {}
};

class Identifier : public ExprTree {

private:
	string tableName;
	string attName;
	MyDB_ExpAttType myExpAttType;
public:

	Identifier (char *tableNameIn, char *attNameIn) {
		tableName = string (tableNameIn);
		attName = string (attNameIn);
		myExpAttType = MyDB_ExpAttType::boolType;
	}


	pair<string, MyDB_AttTypePtr> getAttrs (MyDB_CatalogPtr catalog, string tbName) override {
		string type;
		string attr_name = tbName + "." + attName + ".type";

		MyDB_AttTypePtr attType;
		if(catalog -> getString(attr_name, type)){
			cout << "att name: "<<attName <<endl;
			cout<< "attr_name : "<< attr_name<<endl;
			if(type.compare("bool") == 0){
				attType = make_shared <MyDB_BoolAttType> ();
				myExpAttType = MyDB_ExpAttType::boolType;
			}
			else if(type.compare("string") == 0){
				attType = make_shared <MyDB_StringAttType> ();
				myExpAttType = MyDB_ExpAttType::stringType;
			}
			else if(type.compare("int") == 0){
				attType = make_shared <MyDB_IntAttType> ();
				myExpAttType = MyDB_ExpAttType::stringType;
			}
			else if(type.compare("double") == 0){
				attType = make_shared <MyDB_DoubleAttType> ();
				myExpAttType = MyDB_ExpAttType::stringType;
			}
		}
		return make_pair("[" + attName + "]", attType);
	}

	MyDB_ExpAttType getAttType() {
		return myExpAttType;
	}

	MyDB_ExpType getType(){
		return MyDB_ExpType::identifierExp;
	}

	//TODO: find a way to solve the tablename problem
	string toString () {
		//return "[" + tableName + "_" + attName + "]";
		return "[" + attName + "]";
	}	

	~Identifier () {}
};

class MinusOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	MinusOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "- (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

	MyDB_ExpAttType getAttType() {
		if(lhs->getAttType() == MyDB_ExpAttType::doubleType || rhs->getAttType() == MyDB_ExpAttType::doubleType){
			return MyDB_ExpAttType::doubleType;
		}
		else{
			return MyDB_ExpAttType::intType;
		}
	}

	MyDB_ExpType getType(){
		return MyDB_ExpType::minusExp;
	}


	pair<string, MyDB_AttTypePtr> getAttrs (MyDB_CatalogPtr catalog, string tbName){
		return make_pair("", make_shared <MyDB_BoolAttType> ());
	}
	
	~MinusOp () {}
};

class PlusOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	PlusOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "+ (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

	MyDB_ExpAttType getAttType() {
		if(lhs->getAttType() == MyDB_ExpAttType::stringType){
			return MyDB_ExpAttType ::stringType;
		}

		if(lhs->getAttType() == MyDB_ExpAttType::doubleType || rhs->getAttType() == MyDB_ExpAttType::doubleType){
			return MyDB_ExpAttType::doubleType;
		}
		else{
			return MyDB_ExpAttType::intType;
		}
	}

	pair<string, MyDB_AttTypePtr> getAttrs (MyDB_CatalogPtr catalog, string tbName){
		if(getAttType() == MyDB_ExpAttType::stringType){
			return make_pair("", make_shared <MyDB_StringAttType> ());
		}
		else if(getAttType() == MyDB_ExpAttType::doubleType){
			return make_pair("", make_shared <MyDB_DoubleAttType> ());
		}
		else{
			return make_pair("", make_shared <MyDB_IntAttType> ());
		}

	}

	MyDB_ExpType getType(){
		return MyDB_ExpType::plusExp;
	}



	~PlusOp () {}
};

class TimesOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	TimesOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "* (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

	MyDB_ExpAttType getAttType() {
		if(lhs->getAttType() == MyDB_ExpAttType::doubleType || rhs->getAttType() == MyDB_ExpAttType::doubleType){
			return MyDB_ExpAttType::doubleType;
		}
		else{
			return MyDB_ExpAttType::intType;
		}
	}

	pair<string, MyDB_AttTypePtr> getAttrs (MyDB_CatalogPtr catalog, string tbName){
		return make_pair("", make_shared <MyDB_BoolAttType> ());
	}

	MyDB_ExpType getType(){
		return MyDB_ExpType::timesExp;
	}


	~TimesOp () {}
};

class DivideOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	DivideOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "/ (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

	MyDB_ExpAttType getAttType() {
		if(lhs->getAttType() == MyDB_ExpAttType::doubleType || rhs->getAttType() == MyDB_ExpAttType::doubleType){
			return MyDB_ExpAttType::doubleType;
		}
		else{
			return MyDB_ExpAttType::intType;
		}
	}

	pair<string, MyDB_AttTypePtr> getAttrs (MyDB_CatalogPtr catalog, string tbName){

		return make_pair("", make_shared <MyDB_BoolAttType> ());
	}

	MyDB_ExpType getType(){
		return MyDB_ExpType::timesExp;
	}

	~DivideOp () {}
};

class GtOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	GtOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "> (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

	MyDB_ExpAttType getAttType() {
		return MyDB_ExpAttType::boolType;
	}

	MyDB_ExpType getType(){
		return MyDB_ExpType::gtExp;
	}


	pair<string, MyDB_AttTypePtr> getAttrs (MyDB_CatalogPtr catalog, string tbName){

		return make_pair("", make_shared <MyDB_BoolAttType> ());
	}



	~GtOp () {}
};

class LtOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	LtOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "< (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

	MyDB_ExpAttType getAttType() {
		return MyDB_ExpAttType::boolType;
	}

	pair<string, MyDB_AttTypePtr> getAttrs (MyDB_CatalogPtr catalog, string tbName){

		return make_pair("", make_shared <MyDB_BoolAttType> ());
	}

	MyDB_ExpType getType(){
		return MyDB_ExpType::ltExp;
	}

	~LtOp () {}
};

class NeqOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	NeqOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "!= (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

	MyDB_ExpAttType getAttType() {
		return MyDB_ExpAttType::boolType;
	}

	pair<string, MyDB_AttTypePtr> getAttrs (MyDB_CatalogPtr catalog, string tbName){
		return make_pair("", make_shared <MyDB_BoolAttType> ());
	}

	MyDB_ExpType getType(){
		return MyDB_ExpType::neqExp;
	}

	~NeqOp () {}
};

class OrOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	OrOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "|| (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

	MyDB_ExpAttType getAttType() {
		return MyDB_ExpAttType::boolType;
	}

	MyDB_ExpType getType(){
		return MyDB_ExpType::orExp;
	}

	pair<string, MyDB_AttTypePtr> getAttrs (MyDB_CatalogPtr catalog, string tbName){
		return make_pair("", make_shared <MyDB_BoolAttType> ());
	}

	~OrOp () {}
};

class EqOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	EqOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "== (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

	MyDB_ExpAttType getAttType() {
		return MyDB_ExpAttType::boolType;
	}

	pair<string, MyDB_AttTypePtr> getAttrs (MyDB_CatalogPtr catalog, string tbName){
		return make_pair("", make_shared <MyDB_BoolAttType> ());
	}

	MyDB_ExpType getType(){
		return MyDB_ExpType::eqExp;
	}


	~EqOp () {}
};

class NotOp : public ExprTree {

private:

	ExprTreePtr child;
	
public:

	NotOp (ExprTreePtr childIn) {
		child = childIn;
	}

	string toString () {
		return "!(" + child->toString () + ")";
	}

	MyDB_ExpAttType getAttType() {
		return MyDB_ExpAttType::boolType;
	}

	MyDB_ExpType getType(){
		return MyDB_ExpType::notExp;
	}


	pair<string, MyDB_AttTypePtr> getAttrs (MyDB_CatalogPtr catalog, string tbName){
		return make_pair("", make_shared <MyDB_BoolAttType> ());
	}

	~NotOp () {}
};

class SumOp : public ExprTree {

private:

	ExprTreePtr child;
	
public:

	SumOp (ExprTreePtr childIn) {
		child = childIn;
	}

	string toString () {
		return "sum(" + child->toString () + ")";
	}

	MyDB_ExpAttType getAttType() {
		return child->getAttType();
	}

	MyDB_ExpType getType(){
		return MyDB_ExpType::sumExp;
	}

	pair<string, MyDB_AttTypePtr> getAttrs (MyDB_CatalogPtr catalog, string tbName){
		return make_pair("sum", make_shared <MyDB_DoubleAttType> ());
	}

	~SumOp () {}
};

class AvgOp : public ExprTree {

private:

	ExprTreePtr child;
	
public:

	AvgOp (ExprTreePtr childIn) {
		child = childIn;
	}

	string toString () {
		return "avg(" + child->toString () + ")";
	}

	MyDB_ExpAttType getAttType() {
		return child->getAttType();
	}

	MyDB_ExpType getType(){
		return MyDB_ExpType::avgExp;
	}

	pair<string, MyDB_AttTypePtr> getAttrs (MyDB_CatalogPtr catalog, string tbName){
		return make_pair("avg", make_shared <MyDB_DoubleAttType> ());
	}

	~AvgOp () {}
};

#endif
