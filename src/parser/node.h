/*
 * parser/node.h
 * Copyright (C) 2019 chenjing <chenjing@4paradigm.com>
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FESQL_PARSER_NODE_H_
#define FESQL_PARSER_NODE_H_

#include <string>
#include <list>
#include <iostream>
#include <forward_list>
namespace fesql {
namespace parser {

const std::string SPACE_ST = "+";
const std::string SPACE_ED = "";
enum SQLNodeType {
    kSelectStmt = 0,
    kExpr,
    kResTarget,
    kTable,
    kFunc,
    kWindowDef,
    kFrameBound,
    kFrames,
    kColumn,
    kConst,
    kLimit,
    kAll,
    kList,
    kOrderBy,

    kNull,
    kInt,
    kBigInt,
    kFloat,
    kDouble,
    kString,

    kDesc,
    kAsc,

    kFrameRange,
    kFrameRows,

    kPreceding,
    kFollowing,
    kCurrent,
    kUnknow
};

// Global methods
std::string NameOfSQLNodeType(const SQLNodeType &type);

class SQLNode {

public:
    SQLNode(const SQLNodeType &type, uint32_t line_num, uint32_t location)
        : type_(type), line_num_(line_num), location_(location) {
    }

    virtual ~SQLNode() {}

    virtual void Print(std::ostream &output) const {
        Print(output, SPACE_ST);
    }

    virtual void Print(std::ostream &output, const std::string &tab) const {
        output << tab << SPACE_ED << NameOfSQLNodeType(type_);
    }

    SQLNodeType GetType() const {
        return type_;
    }

    uint32_t GetLineNum() const {
        return line_num_;
    }

    uint32_t GetLocation() const {
        return location_;
    }

    friend std::ostream &operator<<(std::ostream &output, const SQLNode &thiz);

protected:
    SQLNodeType type_;
private:
    uint32_t line_num_;
    uint32_t location_;
};

struct SQLLinkedNode {
    SQLNode *node_ptr_;
    SQLLinkedNode *next_;
    SQLLinkedNode(SQLNode *node_ptr) {
        node_ptr_ = node_ptr;
        next_ = NULL;
    }
    /**
     * destruction: tobe optimized
     */
    ~SQLLinkedNode() {
        delete node_ptr_;
    }
};

class SQLNodeList {

public:
    SQLNodeList() : size_(0), head_(NULL), tail_(NULL) {
    }

    SQLNodeList(SQLLinkedNode *head, SQLLinkedNode *tail, size_t size)
        : size_(size), head_(head), tail_(tail) {
    }

    ~SQLNodeList() {
        tail_ = NULL;
        if (NULL != head_) {
            SQLLinkedNode *pre = head_;
            SQLLinkedNode *p = pre->next_;
            while (NULL != p) {
                delete pre;
                pre = p;
                p = p->next_;
            }
            delete pre;
        }
    }

    const size_t Size() {
        return size_;
    }

    void Print(std::ostream &output) const {
        Print(output, "");
    }

    void Print(std::ostream &output, const std::string &tab) const {
        if (0 == size_ || NULL == head_) {
            output << tab << "[]";
            return;
        }
        output << tab << "[\n";
        SQLLinkedNode *p = head_;
        const std::string space = tab + "\t";
        p->node_ptr_->Print(output, space);
        output << "\n";
        p = p->next_;
        while (NULL != p) {
            p->node_ptr_->Print(output, space);
            p = p->next_;
            output << "\n";
        }
        output << tab << "]";
    }

    void PushFront(SQLNode *node_ptr) {
        SQLLinkedNode *linked_node_ptr = new SQLLinkedNode(node_ptr);
        linked_node_ptr->next_ = head_;
        head_ = linked_node_ptr;
        size_ += 1;
        if (NULL == tail_) {
            tail_ = head_;
        }
    }

