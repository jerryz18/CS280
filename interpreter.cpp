/* Implementation of Recursive-Descent Parser
 * parse.cpp
 * Programming Assignment 2
 * Spring 2021
*/

#include "parserInt.h"
#include <string>
#include <iostream>

#include <queue>
static int error_count = 0;
map<string, bool> defVar;
map<string, Token> SymTable;
map<string, Value> TempResults;
queue <Value> *ValQue;
string empty;

namespace Parser {
    bool pushed_back = false;
    LexItem	pushed_token;

    static LexItem GetNextToken(istream& in, int& line) {
        if( pushed_back ) {
            pushed_back = false;
            return pushed_token;
        }
        return getNextToken(in, line);
    }

    static void PushBackToken(LexItem & t) {
        if( pushed_back ) {
            abort();
        }
        pushed_back = true;
        pushed_token = t;
    }

}

int ErrCount()
{
    return error_count;
}

void ParseError(int line, string msg)
{
    ++error_count;
    cout << line << ": " << msg << endl;

}

//Program is: Prog = PROGRAM IDENT {Decl} {Stmt} END PROGRAM IDENT
bool Prog(istream& in, int& line)
{
    bool dl = false, sl = false;
    LexItem tok = Parser::GetNextToken(in, line);

    if (tok.GetToken() == PROGRAM) {
        tok = Parser::GetNextToken(in, line);
        if (tok.GetToken() == IDENT) {
            empty = tok.GetLexeme();
            dl = Decl(in, line);
            if( !dl  )
            {
                ParseError(line, "Incorrect Declaration in Program");
                return false;
            }
            sl = Stmt(in, line);
            if( !sl  )
            {
                ParseError(line, "Incorrect Statement in program");
                return false;
            }
            tok = Parser::GetNextToken(in, line);

            if (tok.GetToken() == END) {
                tok = Parser::GetNextToken(in, line);

                if (tok.GetToken() == PROGRAM) {
                    tok = Parser::GetNextToken(in, line);

                    if (tok.GetToken() == IDENT) {
                        if(empty == tok.GetLexeme()){
                            return true;
                        }
                    }
                    else
                    {
                        ParseError(line, "Missing Program Name");
                        return false;
                    }
                }
                else
                {
                    ParseError(line, "Missing PROGRAM at the End");
                    return false;
                }
            }
            else
            {
                ParseError(line, "Missing END of Program");
                return false;
            }
        }
    }
    else if(tok.GetToken() == ERR){
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << tok.GetLexeme() << ")" << endl;
        return false;
    }

    return false;
}

//Decl = Type : VarList
//Type = INTEGER | REAL | CHAR
bool Decl(istream& in, int& line) {
    bool status = false;
    LexItem tok;

    LexItem t = Parser::GetNextToken(in, line);

    if(t == INTEGER || t == REAL || t == CHAR) {
        tok = t;
        tok = Parser::GetNextToken(in, line);
        if (tok.GetToken() == COLON) {
            status = IdList(in, line, t);
            //cout<< tok.GetLexeme() << " " << (status? 1: 0) << endl;
            if (status){
                status = Decl(in, line);
                return status;
            }
        }
        else{
            ParseError(line, "Missing Colon");
            return false;
        }
    }

    Parser::PushBackToken(t);
    return true;
}

//Stmt is either a PrintStmt, ReadStmt, IfStmt, or an AssigStmt
//Stmt = AssigStmt | IfStmt | PrintStmt | ReadStmt
bool Stmt(istream& in, int& line) {
    bool status;
    LexItem t = Parser::GetNextToken(in, line);
    switch( t.GetToken() ) {

        case PRINT:
            status = PrintStmt(in, line);
            if(status)
                status = Stmt(in, line);
            break;

        case IF:
            status = IfStmt(in, line);
            if(status)
                status = Stmt(in, line);
            break;

        case IDENT:
            Parser::PushBackToken(t);
            status = AssignStmt(in, line);
            if(status)
                status = Stmt(in, line);
            break;

        case READ:
            status = ReadStmt(in, line);

            if(status)
                status = Stmt(in, line);
            break;

        default:
            Parser::PushBackToken(t);
            return true;
    }

    return status;
}