    void AppendNodeList(SQLNodeList *node_list_ptr) {
        if (NULL == node_list_ptr) {
            return;
        }

        if (NULL == tail_) {
            head_ = node_list_ptr->head_;
            tail_ = head_;
            size_ = node_list_ptr->size_;
            return;
        }

        tail_->next_ = node_list_ptr->head_;
        tail_ = node_list_ptr->tail_;
        size_ += node_list_ptr->size_;
    }

    SQLLinkedNode *GetHead() {
        return head_;
    }
    friend std::ostream &operator<<(std::ostream &output, const SQLNodeList &thiz);
private:
    SQLLinkedNode *head_;
    SQLLinkedNode *tail_;
    size_t size_;
};

/**
 * SQL Node for Select statement
 */
class SelectStmt : public SQLNode {
public:

    SelectStmt() : SQLNode(kSelectStmt, 0, 0), distinct_opt_(0) {
        limit_ptr_ = NULL;
        select_list_ptr_ = NULL;
        tableref_list_ptr_ = NULL;
        where_clause_ptr_ = NULL;
        group_clause_ptr_ = NULL;
        having_clause_ptr_ = NULL;
        order_clause_ptr_ = NULL;
        window_list_ptr_ = NULL;
    }

    ~SelectStmt() {
        delete limit_ptr_;
        delete tableref_list_ptr_;
        delete select_list_ptr_;
        delete where_clause_ptr_;
        delete group_clause_ptr_;
        delete having_clause_ptr_;
        delete order_clause_ptr_;
        delete window_list_ptr_;
    }

    void Print(std::ostream &output, const std::string &orgTab) const {
        SQLNode::Print(output, orgTab);
        const std::string tab = orgTab + "\t";
        const std::string space = tab + "\t";
        output << "\n";
        if (NULL == select_list_ptr_) {
            output << tab << "select_list_ptr_: NULL\n";
        } else {
            output << tab << "select_list: \n";
            select_list_ptr_->Print(output, space);
            output << "\n";
        }

        if (NULL == tableref_list_ptr_) {
            output << tab << "tableref_list_ptr_: NULL\n";
        } else {
            output << tab << "tableref_list_ptr_: \n";
            tableref_list_ptr_->Print(output, space);
            output << "\n";
        }
        if (NULL == where_clause_ptr_) {

            output << tab << "where_clause_: NULL\n";
        } else {
            output << tab << "where_clause_: \n";
            where_clause_ptr_->Print(output, tab);
            output << "\n";
        }

        if (NULL == group_clause_ptr_) {

            output << tab << "group_clause_: NULL\n";
        } else {
            output << tab << "group_clause_: \n";
            group_clause_ptr_->Print(output, tab);
            output << "\n";
        }

        if (NULL == having_clause_ptr_) {

            output << tab << "having_clause_: NULL\n";
        } else {
            output << tab << "having_clause_: " << *(having_clause_ptr_) << "\n";
        }

        if (NULL == order_clause_ptr_) {

            output << tab << "order_clause_: NULL\n";
        } else {
            output << tab << "order_clause_: " << *(order_clause_ptr_) << "\n";
        }

        if (NULL == window_list_ptr_) {
            output << tab << "window_list_ptr_: NULL\n";
        } else {
            output << tab << "window_list_ptr_: \n";
            window_list_ptr_->Print(output, space);
            output << "\n";
        }

        if (NULL == limit_ptr_) {

            output << tab << "limit_clause_: NULL\n";
        } else {
            output << tab << "limit_clause_: \n";
            limit_ptr_->Print(output, tab);
            output << "\n";
        }

    }

    SQLNodeList *GetSelectList() const {
        return select_list_ptr_;
    }

    SQLNode *GetLimit() const {
        return limit_ptr_;
    }

    SQLNodeList *GetTableRefList() const {
        return tableref_list_ptr_;
    }

    SQLNodeList *GetWindowList() const {
        return window_list_ptr_;
    }

    friend void FillSelectAttributions(SelectStmt *node_ptr,
                                       SQLNodeList *select_list_ptr,
                                       SQLNodeList *tableref_list_ptr,
                                       SQLNodeList *window_list_ptr,
                                       SQLNode *limit_ptr) {
        node_ptr->select_list_ptr_ = select_list_ptr;
        node_ptr->tableref_list_ptr_ = tableref_list_ptr;
        node_ptr->window_list_ptr_ = window_list_ptr;
        node_ptr->limit_ptr_ = limit_ptr;
    }

private:
    int distinct_opt_;
    SQLNode *limit_ptr_;
    SQLNodeList *select_list_ptr_;
    SQLNodeList *tableref_list_ptr_;
    SQLNode *where_clause_ptr_;
    SQLNode *group_clause_ptr_;
    SQLNode *having_clause_ptr_;
    SQLNode *order_clause_ptr_;
    SQLNodeList *window_list_ptr_;
};

class ResTarget : public SQLNode {
public:
    ResTarget() : SQLNode(kResTarget, 0, 0) {}
    ResTarget(const std::string &name, SQLNode *val) : SQLNode(kResTarget, 0, 0), name_(name), val_(val) {}
    ResTarget(uint32_t line_num, uint32_t location) : SQLNode(kResTarget, line_num, location), indirection_(NULL) {}
    ~ResTarget() {
        delete val_;
        delete indirection_;
    }

    void Print(std::ostream &output, const std::string &orgTab) const {
        SQLNode::Print(output, orgTab);
        output << "\n";
        const std::string tab = orgTab + "\t" + SPACE_ED;
        const std::string space = orgTab + "\t\t";
        output << tab << "val: \n";
        val_->Print(output, space);
        output << "\n";
        output << tab << "name: \n";
        output << space << name_;
    }

    std::string GetName() const {
        return name_;
    }

    SQLNode *GetVal() const {
        return val_;
    }

private:
    SQLNodeList *indirection_;    /* subscripts, field names, and '*', or NIL */
    SQLNode *val_;            /* the value expression to compute or assign */
    std::string name_;            /* column name or NULL */
};

class WindowDefNode : public SQLNode {

public:
    WindowDefNode()
        : SQLNode(kWindowDef, 0, 0), window_name_(""), partition_list_ptr_(NULL),
          order_list_ptr_(NULL), frame_ptr(NULL) {};
    ~WindowDefNode() {
        delete partition_list_ptr_;
        delete order_list_ptr_;
        delete frame_ptr;
    }

    void SetName(const std::string &name) {
        window_name_ = name;
    }

    std::string GetName() const {
        return window_name_;
    }

    SQLNodeList *GetPartitions() const {
        return partition_list_ptr_;
    }

    SQLNodeList *GetOrders() const {
        return order_list_ptr_;
    }

    SQLNode *GetFrame() const {
        return frame_ptr;
    }

    void Print(std::ostream &output, const std::string &orgTab) const {
        SQLNode::Print(output, orgTab);
        const std::string tab = orgTab + "\t";
        const std::string space = tab + "\t";
        output << "\n";

        output << tab << "window_name: " << window_name_ << "\n";
        if (NULL == partition_list_ptr_) {
            output << tab << "partition_list_ptr_: NULL\n";
        } else {
            output << tab << "partition_list_ptr_: \n";
            partition_list_ptr_->Print(output, space);
            output << "\n";
        }

        if (NULL == order_list_ptr_) {
            output << tab << "order_list_ptr_: NULL\n";
        } else {
            output << tab << "order_list_ptr_: \n";
            order_list_ptr_->Print(output, space);
            output << "\n";
        }
        if (NULL == frame_ptr) {

            output << tab << "frame_ptr: NULL";
        } else {
            output << tab << "frame_ptr: \n";
            frame_ptr->Print(output, space);
        }
    }

    friend void FillWindowSpection(WindowDefNode *node_ptr,
                                   SQLNodeList *partitions,
                                   SQLNodeList *orders,
                                   SQLNode *frame) {
        node_ptr->partition_list_ptr_ = partitions;
        node_ptr->order_list_ptr_ = orders;
        node_ptr->frame_ptr = frame;
    }

private:
    std::string window_name_;            /* window's own name */
    SQLNodeList *partition_list_ptr_;    /* PARTITION BY expression list */
    SQLNodeList *order_list_ptr_;    /* ORDER BY (list of SortBy) */
    SQLNode *frame_ptr;    /* expression for starting bound, if any */
};