//PrintStmt:= print, ExpreList
bool PrintStmt(istream& in, int& line) {
    LexItem t;
    /*createanemptyqueueofValueobjects.*/
    ValQue = new queue<Value>;
    if((t=Parser::GetNextToken(in,line))!=COMA){
        ParseError(line,"MissingaComma");
        return false;
    }
    bool ex = ExprList(in,line);
    if(!ex){
        ParseError(line,"Missing expression after print");
        while(!(*ValQue).empty()){
            ValQue->pop();
        }
        delete ValQue;
        return false;
    }
    while(!(*ValQue).empty()){
        Value nextVal=(*ValQue).front();
        cout<<nextVal;
        ValQue->pop();
    }
    cout<<endl;
    return ex;
}

//IfStmt:= if (Expr) then {Stmt} END IF
bool IfStmt(istream& in, int& line) {
    Value val1, val2;
    bool ex;
    LexItem t;

    if((t=Parser::GetNextToken(in, line)) != LPAREN ) {
        ParseError(line, "Missing Left Parenthesis");
        return false;
    }
    ex = LogicExpr(in, line, val1);
    if(!ex) {
        ParseError(line, "Missing if statement Logic Expression");
        return false;
    }
    if((t=Parser::GetNextToken(in, line)) != RPAREN ) {
        ParseError(line, "Missing Right Parenthesis");
        return false;
    }
    if((t=Parser::GetNextToken(in, line)) != THEN ) {
        ParseError(line, "Missing THEN");
        return false;
    }
    if(val1.GetBool()) {
        bool s = Stmt(in, line);
        if (!s) {
            ParseError(line, "Missing statement for IF");
            return false;
        }
    }
    else{
        while(t!=END){
            t = Parser::GetNextToken(in, line);
        }
        Parser::PushBackToken(t);
    }
    if((t = Parser::GetNextToken(in, line)) != END ) {
        ParseError(line, "Missing END of IF");
        return false;
    }
    if((t=Parser::GetNextToken(in, line)) != IF ) {
        ParseError(line, "Missing IF at End of IF statement");
        return false;
    }
    return true;
}

bool ReadStmt(istream& in, int& line){
    LexItem t;
    if( (t=Parser::GetNextToken(in, line)) != COMA ) {

        ParseError(line, "Missing a Comma");
        return false;
    }
    bool ex = VarList(in, line);
    if( !ex ) {
        ParseError(line, "Missing Variable after Read Statement");
        return false;
    }
    return ex;
}

//IdList:= IDENT {,IDENT}
bool IdList(istream& in, int& line, LexItem & tok) {
    bool status = false;
    string identstr;

    LexItem tok1 = Parser::GetNextToken(in, line);
    if(tok1 == IDENT)
    {
        //set IDENT lexeme to the type tok value
        identstr = tok1.GetLexeme();
        if (!(defVar.find(identstr)->second))
        {
            defVar[identstr] = true;
            SymTable[identstr] = tok.GetToken();
        }
        else
        {
            ParseError(line, "Variable Redefinition");
            return false;
        }

    }
    else
    {
        ParseError(line, "Missing Variable");
        return false;
    }

    tok1 = Parser::GetNextToken(in, line);

    if (tok1 == COMA) {
        status = IdList(in, line, tok);
    }
    else if(tok1.GetToken() == ERR){
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << tok.GetLexeme() << ")" << endl;
        return false;
    }
    else{
        Parser::PushBackToken(tok1);
        return true;
    }
    return status;
}

//VarList
bool VarList(istream& in, int& line)
{
    bool status = false;
    string identstr;
    LexItem tok;
    status = Var(in, line, tok);

    if(!status)
    {
        ParseError(line, "Missing Variable");
        return false;
    }

    LexItem tok1 = Parser::GetNextToken(in, line);

    if (tok1 == COMA) {
        status = VarList(in, line);
    }
    else if(tok1.GetToken() == ERR){
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << tok.GetLexeme() << ")" << endl;
        return false;
    }
    else{
        Parser::PushBackToken(tok);
        return true;
    }
    return status;
}

//Var:= ident
bool Var(istream& in, int& line, LexItem & tok){
    //called only from the AssignStmt function
    string identstr;
    tok = Parser::GetNextToken(in, line);
    if (tok == IDENT)
    {
        identstr = tok.GetLexeme();
        if (!(defVar.find(identstr)->second))
        {
            defVar[identstr] = true;
            ParseError(line, "Undeclared Variable");
            return false;
        }
        return true;
    }
    else if(tok.GetToken() == ERR){
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << tok.GetLexeme() << ")" << endl;
        return false;
    }
    return false;
}

//AssignStmt:= Var = Expr
bool AssignStmt(istream& in, int& line) {
    bool varstatus, status = false;
    LexItem t;
    float floatV;
    Value val1;

    varstatus = Var( in, line, t);
    string temp = t.GetLexeme();

    if (varstatus){
        t = Parser::GetNextToken(in, line);
        if (t == ASSOP){
            status = Expr(in, line, val1);
            if(!status) {
                ParseError(line, "Missing Expression in Assignment Statment");
                return status;
            }
            if(SymTable[temp]==INTEGER && val1.GetType()==VREAL){
                floatV = val1.GetReal();
                val1.SetType(VINT);
                val1.SetInt((int)floatV);
                TempResults[temp]=val1;
            }
            else{
                TempResults[temp]=val1;
            }
        }
        else if(t.GetToken() == ERR){
            ParseError(line, "Unrecognized Input Pattern");
            cout << "(" << t.GetLexeme() << ")" << endl;
            return false;
        }
        else {
            ParseError(line, "Missing Assignment Operator =");
            return false;
        }
    }
    else {
        ParseError(line, "Missing Left-Hand Side Variable in Assignment statement");
        return false;
    }
    return status;
}

//ExprList:= Expr {,Expr}
bool ExprList(istream& in, int& line) {
    bool status = false;
    Value val1;
    status = Expr(in, line, val1);
    if(!status){
        ParseError(line, "Missing Expression");
        return false;
    }
    ValQue->push(val1);
    LexItem tok = Parser::GetNextToken(in, line);

    if (tok == COMA) {
        status = ExprList(in, line);
    }
    else if(tok.GetToken() == ERR){
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << tok.GetLexeme() << ")" << endl;
        return false;
    }
    else{
        Parser::PushBackToken(tok);
        return true;
    }
    return status;
}

//Expr:= Term {(+|-) Term}
bool Expr(istream& in, int& line, Value & retVal)
{
    Value val1, val2;
    bool t1 = Term(in, line, val1);
    LexItem tok;

    if( !t1 ) {
        return false;
    }
    retVal = val1;
    tok = Parser::GetNextToken(in, line);
    if(tok.GetToken() == ERR){
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << tok.GetLexeme() << ")" << endl;
        return false;
    }
    while ( tok == PLUS || tok == MINUS )
    {
        t1 = Term(in, line, val2);
        if(!t1)
        {
            ParseError(line, "Missing operand after operator");
            return false;
        }
        if(retVal.GetType()==VCHAR||val2.GetType()==VCHAR){
            ParseError(line,"Run-Time Error-Illegal Mixed Type Operands");
            return false;
        }
        else{
            if(tok==PLUS){
                retVal=retVal+val2;
            }
            else if(tok==MINUS){
                retVal=retVal-val2;
            }
        }
        tok = Parser::GetNextToken(in, line);
        if(tok.GetToken() == ERR){
            ParseError(line, "Unrecognized Input Pattern");
            cout << "(" << tok.GetLexeme() << ")" << endl;
            return false;
        }
    }
    Parser::PushBackToken(tok);
    return true;
}

//Term:= SFactor {(*|/) SFactor}
bool Term(istream& in, int& line, Value & retVal) {
    Value val1, val2;
    bool t1 = SFactor(in, line, val1);
    LexItem t;
    if( !t1 ) {
        return false;
    }
    retVal = val1;
    t = Parser::GetNextToken(in, line);
    if(t.GetToken() == ERR){
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << t.GetLexeme() << ")" << endl;
        return false;
    }
    while(t == MULT || t == DIV )
    {
        t1 = SFactor(in, line, val2);
        if( !t1 ) {
            ParseError(line, "Missing expression after operator");
            return false;
        }
        if(retVal.GetType()==VCHAR || val2.GetType()==VCHAR){
            ParseError(line,"Run-Time Error-Illegal Mixed Type Operands");
            return false;
        }
        else{
            if(t == MULT){
                retVal = retVal * val2;
            }
            else if(t == DIV){
                if(val2.IsReal() ){
                    if(val2.GetReal()==0){
                        ParseError(line, "Run-Time Error-Illegal Division by Zero");
                        return false;
                    }
                }
                else if(val2.IsInt()){
                    if(val2.GetInt()==0){
                        ParseError(line, "Run-Time Error-Illegal Division by Zero");
                        return false;
                    }
                }
                else {
                    retVal = retVal / val2;
                }
            }
        }
        t = Parser::GetNextToken(in, line);
        if(t.GetToken() == ERR){
            ParseError(line, "Unrecognized Input Pattern");
            cout << "(" << t.GetLexeme() << ")" << endl;
            return false;
        }
        //Evaluate: evaluate the expression for multiplication or division
    }
    Parser::PushBackToken(t);
    return true;
}