class FrameBound : public SQLNode {
public:
    FrameBound() : SQLNode(kFrameBound, 0, 0), bound_type_(kPreceding), offset_(NULL) {};
    FrameBound(SQLNodeType bound_type) :
        SQLNode(kFrameBound, 0, 0), bound_type_(bound_type) {}
    FrameBound(SQLNodeType bound_type, SQLNode *offset) :
        SQLNode(kFrameBound, 0, 0), bound_type_(bound_type), offset_(offset) {}
    ~FrameBound() {
        delete offset_;
    }

    void Print(std::ostream &output, const std::string &orgTab) const {
        SQLNode::Print(output, orgTab);
        const std::string tab = orgTab + "\t";
        const std::string space = tab + "\t";
        output << "\n";
        output << tab << "bound: " << NameOfSQLNodeType(bound_type_) << "\n";
        if (NULL == offset_) {
            output << space << "UNBOUNDED";
        } else {
            offset_->Print(output, space);
        }
    }

    SQLNodeType GetBoundType() const {
        return bound_type_;
    }

    SQLNode *GetOffset() const {
        return offset_;
    }
private:
    SQLNodeType bound_type_;
    SQLNode *offset_;
};

class FrameNode : public SQLNode {
public:
    FrameNode() : SQLNode(kFrames, 0, 0), frame_type_(kFrameRange), start_(NULL), end_(NULL) {};
    FrameNode(SQLNodeType frame_type, FrameBound *start, FrameBound *end) : SQLNode(kFrames, 0, 0),
                                                                            frame_type_(frame_type),
                                                                            start_(start),
                                                                            end_(end) {};

    ~FrameNode() {
        delete start_;
        delete end_;
    }

    void SetFrameType(SQLNodeType frame_type) {
        frame_type_ = frame_type;
    }

    void Print(std::ostream &output, const std::string &orgTab) const {
        SQLNode::Print(output, orgTab);
        const std::string tab = orgTab + "\t";
        const std::string space = tab + "\t";
        output << "\n";
        output << tab << "frames_type_ : " << NameOfSQLNodeType(frame_type_) << "\n";
        if (NULL == start_) {
            output << tab << "start: UNBOUNDED: \n";
        } else {
            output << tab << "start: \n";
            start_->Print(output, space);
            output << "\n";
        }

        if (NULL == end_) {
            output << tab << "end: UNBOUNDED";
        } else {
            output << tab << "end: \n";
            end_->Print(output, space);
        }
    }

    SQLNodeType GetFrameType() const {
        return frame_type_;
    }

    SQLNode *GetStart() const {
        return start_;
    }

    SQLNode *GetEnd() const {
        return end_;
    }
private:
    SQLNodeType frame_type_;
    SQLNode *start_;
    SQLNode *end_;
};

class LimitNode : public SQLNode {
public:
    LimitNode(int cnt) : SQLNode(kLimit, 0, 0), limit_cnt_(cnt) {};
    int GetLimitCount() const {
        return limit_cnt_;
    }
private:
    int limit_cnt_;
};
class SQLExprNode : public SQLNode {
public:
    SQLExprNode() : SQLNode(kExpr, 0, 0) {}
    SQLExprNode(uint32_t line_num, uint32_t location) : SQLNode(kExpr, line_num, location) {
    }
    ~SQLExprNode() {
    }
};

class ColumnRefNode : public SQLNode {

public:
    ColumnRefNode(const std::string &column_name)
        : SQLNode(kColumn, 0, 0), column_name_(column_name), relation_name_("") {
    }

    ColumnRefNode(const std::string &column_name, const std::string &relation_name)
        : SQLNode(kColumn, 0, 0), column_name_(column_name), relation_name_(relation_name) {
    }

    std::string GetRelationName() const {
        return relation_name_;
    }

    std::string GetColumnName() const {
        return column_name_;
    }

    void Print(std::ostream &output, const std::string &orgTab) const {
        SQLNode::Print(output, orgTab);
        output << "\n";
        const std::string tab = orgTab + "\t" + SPACE_ED;
        output << tab << "column_ref: " << "{relation_name: "
               << relation_name_ << "," << " column_name: " << column_name_ << "}";
    }

private:
    std::string column_name_;
    std::string relation_name_;

};

class OrderByNode : public SQLNode {

public:
    OrderByNode(SQLNode *order) : SQLNode(kOrderBy, 0, 0), sort_type_(kDesc), order_by_(order) {}
    ~OrderByNode() {
        delete order_by_;
    }

    void Print(std::ostream &output, const std::string &orgTab) const {
        SQLNode::Print(output, orgTab);
        output << "\n";
        const std::string tab = orgTab + "\t" + SPACE_ED;
        const std::string space = orgTab + "\t" + "\t" + SPACE_ED;
        output << tab << "sort_type_: " << NameOfSQLNodeType(sort_type_) << "\n";
        if (NULL == order_by_) {
            output << tab << "order_by_: NULL\n";
        } else {
            output << tab << "order_by_: \n";
            order_by_->Print(output, space);
            output << "\n";
        }
    }

    SQLNodeType GetSortType() const {
        return sort_type_;
    }
    SQLNode *GetOrderBy() const {
        return order_by_;
    }
private:
    SQLNodeType sort_type_;
    SQLNode *order_by_;
};

class TableNode : SQLNode {

public:
    TableNode(const std::string &name, const std::string &alias)
        : SQLNode(kTable, 0, 0), org_table_name_(name), alias_table_name_(alias) {

    }

    void Print(std::ostream &output, const std::string &orgTab) const {
        SQLNode::Print(output, orgTab);
        output << "\n";
        const std::string tab = orgTab + "\t" + SPACE_ED;
        output << tab << "table: " << org_table_name_ << ", alias: " << alias_table_name_;
    }

    std::string GetOrgTableName() const {
        return org_table_name_;
    }

    std::string GetAliasTableName() const {
        return alias_table_name_;
    }

private:
    std::string org_table_name_;
    std::string alias_table_name_;
};

class FuncNode : SQLNode {

public:
    FuncNode(const std::string &function_name)
        : SQLNode(kFunc, 0, 0), function_name_(function_name), args_(NULL), over_(NULL) {};
    FuncNode(const std::string &function_name, SQLNodeList *args, WindowDefNode *over)
        : SQLNode(kFunc, 0, 0), function_name_(function_name), args_(args), over_(over) {};

    ~FuncNode() {
        delete args_;
        delete over_;
    }

    void Print(std::ostream &output, const std::string &orgTab) const {
        SQLNode::Print(output, orgTab);
        output << "\n";
        const std::string tab = orgTab + "\t" + SPACE_ED;
        const std::string space = orgTab + "\t\t";
        output << tab << "function_name: " << function_name_;
        output << "\n";
        output << tab << "args: \n";
        if (NULL == args_ || 0 == args_->Size()) {
            output << space << "[]";
        } else {
            args_->Print(output, space);
        }
        output << "\n";
        if (NULL == over_) {
            output << tab << "over: NULL\n";
        } else {
            output << tab << "over: \n";
            over_->Print(output, space);
        }
    }

    std::string GetFunctionName() const {
        return function_name_;
    }

    SQLNodeList *GetArgs() const {
        return args_;
    }

    WindowDefNode *GetOver() const {
        return over_;
    }
private:
    std::string function_name_;
    SQLNodeList *args_;
    WindowDefNode *over_;
};