//SFactor = Sign Factor | Factor
bool SFactor(istream& in, int& line, Value & retVal)
{
    LexItem t = Parser::GetNextToken(in, line);
    bool status;
    int sign = 0;
    if(t == MINUS )
    {
        sign = -1;
    }
    else if(t == PLUS){
        sign = 1;
    }
    else{
        Parser::PushBackToken(t);
    }
    status = Factor(in, line, sign, retVal);
    return status;
}

//LogicExpr = Expr (== | <) Expr
bool LogicExpr(istream&  in, int&  line, Value & retVal){
    Value val1, val2;
    bool t1 = Expr(in, line, val1);
    LexItem tok;
    retVal.SetType(VBOOL);

    if( !t1 ) {
        return false;
    }
    tok = Parser::GetNextToken(in, line);
    if(tok.GetToken() == ERR){
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << tok.GetLexeme() << ")" << endl;
        return false;
    }
    if ( tok == LTHAN  || tok == EQUAL) {
        t1 = Expr(in, line, val2);
        if(!t1) {
            ParseError(line, "Missing expression after relational operator");
            return false;
        }
        if(val1.GetType()==VCHAR || val2.GetType()==VCHAR){
            ParseError(line,"Run-Time Error-Illegal Mixed Type Operands");
            return false;
        }
        if(tok == EQUAL)
        {
            retVal = val1 == val2;
        }
        else if (tok == LTHAN){
            retVal=val1 < val2;
        }
        return true;
    }
    Parser::PushBackToken(tok);
    return true;
}

//Factor := ident | iconst | rconst | sconst | (Expr)
bool Factor(istream& in, int& line, int sign, Value & retVal)
{
    LexItem tok = Parser::GetNextToken(in, line);
    if( tok == IDENT ) {
        string lexeme = tok.GetLexeme();
        if (!(defVar.find(lexeme)->second))
        {
            ParseError(line, "Undefined Variable");
            return false;
        }
        if (TempResults.find(lexeme) == TempResults.end()) {
            ParseError(line, "Undefined Variable");
            return false;
        }
        retVal = TempResults[lexeme];
        if (sign == -1){
            if(retVal.IsReal()){
                retVal.SetReal(-(retVal.GetReal()));
            }
            if(retVal.IsInt()){
                retVal.SetInt(-(retVal.GetInt()));
            }
        }
        return true;
    }
    else if( tok == ICONST ) {
        if (sign == -1){
            retVal = Value(-(stoi(tok.GetLexeme())));
        }
        else{
            retVal = Value(stoi(tok.GetLexeme()));
        }
        return true;
    }
    else if( tok == SCONST ) {
        retVal = Value(tok.GetLexeme());
        return true;
    }
    else if( tok == RCONST ) {
        if(sign == -1){
            retVal = Value(-(stof(tok.GetLexeme())));
        }
        else{
            retVal = Value(stof(tok.GetLexeme()));
        }
        return true;
    }
    else if( tok == LPAREN ) {
        bool ex = Expr(in, line, retVal);
        if( !ex ) {
            ParseError(line, "Missing expression after (");
            return false;
        }
        if( Parser::GetNextToken(in, line) == RPAREN )
        {
            return ex;
        }
        ParseError(line, "Missing ) after expression");
        return false;
    }
    else if(tok.GetToken() == ERR){
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << tok.GetLexeme() << ")" << endl;
        return false;
    }
    ParseError(line, "Unrecognized input");
    return true;
}