class ConstNode : public SQLNode {

public:
    ConstNode() : SQLNode(kNull, 0, 0) {
    }
    ConstNode(int val) : SQLNode(kInt, 0, 0) {
        val_.vint = val;
    }
    ConstNode(long val) : SQLNode(kBigInt, 0, 0) {
        val_.vlong = val;
    }
    ConstNode(float val) : SQLNode(kFloat, 0, 0) {
        val_.vfloat = val;
    }

    ConstNode(double val) : SQLNode(kDouble, 0, 0) {
        val_.vdouble = val;
    }

    ConstNode(const char *val) : SQLNode(kString, 0, 0) {
        val_.vstr = val;
    }
    ConstNode(const std::string &val) : SQLNode(kString, 0, 0) {
        val_.vstr = val.c_str();
    }

    ~ConstNode() {}
    void Print(std::ostream &output, const std::string &orgTab) const {
        SQLNode::Print(output, orgTab);
        output << "\n";
        const std::string tab = orgTab + "\t";
        output << tab;
        switch (type_) {
            case kInt:output << "value: " << val_.vint;
                break;
            case kBigInt:output << "value: " << val_.vlong;
                break;
            case kString:output << "value: " << val_.vstr;
                break;
            case kFloat:output << "value: " << val_.vfloat;
                break;
            case kDouble:output << "value: " << val_.vdouble;
                break;
            default:
                output << "value: unknow";
        }
    }

    int GetInt() {
        return val_.vint;
    }

    long GetLong() {
        return val_.vlong;
    }

    const char *GetStr() {
        return val_.vstr;
    }

    float GetFloat() {
        return val_.vfloat;
    }

    double GetDouble() {
        return val_.vdouble;
    }

private:
    union {
        int vint;        /* machine integer */
        long vlong;        /* machine integer */
        const char *vstr;        /* string */
        float vfloat;
        double vdouble;
    } val_;
};

class OtherSqlNode : public SQLNode {
public:
    OtherSqlNode(SQLNodeType &type) : SQLNode(type, 0, 0) {}
    OtherSqlNode(SQLNodeType &type, uint32_t line_num, uint32_t location) : SQLNode(type, line_num, location) {}
    void AddChild(SQLNode *node) {};
};

class UnknowSqlNode : public SQLNode {
public:
    UnknowSqlNode() : SQLNode(kUnknow, 0, 0) {}
    UnknowSqlNode(uint32_t line_num, uint32_t location) : SQLNode(kUnknow, line_num, location) {}

    void AddChild(SQLNode *node) {};
};

SQLNode *MakeSelectStmtNode(SQLNodeList *select_list_ptr_,
                            SQLNodeList *tableref_list_ptr,
                            SQLNodeList *window_clause_ptr,
                            SQLNode *limit_clause_ptr);
SQLNode *MakeTableNode(const std::string &name, const std::string &alias);
SQLNode *MakeFuncNode(const std::string &name, SQLNodeList *args, SQLNode *over);
SQLNode *MakeWindowDefNode(const std::string &name);
SQLNode *MakeWindowDefNode(SQLNodeList *partitions, SQLNodeList *orders, SQLNode *frame);
SQLNode *MakeOrderByNode(SQLNode *node_ptr);
SQLNode *MakeFrameNode(SQLNode *start, SQLNode *end);
SQLNode *MakeRangeFrameNode(SQLNode *node_ptr);
SQLNode *MakeRowsFrameNode(SQLNode *node_ptr);

SQLNode *MakeLimitNode(int count);
SQLNode *MakeConstNode(int value);
SQLNode *MakeConstNode(long value);
SQLNode *MakeConstNode(float value);
SQLNode *MakeConstNode(double value);
SQLNode *MakeConstNode(const std::string &value);
SQLNode *MakeColumnRefNode(const std::string &column_name, const std::string &relation_name);
SQLNode *MakeResTargetNode(SQLNode *node_ptr, const std::string &name);
SQLNode *MakeNode(const SQLNodeType &type, ...);
SQLNodeList *MakeNodeList(fesql::parser::SQLNode *node_ptr);

} // namespace of parser
} // namespace of fesql
#endif /* !FESQL_PARSER_NODE_H_ */
