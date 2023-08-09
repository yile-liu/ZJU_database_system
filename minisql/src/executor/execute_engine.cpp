#include "executor/execute_engine.h"
#include <sys/time.h>
#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include "glog/logging.h"
#include "index/b_plus_tree_index.h"

inline void PrintTable(const vector<string> &header, const vector<vector<string>> &i_col_j_row,
                       const vector<uint> &output_index = {}) {
  uint col_num = header.size();
  uint row_num;
  vector<uint> max_width;
  FILE *fp = fopen("result.txt", "w");
  if (output_index.empty()) {  // 全部输出
    row_num = i_col_j_row[0].size();
    for (uint i = 0; i < col_num; ++i) {
      max_width.emplace_back(header[i].length());
      for (uint j = 0; j < row_num; j++) {
        if (i_col_j_row[i][j].length() > max_width[i]) max_width[i] = i_col_j_row[i][j].length();
      }
    }
    for (uint i = 0; i < col_num; ++i) {
      fputc('+', fp);
      for (uint j = 0; j < max_width[i] + 2; j++) fputc('-', fp);
    }
    fprintf(fp, "+\n");
    for (uint i = 0; i < col_num; ++i) {
      fprintf(fp, "| %-*s ", max_width[i], header[i].c_str());
    }
    fprintf(fp, "|\n");
    for (uint i = 0; i < col_num; ++i) {
      fputc('+', fp);
      for (uint j = 0; j < max_width[i] + 2; j++) fputc('-', fp);
    }
    fprintf(fp, "+\n");
    for (uint j = 0; j < row_num; j++) {
      for (uint i = 0; i < col_num; i++) {
        fprintf(fp, "| %-*s ", max_width[i], i_col_j_row[i][j].c_str());
      }
      fprintf(fp, "|\n");
    }
    for (uint i = 0; i < col_num; ++i) {
      fputc('+', fp);
      for (uint j = 0; j < max_width[i] + 2; j++) fputc('-', fp);
    }
    fprintf(fp, "+\n");
    fclose(fp);
    uint start = 0, end = 30;
    max_width.clear();
    do {
      if (end > row_num) end = row_num;
      for (uint i = 0; i < col_num; ++i) {
        max_width.emplace_back(header[i].length());
        for (uint j = start; j < end; j++) {
          if (i_col_j_row[i][j].length() > max_width[i]) max_width[i] = i_col_j_row[i][j].length();
        }
      }
      for (uint i = 0; i < col_num; ++i) {
        putchar('+');
        for (uint j = 0; j < max_width[i] + 2; j++) putchar('-');
      }
      printf("+\n");
      for (uint i = 0; i < col_num; ++i) {
        printf("| %-*s ", max_width[i], header[i].c_str());
      }
      printf("|\n");
      for (uint i = 0; i < col_num; ++i) {
        putchar('+');
        for (uint j = 0; j < max_width[i] + 2; j++) putchar('-');
      }
      printf("+\n");
      for (uint j = start; j < end; j++) {
        for (uint i = 0; i < col_num; i++) {
          printf("| %-*s ", max_width[i], i_col_j_row[i][j].c_str());
        }
        printf("|\n");
      }
      for (uint i = 0; i < col_num; ++i) {
        putchar('+');
        for (uint j = 0; j < max_width[i] + 2; j++) putchar('-');
      }
      printf("+\n");
      if (end == row_num) break;
      start += 30;
      end += 30;
      cout << "(Press enter to continue or press 'q' to quit)\n:";
      if (getchar() == 'q') {
        getchar();
        break;
      }
    } while (true);
    cout << "Done. The complete result is in 'result.txt'" << endl;
  } else {
    row_num = output_index.size();
    for (uint i = 0; i < col_num; ++i) {
      max_width.emplace_back(header[i].length());
      for (uint j : output_index) {
        if (i_col_j_row[i][j].length() > max_width[i]) max_width[i] = i_col_j_row[i][j].length();
      }
    }
    for (uint i = 0; i < col_num; ++i) {
      fputc('+', fp);
      for (uint j = 0; j < max_width[i] + 2; j++) fputc('-', fp);
    }
    fprintf(fp, "+\n");
    for (uint i = 0; i < col_num; ++i) {
      fprintf(fp, "| %-*s ", max_width[i], header[i].c_str());
    }
    fprintf(fp, "|\n");
    for (uint i = 0; i < col_num; ++i) {
      fputc('+', fp);
      for (uint j = 0; j < max_width[i] + 2; j++) fputc('-', fp);
    }
    fprintf(fp, "+\n");
    for (uint j : output_index) {
      for (uint i = 0; i < col_num; i++) {
        fprintf(fp, "| %-*s ", max_width[i], i_col_j_row[i][j].c_str());
      }
      fprintf(fp, "|\n");
    }
    for (uint i = 0; i < col_num; ++i) {
      fputc('+', fp);
      for (uint j = 0; j < max_width[i] + 2; j++) fputc('-', fp);
    }
    fprintf(fp, "+\n");
    fclose(fp);
    uint start = 0, end = 30;
    max_width.clear();
    do {
      if (end >= output_index.size()) end = output_index.size();
      for (uint i = 0; i < col_num; ++i) {
        max_width.emplace_back(header[i].length());
        for (uint j = start; j < end; j++) {
          if (i_col_j_row[i][j].length() > max_width[i]) max_width[i] = i_col_j_row[i][j].length();
        }
      }
      for (uint i = 0; i < col_num; ++i) {
        putchar('+');
        for (uint j = 0; j < max_width[i] + 2; j++) putchar('-');
      }
      printf("+\n");
      for (uint i = 0; i < col_num; ++i) {
        printf("| %-*s ", max_width[i], header[i].c_str());
      }
      printf("|\n");
      for (uint i = 0; i < col_num; ++i) {
        putchar('+');
        for (uint j = 0; j < max_width[i] + 2; j++) putchar('-');
      }
      printf("+\n");
      for (uint j = start; j < end; j++) {
        for (uint i = 0; i < col_num; i++) {
          printf("| %-*s ", max_width[i], i_col_j_row[i][output_index[j]].c_str());
        }
        printf("|\n");
      }
      for (uint i = 0; i < col_num; ++i) {
        putchar('+');
        for (uint j = 0; j < max_width[i] + 2; j++) putchar('-');
      }
      printf("+\n");
      if (end == output_index.size()) break;
      start += 30;
      end += 30;
      cout << "(Press enter to continue or press 'q' to quit)\n:";
      if (getchar() == 'q') {
        getchar();
        break;
      }
    } while (true);
    cout << "Done. The complete result is in 'result.txt'" << endl;
  }
}
inline void PrintInfo(ExecuteContext *context) {
  struct timeval end {};
  gettimeofday(&end, nullptr);
  if (!context->dont_print) {
    if (context->print_flag == 0)
      if (end.tv_usec < context->start.tv_usec)
        cout << "Query OK, " << context->rows_num << " row affected." << endl;
      else
        cout << "Query OK, " << context->rows_num << " row affected." << endl;
    else {
      if (context->rows_num != 0) {
        if (end.tv_usec < context->start.tv_usec)
          cout << context->rows_num << " rows in set."<< endl;
        else
          cout << context->rows_num << " rows in set." << endl;
      } else {
        if (end.tv_usec < context->start.tv_usec)
          cout << "Empty set." << endl;
        else
          cout << "Empty set." << endl;
      }
    }
  }
}

ExecuteEngine::ExecuteEngine() {
  fstream list_file;
  list_file.open(LIST_FILE, std::ios::binary | std::ios::in);
  char DB_name[MAX_FILE_NAME_LENGTH], DB_path[MAX_FILE_NAME_LENGTH];
  if (list_file.is_open()) {
    list_file.getline(DB_name, MAX_FILE_NAME_LENGTH);
    while (strcmp(DB_name, "")) {
      strcpy(DB_path, DB_name);
      strcat(DB_path, ".db");
      auto DB_ptr = new DBStorageEngine(DB_path, false);
      dbs_.insert(make_pair(DB_name, DB_ptr));
      list_file.getline(DB_name, MAX_FILE_NAME_LENGTH);
    }
  }
  current_db_ = "";
}

dberr_t ExecuteEngine::Execute(pSyntaxNode ast, ExecuteContext *context) {
  if (ast == nullptr) {
    return DB_FAILED;
  }
  switch (ast->type_) {
    case kNodeCreateDB:
      return ExecuteCreateDatabase(ast, context);
    case kNodeDropDB:
      return ExecuteDropDatabase(ast, context);
    case kNodeShowDB:
      return ExecuteShowDatabases(ast, context);
    case kNodeUseDB:
      return ExecuteUseDatabase(ast, context);
    case kNodeShowTables:
      return ExecuteShowTables(ast, context);
    case kNodeCreateTable:
      return ExecuteCreateTable(ast, context);
    case kNodeDropTable:
      return ExecuteDropTable(ast, context);
    case kNodeShowIndexes:
      return ExecuteShowIndexes(ast, context);
    case kNodeCreateIndex:
      return ExecuteCreateIndex(ast, context);
    case kNodeDropIndex:
      return ExecuteDropIndex(ast, context);
    case kNodeSelect:
      return ExecuteSelect(ast, context);
    case kNodeInsert:
      return ExecuteInsert(ast, context);
    case kNodeDelete:
      return ExecuteDelete(ast, context);
    case kNodeUpdate:
      return ExecuteUpdate(ast, context);
    case kNodeTrxBegin:
      return ExecuteTrxBegin(ast, context);
    case kNodeTrxCommit:
      return ExecuteTrxCommit(ast, context);
    case kNodeTrxRollback:
      return ExecuteTrxRollback(ast, context);
    case kNodeExecFile:
      return ExecuteExecfile(ast, context);
    case kNodeQuit:
      return ExecuteQuit(ast, context);
    default:
      break;
  }
  return DB_FAILED;
}

dberr_t ExecuteEngine::ExecuteCreateDatabase(pSyntaxNode ast, ExecuteContext *context) {
#ifdef ENABLE_EXECUTE_DEBUG
  LOG(INFO) << "ExecuteCreateDatabase" << std::endl;
#endif
  char *DB_name = ast->child_->val_;
  char DB_path[MAX_FILE_NAME_LENGTH];
  if (dbs_.find(DB_name) != dbs_.end()) {
    cout << "Database named '" << DB_name << "' already exists." << endl;
    return DB_FAILED;
  } else {
    strcpy(DB_path, DB_name);
    strcat(DB_path, ".db");
    auto DB_ptr = new DBStorageEngine(DB_path, true);
    dbs_.insert(make_pair(DB_name, DB_ptr));
    cout << "Database created." << endl;
    return DB_SUCCESS;
  }
}

dberr_t ExecuteEngine::ExecuteDropDatabase(pSyntaxNode ast, ExecuteContext *context) {
#ifdef ENABLE_EXECUTE_DEBUG
  LOG(INFO) << "ExecuteDropDatabase" << std::endl;
#endif
  char *DB_name = ast->child_->val_;
  dbs_.erase(DB_name);
  cout << "Database Dropped." << endl;
  return DB_SUCCESS;
}

dberr_t ExecuteEngine::ExecuteShowDatabases(pSyntaxNode ast, ExecuteContext *context) {
#ifdef ENABLE_EXECUTE_DEBUG
  LOG(INFO) << "ExecuteShowDatabases" << std::endl;
#endif
  if (dbs_.empty()) {
    cout << "Empty!" << endl;
    return DB_FAILED;
  }
  context->print_flag = 1;
  if (!context->dont_print) gettimeofday(&context->start, nullptr);
  uint max_width = 8;
  for (const auto &itr : dbs_) {
    if (itr.first.length() > max_width) max_width = itr.first.length();
  }
  cout << "+" << setfill('-') << setw(max_width + 2) << ""
       << "+" << endl;
  cout << "| " << std::left << setfill(' ') << setw(max_width) << "Database"
       << " |" << endl;
  cout << "+" << setfill('-') << setw(max_width + 2) << ""
       << "+" << endl;
  for (const auto &itr : dbs_) {
    cout << "| " << std::left << setfill(' ') << setw(max_width) << itr.first << " |" << endl;
    context->rows_num++;
  }
  cout << "+" << setfill('-') << setw(max_width + 2) << ""
       << "+" << endl;
  PrintInfo(context);
  return DB_SUCCESS;
}

dberr_t ExecuteEngine::ExecuteUseDatabase(pSyntaxNode ast, ExecuteContext *context) {
#ifdef ENABLE_EXECUTE_DEBUG
  LOG(INFO) << "ExecuteUseDatabase" << std::endl;
#endif
  string DB_name = ast->child_->val_;
  if (dbs_.find(DB_name) != dbs_.end()) {
    current_db_ = DB_name;
    cout << "Database changed." << endl;
    return DB_SUCCESS;
  } else {
    cout << "Unknown database '" << DB_name << "'" << endl;
    return DB_FAILED;
  }
}

dberr_t ExecuteEngine::ExecuteShowTables(pSyntaxNode ast, ExecuteContext *context) {
#ifdef ENABLE_EXECUTE_DEBUG
  LOG(INFO) << "ExecuteShowTables" << std::endl;
#endif
  if (current_db_.empty()) {
    cout << "No database selected." << endl;
    return DB_FAILED;
  }
  context->print_flag = 1;
  if (!context->dont_print) gettimeofday(&context->start, NULL);
  vector<TableInfo *> tables;
  if (dbs_[current_db_]->catalog_mgr_->GetTables(tables) == DB_TABLE_NOT_EXIST) {
    cout << "Empty set." << endl;
    return DB_FAILED;
  }
  string title = "Tables_in_";
  title += current_db_;
  int max_len = title.length();
  for (auto table : tables) {
    if (table->GetTableName().length() > max_len) max_len = table->GetTableName().length();
  }
  cout << "+" << setfill('-') << setw(max_len + 2) << ""
       << "+" << endl;
  cout << "| " << setfill(' ') << setw(max_len) << title << " |" << endl;
  cout << "+" << setfill('-') << setw(max_len + 2) << ""
       << "+" << endl;

  for (auto table : tables) {
    cout << "| " << setfill(' ') << setw(max_len) << table->GetTableName() << " |" << endl;
    context->rows_num++;
  }

  cout << "+" << setfill('-') << setw(max_len + 2) << ""
       << "+" << endl;

  PrintInfo(context);
  return DB_SUCCESS;
}

dberr_t ExecuteEngine::ExecuteCreateTable(pSyntaxNode ast, ExecuteContext *context) {
#ifdef ENABLE_EXECUTE_DEBUG
  LOG(INFO) << "ExecuteCreateTable" << std::endl;
#endif
  // 是否打开了数据库
  if (current_db_.empty()) {
    cout << "No database selected" << endl;
    return DB_FAILED;
  }
  context->print_flag = 0;
  if (!context->dont_print) gettimeofday(&context->start, NULL);
  context->table_name_ = ast->child_->val_;

  // 遍历节点创建coloumn得到columns和index_keys
  vector<string> unique_column_name;
  auto *columns = new vector<Column *>;
  vector<string> index_keys;
  pSyntaxNode ast_ptr = ast->child_->next_->child_;
  int index = 0;
  try {
    while (ast_ptr != nullptr) {
      if (ast_ptr->type_ == kNodeColumnList) {
        string type = ast_ptr->val_;
        pSyntaxNode col_list_ptr = ast_ptr->child_;
        while (col_list_ptr != nullptr) {
          bool is_find = false;
          for (auto column : *columns) {  // 寻找索引映射关系
            if (column->GetName() == col_list_ptr->val_) {
              index_keys.emplace_back(col_list_ptr->val_);
              is_find = true;
              break;
            }
          }
          if (!is_find) {
            if (type == "primary keys" || type == "index keys") {
              string error_msg = "Key column '";
              error_msg += col_list_ptr->val_;
              error_msg += "' does not exist in table!";
              throw invalid_argument(error_msg);
            } else if (type == "select columns") {
              string error_msg = "Unknown column '";
              error_msg += col_list_ptr->val_;
              error_msg += "' in 'filed list'";
              throw invalid_argument(error_msg);
            }
          }
          col_list_ptr = col_list_ptr->next_;
        }
      } else if (ast_ptr->type_ == kNodeColumnDefinition) {
        bool is_unique = false;
        if (ast_ptr->val_ != nullptr && strcmp("unique", ast_ptr->val_) == 0) is_unique = true;
        string column_name = ast_ptr->child_->val_;
        string type = ast_ptr->child_->next_->val_;
        if (type == "int") {
          Column *column = new Column(column_name, kTypeInt, index++, false, is_unique);
          columns->emplace_back(column);
        }
        else if (type == "float") {
          Column *column = new Column(column_name, kTypeFloat, index++, false, is_unique);
          columns->emplace_back(column);
        }
        else if (type == "char") {
          string len_str = ast_ptr->child_->next_->child_->val_;
          int len = stoi(len_str);
          if (len > 4000 || len < 0 || len_str.find('.') != string::npos) throw invalid_argument("Invalid char length");
          if (len == 0 && is_unique)
            throw logic_error("The used storage engine can't index column '" + column_name + "'");
          Column *column = new Column(column_name, kTypeChar, len, index++, true, is_unique);
          columns->emplace_back(column);
        }
        if(is_unique){//将unique列的列名添加到vector，创建表成功后，为该表创建这些列的索引
          unique_column_name.push_back(ast_ptr->child_->val_);
        }
      }
      ast_ptr = ast_ptr->next_;
    }
  } catch (exception &error) {
    cout << error.what() << endl;
    for (auto column : *columns) delete column;
    return DB_FAILED;
  }
  // 创建table_info
  Schema *schema = new Schema(*columns);
  TableInfo *table_info;
  dberr_t result = dbs_[current_db_]->catalog_mgr_->CreateTable(context->table_name_, schema, nullptr, table_info);
  // 处理结果
  switch (result) {
    case DB_TABLE_ALREADY_EXIST:
      cout << "Table '" << context->table_name_ << "' already exists!" << endl;
      for (auto column : *columns) delete column;
      return DB_FAILED;
    case DB_SUCCESS:
      IndexInfo *index_info;
      if (!index_keys.empty()) {
        string index_name = context->table_name_ + ".PRIMARY";
        dbs_[current_db_]->catalog_mgr_->CreateIndex(context->table_name_, index_name, index_keys, nullptr, index_info);
      }

      if(!unique_column_name.empty()) {
        for(auto column_name:unique_column_name) {//为每个unique的列单独建索,索引名为table_name+column_name
          string index_name = context->table_name_ + "_" +column_name;
          IndexInfo *index_info;
          vector<string> index_key;
          index_key.push_back(column_name);
          dberr_t result = dbs_[current_db_]->catalog_mgr_->CreateIndex(context->table_name_,index_name,index_key,nullptr,index_info);
          if(result!=DB_SUCCESS){
            string error_msg = "Failed to create index on '" + context->table_name_ + "' for unique column '" + column_name + "'";
            throw invalid_argument(error_msg);
          }
        }//end for each
      }

      PrintInfo(context);
      return DB_SUCCESS;
    default:
      delete schema;
      for (auto column : *columns) delete column;
      return DB_FAILED;
  }
}

dberr_t ExecuteEngine::ExecuteDropTable(pSyntaxNode ast, ExecuteContext *context) {
#ifdef ENABLE_EXECUTE_DEBUG
  LOG(INFO) << "ExecuteDropTable" << std::endl;
#endif
  if (current_db_.empty()) {
    cout << "No database selected" << endl;
    return DB_FAILED;
  }
  context->print_flag = 0;
  if (!context->dont_print) gettimeofday(&context->start, NULL);
  string table_name = ast->child_->val_;
  if (ast->child_->val_ != nullptr) {
    dberr_t result = dbs_[current_db_]->catalog_mgr_->DropTable(table_name);
    if (result == DB_TABLE_NOT_EXIST) {
      cout << "Table '" << table_name << "' does not exist!" << endl;
      return DB_FAILED;
    } else if (result == DB_SUCCESS) {
      context->rows_num++;
      PrintInfo(context);
      return DB_SUCCESS;
    }
  }
  cout << "Unknown error" << endl;
  return DB_FAILED;
}

dberr_t ExecuteEngine::ExecuteShowIndexes(pSyntaxNode ast, ExecuteContext *context) {
#ifdef ENABLE_EXECUTE_DEBUG
  LOG(INFO) << "ExecuteShowIndexes" << std::endl;
#endif
  if (current_db_.empty()) {
    cout << "No database selected" << endl;
    return DB_FAILED;
  }
  context->print_flag = 1;
  if (!context->dont_print) gettimeofday(&context->start, NULL);
  vector<TableInfo *> tables;
  if (dbs_[current_db_]->catalog_mgr_->GetTables(tables) == DB_FAILED) {
    cout << "Empty set." << endl;
    return DB_FAILED;
  }
  vector<vector<IndexInfo *>> indexes_of_table_;
  vector<uint> index_num_of_table_;
  uint total_rows = 0;
  for (uint i = 0; i < tables.size(); i++) {
    indexes_of_table_.emplace_back(vector<IndexInfo *>());
    dbs_[current_db_]->catalog_mgr_->GetTableIndexes(tables[i]->GetTableName(), indexes_of_table_[i]);
    total_rows += indexes_of_table_[i].size();
    index_num_of_table_.emplace_back(indexes_of_table_[i].size());
  }
  if (total_rows == 0) {
    cout << "Empty set." << endl;
    return DB_FAILED;
  }
  vector<string> header{"table_name_", "index_name"};
  vector<vector<string>> output_info;
  output_info.emplace_back(vector<string>());
  output_info.emplace_back(vector<string>());
  for (uint i = 0; i < index_num_of_table_.size(); i++) {
    for (uint j = 0; j < index_num_of_table_[i]; j++) {
      output_info[0].emplace_back(tables[i]->GetTableName());
      output_info[1].emplace_back(indexes_of_table_[i][j]->GetIndexName());
    }
  }
  PrintTable(header, output_info);
  context->rows_num = total_rows;
  PrintInfo(context);
  return DB_SUCCESS;
}

dberr_t ExecuteEngine::ExecuteCreateIndex(pSyntaxNode ast, ExecuteContext *context) {
#ifdef ENABLE_EXECUTE_DEBUG
  LOG(INFO) << "ExecuteCreateIndex" << std::endl;
#endif
  if (current_db_.empty()) {
    cout << "No database selected" << endl;
    return DB_FAILED;
  }
  context->print_flag = 0;
  if (!context->dont_print) gettimeofday(&context->start, NULL);
  context->index_name = ast->child_->val_;
  context->table_name_ = ast->child_->next_->val_;
  TableInfo *table_info;
  if (dbs_[current_db_]->catalog_mgr_->GetTable(context->table_name_, table_info) == DB_TABLE_NOT_EXIST) {
    cout << "Table '" << context->table_name_ << "' doesn't exist" << endl;
    return DB_FAILED;
  }
  if (ExecuteInternalNode(ast->child_->next_->next_, context) == DB_COLUMN_NAME_NOT_EXIST) {
    cout << "No match column!" << endl;
  }
  bool is_unique{false};
  bool flag = false;
  string error_info("(");
  for (auto col : context->select_columns_) {
    if (flag)
      error_info += ", ";
    else
      flag = true;
    if (col->IsUnique()) is_unique = true;
    error_info += col->GetName();
  }
  error_info += ") is not unique, cannot create index on it";
  if (!is_unique) {
    cout << error_info << endl;
    return DB_FAILED;
  }
  IndexInfo *index_info;
  vector<string> index_keys;
  for (auto col : context->select_columns_) {
    index_keys.emplace_back(col->GetName());
  }
  cout << "!" << endl;
  dberr_t error_flag = dbs_[current_db_]->catalog_mgr_->CreateIndex(context->table_name_, context->index_name,
                                                                    index_keys, context->txn_, index_info);

  switch (error_flag) {
    case DB_TABLE_NOT_EXIST: {
      cout << "Table '" << context->table_name_ << "' doesn't exist" << endl;
      return DB_FAILED;
    }
    case DB_INDEX_ALREADY_EXIST: {
      cout << "Index '" << context->index_name << "' of table '" << context->table_name_ << "' already exists" << endl;
      return DB_FAILED;
    }
    case DB_SUCCESS: {
      context->rows_num++;
      PrintInfo(context);
      return DB_SUCCESS;
    }
    default:
      return DB_FAILED;
  }
}

dberr_t ExecuteEngine::ExecuteDropIndex(pSyntaxNode ast, ExecuteContext *context) {
#ifdef ENABLE_EXECUTE_DEBUG
  LOG(INFO) << "ExecuteDropIndex" << std::endl;
#endif
  if (current_db_.empty()) {
    cout << "No database selected." << endl;
    return DB_FAILED;
  }
  context->print_flag = 0;
  if (!context->dont_print) gettimeofday(&context->start, NULL);
  vector<TableInfo *> tables;
  if (dbs_[current_db_]->catalog_mgr_->GetTables(tables) == DB_FAILED) {
    return DB_FAILED;
  }
  string index_name(ast->child_->val_);
  for (auto table : tables) {
    if (dbs_[current_db_]->catalog_mgr_->DropIndex(table->GetTableName(), index_name) == DB_SUCCESS)
      context->rows_num++;
  }
  if (context->rows_num > 0) {
    PrintInfo(context);
    return DB_SUCCESS;
  } else {
    cout << "Index '" << context->index_name << "' doesn't exist" << endl;
    return DB_FAILED;
  }
}

dberr_t ExecuteEngine::ExecuteSelect(pSyntaxNode ast, ExecuteContext *context) {
#ifdef ENABLE_EXECUTE_DEBUG
  LOG(INFO) << "ExecuteSelect" << std::endl;
#endif
  context->table_name_ = ast->child_->next_->val_;
  context->print_flag = 1;
  // 打开当前使用的DB
  if (current_db_.empty()) {
    cout << "No database selected." << endl;
    return DB_FAILED;
  } else if (dbs_.find(current_db_) == dbs_.end()) {
    cout << "Internal error." << endl;
    return DB_FAILED;
  }
  DBStorageEngine *DB_storage_engine = dbs_.find(current_db_)->second;
  TableInfo *table_info;
  dberr_t res;
  if (DB_storage_engine->catalog_mgr_->GetTable(context->table_name_, table_info) == DB_SUCCESS) {
    // 能够打开对应的表
    // 确定select column
    res = ExecuteInternalNode(ast->child_, context);
    if (res == DB_SUCCESS) {
      vector<string> header;
      vector<vector<string>> output_info;
      if (!ast->child_->next_->next_) {
        // 如果没有条件 直接输出整张表
        for (auto col_item : context->select_columns_) {
          header.emplace_back(col_item->GetName());
          uint32_t col_id;
          table_info->GetSchema()->GetColumnIndex(col_item->GetName(), col_id);
          output_info.emplace_back(vector<string>());

          for (auto iter = table_info->GetTableHeap()->Begin(context->txn_); iter != table_info->GetTableHeap()->End();
               iter++) {
            output_info.back().emplace_back((*iter).GetField(col_id)->GetData());
            context->rows_num++;
          }
        }
        context->rows_num/=context->select_columns_.size();
        if (!output_info.back().empty())
          PrintTable(header, output_info);
        else
          cout << "Empty set." << endl;
        PrintInfo(context);
        return DB_SUCCESS;
      } else {
        // 开始根据Condition进行筛选
        res = ExecuteInternalNode(ast->child_->next_->next_->child_, context);
        if (res == DB_SUCCESS) {
          for (auto col_item : context->select_columns_) {
            header.emplace_back(col_item->GetName());
            uint32_t col_id;
            table_info->GetSchema()->GetColumnIndex(col_item->GetName(), col_id);
            output_info.emplace_back(vector<string>());
            for (auto row_item : context->internal_result_rid_) {
              Row row(row_item);
              table_info->GetTableHeap()->GetTuple(&row, context->txn_);
              output_info.back().emplace_back(row.GetField(col_id)->GetData());
              context->rows_num++;
            }
          }
          context->rows_num/=context->select_columns_.size();
          if (!output_info.back().empty())
            PrintTable(header, output_info);
          else
            cout << "Empty set." << endl;
          PrintInfo(context);
          return DB_SUCCESS;
        } else {
          return res;
        }
      }
    } else {
      return res;
    }
  }
  return DB_FAILED;
}

dberr_t ExecuteEngine::ExecuteInsert(pSyntaxNode ast, ExecuteContext *context) {
#ifdef ENABLE_EXECUTE_DEBUG
  LOG(INFO) << "ExecuteInsert" << std::endl;
#endif
  context->table_name_ = ast->child_->val_;
  if (current_db_.empty()) {
    cout << "No database selected." << endl;
    return DB_FAILED;
  }
  context->print_flag = 0;
  if (!context->dont_print) gettimeofday(&context->start, nullptr);
  string table_name = ast->child_->val_;
  TableInfo *table_info;
  vector<IndexInfo *> indexes;
  if (dbs_[current_db_]->catalog_mgr_->GetTable(table_name, table_info) == DB_TABLE_NOT_EXIST) {
    cout << "Table '" << context->table_name_ << "' doesn't exist." << endl;
    return DB_FAILED;
  }
  dbs_[current_db_]->catalog_mgr_->GetTableIndexes(table_name, indexes);
  ast = ast->child_->next_->child_;
  vector<Field> fields;
  vector<Column *> columns = table_info->GetSchema()->GetColumns();
  for (auto col : columns) {
    if (ast == nullptr) {
      cout << " Column count doesn't match value count." << endl;
      return DB_FAILED;
    } else if (ast->type_ == kNodeNull) {
      if (col->IsNullable() && !col->IsUnique()) {
        fields.emplace_back(Field(col->GetType()));
      } else {
        cout << "Field '" << col->GetName() << "' can't be null." << endl;
        return DB_FAILED;
      }
    } else {
      string value = ast->val_;
      if (col->GetType() == kTypeInt && ast->type_ == kNodeNumber) {  // 类型检查
        int val_int = atoi(value.c_str());
        fields.emplace_back(Field(col->GetType(), val_int));
      } else if (col->GetType() == kTypeFloat && ast->type_ == kNodeNumber) {
        float val_float = atof(value.c_str());
        fields.emplace_back(Field(col->GetType(), val_float));
      } else if (col->GetType() == kTypeChar && ast->type_ == kNodeString) {
        if (value.length() > col->GetLength()) {
          cout << "Data too long for column '" << col->GetName() << "'" << endl;
          return DB_FAILED;
        } else {
          char *val_char = new char[value.length() + 1];
          strcpy(val_char, value.c_str());
          fields.emplace_back(Field(col->GetType(), val_char, value.length(), true));
          delete[] val_char;
        }
      } else {
        cout << "Wrong data type for column '" << col->GetName() << "'" << endl;
        return DB_FAILED;
      }
    }
    ast = ast->next_;
  }
  if (ast != nullptr) {
    cout << " Column count doesn't match value count." << endl;
    return DB_FAILED;
  }
  Row row(fields);
  vector<vector<Field>> indexes_fields;
  for (auto index : indexes) {
    vector<Field> index_fields;
    for (auto col : index->GetIndexKeySchema()->GetColumns()) {
      index_fields.emplace_back(*row.GetFields()[col->GetTableInd()]);
    }
    indexes_fields.emplace_back(index_fields);
    vector<RowId> result;
    if (index->GetIndex()->ScanKey(Row(index_fields), result, nullptr) == DB_SUCCESS) {
      string error_info = "Duplicate entry '";
      bool temp_flag = false;
      for (auto &filed : index_fields) {
        if (temp_flag)
          error_info += "-";
        else
          temp_flag = true;
      }
      error_info += "' for key '";
      error_info += index->GetIndexName();
      error_info += "'";
      cout << error_info << endl;
      return DB_FAILED;
    }
  }
  assert(dbs_[current_db_]->bpm_->CheckAllUnpinned());
  if (table_info->GetTableHeap()->InsertTuple(row, nullptr)) {
    assert(dbs_[current_db_]->bpm_->CheckAllUnpinned());
    for (uint i = 0; i < indexes.size(); i++) {
      if (indexes[i]->GetIndex()->InsertEntry(Row(indexes_fields[i]), row.GetRowId(), nullptr) == DB_SUCCESS) {
      } else {
        cout << "Unknown error." << endl;
        return DB_FAILED;
      }
    }
    context->rows_num++;
    PrintInfo(context);
    return DB_SUCCESS;
  } else {
    cout << "Unknown error." << endl;
    return DB_FAILED;
  }
  return DB_FAILED;
}

dberr_t ExecuteEngine::ExecuteDelete(pSyntaxNode ast, ExecuteContext *context) {
#ifdef ENABLE_EXECUTE_DEBUG
  LOG(INFO) << "ExecuteDelete" << std::endl;
#endif

  if (current_db_.empty()) {
    cout << "No database selected" << endl;
    return DB_FAILED;
  }
  context->print_flag = 0;
  context->table_name_ = ast->child_->val_;
  // 打开当前使用的DB
  DBStorageEngine *DB_storage_engine = dbs_.find(current_db_)->second;
  TableInfo *table_info;
  dberr_t res;
  if (DB_storage_engine->catalog_mgr_->GetTable(context->table_name_, table_info) == DB_TABLE_NOT_EXIST) {
    cout << "Table '" << context->table_name_ << "' doesn't exist." << endl;
    return DB_FAILED;
  }
  vector<IndexInfo *> indexes;
  DB_storage_engine->catalog_mgr_->GetTableIndexes(context->table_name_, indexes);
  if (ast->child_->next_ != nullptr) {
    //    有condition
    res = ExecuteInternalNode(ast->child_->next_->child_, context);
    if (res == DB_SUCCESS) {
      set<RowId> result;
      result = context->internal_result_rid_;
      for (auto rid : result) {
        Row row(rid);
        table_info->GetTableHeap()->GetTuple(&row, context->txn_);
        for (auto index : indexes) {
          vector<Field> key_fields;
          for (auto col : index->GetIndexKeySchema()->GetColumns()) {
            key_fields.emplace_back(*row.GetField(col->GetTableInd()));
          }
          index->GetIndex()->RemoveEntry(Row(key_fields), INVALID_ROWID, context->txn_);
        }
        table_info->GetTableHeap()->ApplyDelete(row.GetRowId(), context->txn_);
        context->rows_num++;
      }
    }
    PrintInfo(context);
    return res;
  } else {
    //    无condition
    for (auto iter = table_info->GetTableHeap()->Begin(context->txn_); iter != table_info->GetTableHeap()->End();
         iter++) {
      Row row(iter->GetRowId());
      table_info->GetTableHeap()->GetTuple(&row, context->txn_);
      for (auto index : indexes) {
        vector<Field> key_fields;
        for (auto col : index->GetIndexKeySchema()->GetColumns()) {
          key_fields.emplace_back(*row.GetField(col->GetTableInd()));
        }
        index->GetIndex()->RemoveEntry(Row(key_fields), INVALID_ROWID, context->txn_);
      }
      table_info->GetTableHeap()->ApplyDelete(row.GetRowId(), context->txn_);
      context->rows_num++;
    }
  }
  PrintInfo(context);
  return DB_SUCCESS;
}

dberr_t ExecuteEngine::ExecuteUpdate(pSyntaxNode ast, ExecuteContext *context) {
#ifdef ENABLE_EXECUTE_DEBUG
  LOG(INFO) << "ExecuteUpdate" << std::endl;
#endif
  if (current_db_.empty()) {
    cout << "No database selected" << endl;
    return DB_FAILED;
  }
  context->print_flag = 0;
  context->table_name_ = ast->child_->val_;
  // 打开当前使用的DB
  DBStorageEngine *DB_storage_engine = dbs_.find(current_db_)->second;
  TableInfo *table_info;
  dberr_t res;
  if (DB_storage_engine->catalog_mgr_->GetTable(context->table_name_, table_info) == DB_TABLE_NOT_EXIST) {
    cout << "Table '" << context->table_name_ << "' doesn't exist" << endl;
    return DB_FAILED;
  }
  vector<IndexInfo *> indexes;
  DB_storage_engine->catalog_mgr_->GetTableIndexes(context->table_name_, indexes);
  if (ast->child_->next_->next_ != nullptr) {
    //    有condition
    res = ExecuteInternalNode(ast->child_->next_->next_->child_, context);
    if (res == DB_SUCCESS) {
      ast = ast->child_->next_->child_;
      while (ast != nullptr) {
        for (auto rid : context->internal_result_rid_) {
          Row old_row(rid);
          table_info->GetTableHeap()->GetTuple(&old_row, context->txn_);

          vector<Field> new_row_fields;
          for (uint i = 0; i < old_row.GetFieldCount(); ++i) {
            new_row_fields.emplace_back(*old_row.GetField(i));
          }
          uint32_t new_index;
          table_info->GetSchema()->GetColumnIndex(ast->child_->val_, new_index);
          if ((table_info->GetSchema()->GetColumn(new_index)->GetType() == kTypeInt or
               table_info->GetSchema()->GetColumn(new_index)->GetType() == kTypeFloat) and
              ast->child_->next_->type_ == kNodeNumber) {
            if (table_info->GetSchema()->GetColumn(new_index)->GetType() == kTypeInt) {
              Field new_field(kTypeInt, atoi(ast->child_->next_->val_));
              new_row_fields[new_index] = new_field;
            } else if (table_info->GetSchema()->GetColumn(new_index)->GetType() == kTypeFloat) {
              Field new_field(kTypeFloat, (float)atof(ast->child_->next_->val_));
              new_row_fields[new_index] = new_field;
            }
          } else if (table_info->GetSchema()->GetColumn(new_index)->GetType() == kTypeChar and
                     ast->child_->next_->type_ == kNodeString) {
            Field new_field(kTypeChar, ast->child_->next_->val_, strlen(ast->child_->next_->val_), true);
            new_row_fields[new_index] = new_field;
          }
          // 错误类型
          else
            return DB_FAILED;
          vector<vector<Field>> indexes_fields;
          Row new_row(new_row_fields);
          for (auto index : indexes) {
            vector<Field> index_fields;
            if (index->GetIndexKeySchema()->GetColumn(0)->GetName()==table_info->GetSchema()->GetColumn(new_index)->GetName()){
              for (auto col : index->GetIndexKeySchema()->GetColumns()) {
                index_fields.emplace_back(*new_row.GetFields()[col->GetTableInd()]);
              }
              indexes_fields.emplace_back(index_fields);
              vector<RowId> result;
              if (index->GetIndex()->ScanKey(Row(index_fields), result, nullptr) == DB_SUCCESS) {
                string error_info = "Duplicate entry";
                bool temp_flag = false;
                for (auto &filed : index_fields) {
                  if (temp_flag)
                    error_info += "-";
                  else
                    temp_flag = true;
                }
                error_info += " for key '";
                error_info += index->GetIndexName();
                error_info += "'";
                cout << error_info << endl;
                return DB_FAILED;
              }
            }
          }
          for (auto index : indexes) {
            vector<Field> old_key_fields;
            vector<Field> new_key_fields;
            for (auto col : index->GetIndexKeySchema()->GetColumns()) {
              old_key_fields.emplace_back(*old_row.GetField(col->GetTableInd()));
              new_key_fields.emplace_back(new_row_fields[col->GetTableInd()]);
            }
            index->GetIndex()->RemoveEntry(Row(old_key_fields), INVALID_ROWID, context->txn_);
            index->GetIndex()->InsertEntry(Row(new_key_fields), old_row.GetRowId(), context->txn_);
          }

          table_info->GetTableHeap()->UpdateTuple(new_row, old_row.GetRowId(), context->txn_);
          context->rows_num++;
        }
        ast = ast->next_;
      }
    }
    PrintInfo(context);
    return res;
  } else {
    //    无condition
    ast = ast->child_->next_->child_;
    while (ast != nullptr) {
      for (auto iter = table_info->GetTableHeap()->Begin(context->txn_); iter != table_info->GetTableHeap()->End();
           iter++) {
        Row old_row(iter->GetRowId());
        table_info->GetTableHeap()->GetTuple(&old_row, context->txn_);

        vector<Field> new_row_fields;
        for (uint i = 0; i < old_row.GetFieldCount(); ++i) {
          new_row_fields.emplace_back(*old_row.GetField(i));
        }
        uint32_t new_index;
        table_info->GetSchema()->GetColumnIndex(ast->child_->val_, new_index);
        if ((table_info->GetSchema()->GetColumn(new_index)->GetType() == kTypeInt or
             table_info->GetSchema()->GetColumn(new_index)->GetType() == kTypeFloat) and
            ast->child_->next_->type_ == kNodeNumber) {
          if (table_info->GetSchema()->GetColumn(new_index)->GetType() == kTypeInt) {
            Field new_field(kTypeInt, atoi(ast->child_->next_->val_));
            new_row_fields[new_index] = new_field;
          } else if (table_info->GetSchema()->GetColumn(new_index)->GetType() == kTypeFloat) {
            Field new_field(kTypeFloat, (float)atof(ast->child_->next_->val_));
            new_row_fields[new_index] = new_field;
          }
        } else if (table_info->GetSchema()->GetColumn(new_index)->GetType() == kTypeChar and
                   ast->child_->next_->type_ == kNodeString) {
          Field new_field(kTypeChar, ast->child_->next_->val_, strlen(ast->child_->next_->val_), true);
          new_row_fields[new_index] = new_field;
        }
        //        错误类型
        else
          return DB_FAILED;
        vector<vector<Field>> indexes_fields;
        Row new_row(new_row_fields);
        for (auto index : indexes) {
          vector<Field> index_fields;
          if (index->GetIndexKeySchema()->GetColumn(0)->GetName()==table_info->GetSchema()->GetColumn(new_index)->GetName()){
            for (auto col : index->GetIndexKeySchema()->GetColumns()) {
              index_fields.emplace_back(*new_row.GetFields()[col->GetTableInd()]);
            }
            indexes_fields.emplace_back(index_fields);
            vector<RowId> result;
            if (index->GetIndex()->ScanKey(Row(index_fields), result, nullptr) == DB_SUCCESS) {
              string error_info = "Duplicate entry";
              bool temp_flag = false;
              for (auto &filed : index_fields) {
                if (temp_flag)
                  error_info += "-";
                else
                  temp_flag = true;
              }
              error_info += " for key '";
              error_info += index->GetIndexName();
              error_info += "'";
              cout << error_info << endl;
              return DB_FAILED;
            }
          }
        }
        for (auto index : indexes) {
          vector<Field> old_key_fields;
          vector<Field> new_key_fields;
          for (auto col : index->GetIndexKeySchema()->GetColumns()) {
            old_key_fields.emplace_back(*old_row.GetField(col->GetTableInd()));
            new_key_fields.emplace_back(new_row_fields[col->GetTableInd()]);
          }
          index->GetIndex()->RemoveEntry(Row(old_key_fields), INVALID_ROWID, context->txn_);
          index->GetIndex()->InsertEntry(Row(new_key_fields), old_row.GetRowId(), context->txn_);
        }

        table_info->GetTableHeap()->UpdateTuple(new_row, old_row.GetRowId(), context->txn_);
        context->rows_num++;
      }
      ast = ast->next_;
    }
  }
  PrintInfo(context);
  return DB_SUCCESS;
}

dberr_t ExecuteEngine::ExecuteTrxBegin(pSyntaxNode ast, ExecuteContext *context) {
#ifdef ENABLE_EXECUTE_DEBUG
  LOG(INFO) << "ExecuteTrxBegin" << std::endl;
#endif
  return DB_FAILED;
}

dberr_t ExecuteEngine::ExecuteTrxCommit(pSyntaxNode ast, ExecuteContext *context) {
#ifdef ENABLE_EXECUTE_DEBUG
  LOG(INFO) << "ExecuteTrxCommit" << std::endl;
#endif
  return DB_FAILED;
}

dberr_t ExecuteEngine::ExecuteTrxRollback(pSyntaxNode ast, ExecuteContext *context) {
#ifdef ENABLE_EXECUTE_DEBUG
  LOG(INFO) << "ExecuteTrxRollback" << std::endl;
#endif
  return DB_FAILED;
}

dberr_t ExecuteEngine::ExecuteExecfile(pSyntaxNode ast, ExecuteContext *context) {
#ifdef ENABLE_EXECUTE_DEBUG
  LOG(INFO) << "ExecuteExecfile" << std::endl;
#endif
  return DB_FAILED;
}

dberr_t ExecuteEngine::ExecuteQuit(pSyntaxNode ast, ExecuteContext *context) {
#ifdef ENABLE_EXECUTE_DEBUG
  LOG(INFO) << "ExecuteQuit" << std::endl;
#endif
  ASSERT(ast->type_ == kNodeQuit, "Unexpected node type.");
  context->flag_quit_ = true;
  return DB_SUCCESS;
}

dberr_t ExecuteEngine::ExecuteInternalNode(pSyntaxNode ast, ExecuteContext *context) {
  switch (ast->type_) {
    case kNodeColumnDefinition:
      break;
    case kNodeConnector:
      return ExecuteConnector(ast, context);
    case kNodeCompareOperator:
      return ExecuteCompareOperator(ast, context);
    case kNodeAllColumns:
      return ExecuteAllColumn(ast, context);
    case kNodeColumnList:
      return ExecuteColumnList(ast, context);
    default:
      cerr << endl << "ExecuteInternalNode: Invalid internal node type!" << endl;
      return DB_FAILED;
  }
}
dberr_t ExecuteEngine::ExecuteCompareOperator(pSyntaxNode ast, ExecuteContext *context) {
  // 清空临时表
  (*context).internal_result_rid_.clear();
  // 获取相关信息
  string operation = ast->val_;
  string attribute_name = ast->child_->val_;
  string value = ast->child_->next_->val_;
  uint32_t field_index;
  // 打开当前使用的DB
  DBStorageEngine *DB_storage_engine = dbs_.find(current_db_)->second;
  TableInfo *table_info;
  if (DB_storage_engine->catalog_mgr_->GetTable(context->table_name_, table_info) == DB_SUCCESS) {
    // 打开对应的表
    Schema *schema = table_info->GetSchema();
    if (schema->GetColumnIndex(attribute_name, field_index) == DB_COLUMN_NAME_NOT_EXIST) {
      return DB_COLUMN_NAME_NOT_EXIST;
    }
    const Column *column = schema->GetColumn(field_index);
    vector<Column *> columns;
    columns.emplace_back(const_cast<Column *>(column));
    TypeId type_id = column->GetType();

    // 新建field用于table_heap遍历比较
    Field field_int(kTypeInt, atoi(value.c_str()));
    Field field_float(kTypeFloat, (float)atof(value.c_str()));
    Field field_char(kTypeChar, (char *)value.c_str(), strlen(value.c_str()), false);

    // 检查是否存在可用的index
    vector<IndexInfo *> indexes;
    IndexInfo *index_result;
    bool find_index = false;
    DB_storage_engine->catalog_mgr_->GetTableIndexes(context->table_name_, indexes);
    for (auto index_item : indexes) {
      if (index_item->GetIndexKeySchema()->GetColumns() == columns) {
        index_result = index_item;
        find_index = true;
        break;
      }
    }

    // 生成index的key用于比较
    IndexSchema *index_schema;
    vector<RowId> index_rid;
    vector<Field> index_fields;
    GenericKey<64> index_key;
    if (find_index) {
      switch (type_id) {
        case kTypeInt:
          index_fields.emplace_back(field_int);
          break;
        case kTypeFloat:
          index_fields.emplace_back(field_float);
          break;
        case kTypeChar:
          index_fields.emplace_back(field_char);
          break;
      }
      index_schema = index_result->GetIndexKeySchema();
      Row index_row(index_fields);
      index_key.SerializeFromKey(index_row, index_schema);
    }

    // 没有找到index时用table_heap遍历进行比较
    if (!find_index) {
      auto iter = table_info->GetTableHeap()->Begin(context->txn_);
      if (operation == "=") {
        switch (type_id) {
          case kTypeInt:
            for (; iter != table_info->GetTableHeap()->End(); iter++)
              if ((*iter).GetField(field_index)->CompareEquals(field_int))
                context->internal_result_rid_.insert((*iter).GetRowId());
            break;
          case kTypeFloat:
            for (; iter != table_info->GetTableHeap()->End(); iter++)
              if ((*iter).GetField(field_index)->CompareEquals(field_float))
                context->internal_result_rid_.insert((*iter).GetRowId());
            break;
          case kTypeChar:
            for (; iter != table_info->GetTableHeap()->End(); iter++)
              if ((*iter).GetField(field_index)->CompareEquals(field_char))
                context->internal_result_rid_.insert((*iter).GetRowId());
            break;
          case kTypeInvalid:
            cerr << "ExecuteCompareOperator: Cannot operate a invalid type field!" << endl;
            return DB_FAILED;
        }
      } else if (operation == "<") {
        switch (type_id) {
          case kTypeInt:
            for (; iter != table_info->GetTableHeap()->End(); iter++)
              if ((*iter).GetField(field_index)->CompareLessThan(field_int))
                context->internal_result_rid_.insert((*iter).GetRowId());
            break;
          case kTypeFloat:
            for (; iter != table_info->GetTableHeap()->End(); iter++)
              if ((*iter).GetField(field_index)->CompareLessThan(field_float))
                context->internal_result_rid_.insert((*iter).GetRowId());
            break;
          case kTypeChar:
            for (; iter != table_info->GetTableHeap()->End(); iter++)
              if ((*iter).GetField(field_index)->CompareLessThan(field_char))
                context->internal_result_rid_.insert((*iter).GetRowId());
            break;
          case kTypeInvalid:
            cerr << "ExecuteCompareOperator: Cannot operate a invalid type field!" << endl;
            return DB_FAILED;
        }
      } else if (operation == "<=") {
        switch (type_id) {
          case kTypeInt:
            for (; iter != table_info->GetTableHeap()->End(); iter++)
              if ((*iter).GetField(field_index)->CompareLessThanEquals(field_int))
                context->internal_result_rid_.insert((*iter).GetRowId());
            break;
          case kTypeFloat:
            for (; iter != table_info->GetTableHeap()->End(); iter++)
              if ((*iter).GetField(field_index)->CompareLessThanEquals(field_float))
                context->internal_result_rid_.insert((*iter).GetRowId());
            break;
          case kTypeChar:
            for (; iter != table_info->GetTableHeap()->End(); iter++)
              if ((*iter).GetField(field_index)->CompareLessThanEquals(field_char))
                context->internal_result_rid_.insert((*iter).GetRowId());
            break;
          case kTypeInvalid:
            cerr << "ExecuteCompareOperator: Cannot operate a invalid type field!" << endl;
            return DB_FAILED;
        }
      } else if (operation == ">") {
        switch (type_id) {
          case kTypeInt:
            for (; iter != table_info->GetTableHeap()->End(); iter++)
              if ((*iter).GetField(field_index)->CompareGreaterThan(field_int))
                context->internal_result_rid_.insert((*iter).GetRowId());
            break;
          case kTypeFloat:
            for (; iter != table_info->GetTableHeap()->End(); iter++)
              if ((*iter).GetField(field_index)->CompareGreaterThan(field_float))
                context->internal_result_rid_.insert((*iter).GetRowId());
            break;
          case kTypeChar:
            for (; iter != table_info->GetTableHeap()->End(); iter++)
              if ((*iter).GetField(field_index)->CompareGreaterThan(field_char))
                context->internal_result_rid_.insert((*iter).GetRowId());
            break;
          case kTypeInvalid:
            cerr << "ExecuteCompareOperator: Cannot operate a invalid type field!" << endl;
            return DB_FAILED;
        }
      } else if (operation == ">=") {
        switch (type_id) {
          case kTypeInt:
            for (; iter != table_info->GetTableHeap()->End(); iter++)
              if ((*iter).GetField(field_index)->CompareGreaterThanEquals(field_int))
                context->internal_result_rid_.insert((*iter).GetRowId());
            break;
          case kTypeFloat:
            for (; iter != table_info->GetTableHeap()->End(); iter++)
              if ((*iter).GetField(field_index)->CompareGreaterThanEquals(field_float))
                context->internal_result_rid_.insert((*iter).GetRowId());
            break;
          case kTypeChar:
            for (; iter != table_info->GetTableHeap()->End(); iter++)
              if ((*iter).GetField(field_index)->CompareGreaterThanEquals(field_char))
                context->internal_result_rid_.insert((*iter).GetRowId());
            break;
          case kTypeInvalid:
            cerr << "ExecuteCompareOperator: Cannot operate a invalid type field!" << endl;
            return DB_FAILED;
        }
      } else if (operation == "<>") {
        switch (type_id) {
          case kTypeInt:
            for (; iter != table_info->GetTableHeap()->End(); iter++)
              if (!(*iter).GetField(field_index)->CompareEquals(field_int))
                context->internal_result_rid_.insert((*iter).GetRowId());
            break;
          case kTypeFloat:
            for (; iter != table_info->GetTableHeap()->End(); iter++)
              if (!(*iter).GetField(field_index)->CompareEquals(field_float))
                context->internal_result_rid_.insert((*iter).GetRowId());
            break;
          case kTypeChar:
            for (; iter != table_info->GetTableHeap()->End(); iter++)
              if (!(*iter).GetField(field_index)->CompareEquals(field_char))
                context->internal_result_rid_.insert((*iter).GetRowId());
            break;
          case kTypeInvalid:
            cerr << "ExecuteCompareOperator: Cannot operate a invalid type field!" << endl;
            return DB_FAILED;
        }
      } else if (operation == "is") {
        if (ast->type_ == kNodeNull) {
          for (; iter != table_info->GetTableHeap()->End(); iter++)
            if ((*iter).GetField(field_index)->IsNull()) context->internal_result_rid_.insert((*iter).GetRowId());
        } else {
          cerr << "ExecuteCompareOperator: Operator \"is\" cannot followed by value!" << endl;
          return DB_FAILED;
        }
      } else if (operation == "not") {
        if (ast->type_ == kNodeNull) {
          for (; iter != table_info->GetTableHeap()->End(); iter++)
            if (!(*iter).GetField(field_index)->IsNull()) context->internal_result_rid_.insert((*iter).GetRowId());
        } else {
          cerr << "ExecuteCompareOperator: Operator \"not\" cannot followed by value!" << endl;
          return DB_FAILED;
        }
      }
    }
    // 用index进行查找比较
    else {
      if (operation == "=") {
        if (index_result->GetIndex()->GetContainer().GetValue(index_key, index_rid, context->txn_)) {
          context->internal_result_rid_.insert(index_rid[0]);
        }
      } else if (operation == "<>") {
        bool has_equal = false;
        vector<RowId> equal_rid;
        if (index_result->GetIndex()->GetContainer().GetValue(index_key, equal_rid, context->txn_)) {
          has_equal = true;
        }

        if (has_equal) {
          for (auto iter = index_result->GetIndex()->GetContainer().Begin();
               iter != index_result->GetIndex()->GetContainer().End(); ++iter) {
            if (!((*iter).second == equal_rid[0])) {
              context->internal_result_rid_.insert((*iter).second);
            }
          }
        } else {
          for (auto iter = index_result->GetIndex()->GetContainer().Begin();
               iter != index_result->GetIndex()->GetContainer().End(); ++iter) {
            context->internal_result_rid_.insert((*iter).second);
          }
        }
      } else if (operation == ">") {
        Row target_row(INVALID_ROWID);
        index_key.DeserializeToKey(target_row, index_schema);
        Row row(INVALID_ROWID);
        const Field *comp_field = target_row.GetField(0);
        for (auto iter = index_result->GetIndex()->GetContainer().Begin();
             iter != index_result->GetIndex()->GetContainer().End(); ++iter) {
          (*iter).first.DeserializeToKey(row, index_schema);
          if (row.GetField(0)->CompareGreaterThan(*comp_field)) {
            context->internal_result_rid_.insert((*iter).second);
          }
        }
      } else if (operation == "<") {
        Row target_row(INVALID_ROWID);
        index_key.DeserializeToKey(target_row, index_schema);
        Row row(INVALID_ROWID);
        const Field *comp_field = target_row.GetField(0);
        for (auto iter = index_result->GetIndex()->GetContainer().Begin();
             iter != index_result->GetIndex()->GetContainer().End(); ++iter) {
          (*iter).first.DeserializeToKey(row, index_schema);
          if (row.GetField(0)->CompareLessThan(*comp_field)) {
            context->internal_result_rid_.insert((*iter).second);
          }
        }
      } else if (operation == "<=") {
        Row target_row(INVALID_ROWID);
        index_key.DeserializeToKey(target_row, index_schema);
        Row row(INVALID_ROWID);
        const Field *comp_field = target_row.GetField(0);
        for (auto iter = index_result->GetIndex()->GetContainer().Begin();
             iter != index_result->GetIndex()->GetContainer().End(); ++iter) {
          (*iter).first.DeserializeToKey(row, index_schema);
          if (row.GetField(0)->CompareLessThanEquals(*comp_field)) {
            context->internal_result_rid_.insert((*iter).second);
          }
        }
      } else if (operation == ">=") {
        Row target_row(INVALID_ROWID);
        index_key.DeserializeToKey(target_row, index_schema);
        Row row(INVALID_ROWID);
        const Field *comp_field = target_row.GetField(0);
        for (auto iter = index_result->GetIndex()->GetContainer().Begin();
             iter != index_result->GetIndex()->GetContainer().End(); ++iter) {
          (*iter).first.DeserializeToKey(row, index_schema);
          if (row.GetField(0)->CompareGreaterThanEquals(*comp_field)) {
            context->internal_result_rid_.insert((*iter).second);
          }
        }
      } else {
        return DB_FAILED;
      }
    }
  } else {
    return DB_TABLE_NOT_EXIST;
  }
  return DB_SUCCESS;
}

dberr_t ExecuteEngine::ExecuteAllColumn(pSyntaxNode ast, ExecuteContext *context) {
  context->select_columns_.clear();
  DBStorageEngine *DB_storage_engine = dbs_.find(current_db_)->second;
  TableInfo *table_info;
  DB_storage_engine->catalog_mgr_->GetTable(context->table_name_, table_info);
  context->select_columns_ = table_info->GetSchema()->GetColumns();
  return DB_SUCCESS;
}

dberr_t ExecuteEngine::ExecuteColumnList(pSyntaxNode ast, ExecuteContext *context) {
  context->select_columns_.clear();
  DBStorageEngine *DB_storage_engine = dbs_.find(current_db_)->second;
  TableInfo *table_info;
  DB_storage_engine->catalog_mgr_->GetTable(context->table_name_, table_info);
  pSyntaxNode col_identifier_node = ast->child_;
  string col_name;
  uint32_t col_id;

  while (col_identifier_node != nullptr) {
    col_name = col_identifier_node->val_;
    if (table_info->GetSchema()->GetColumnIndex(col_name, col_id) == DB_SUCCESS) {
      context->select_columns_.emplace_back(const_cast<Column *>(table_info->GetSchema()->GetColumn(col_id)));
      col_identifier_node = col_identifier_node->next_;
    } else {
      return DB_COLUMN_NAME_NOT_EXIST;
    }
  }
  return DB_SUCCESS;
}
dberr_t ExecuteEngine::ExecuteConnector(pSyntaxNode ast, ExecuteContext *context) {
  std::set<RowId> temp_table_1, temp_table_2;
  dberr_t res;
  // 尝试获取两张临时表
  res = ExecuteInternalNode(ast->child_, context);
  if (res == DB_SUCCESS) {
    temp_table_1 = context->internal_result_rid_;
  } else {
    return res;
  }
  res = ExecuteInternalNode(ast->child_->next_, context);
  if (res == DB_SUCCESS) {
    temp_table_2 = context->internal_result_rid_;
  } else {
    return res;
  }
  // 进行and或or操作
  string connector = ast->val_;
  auto iter_1 = temp_table_1.begin();
  auto iter_2 = temp_table_2.begin();
  context->internal_result_rid_.clear();
  if (connector == "and") {
    while (iter_1 != temp_table_1.end() || iter_2 != temp_table_2.end()) {
      if (iter_1 == temp_table_1.end()) {
        iter_2++;
      } else if (iter_2 == temp_table_2.end()) {
        iter_1++;
      } else if (*iter_1 == *iter_2) {
        context->internal_result_rid_.insert(*iter_1);
        iter_1++;
        iter_2++;
      } else if (*iter_1 < *iter_2) {
        iter_1++;
      } else {
        iter_2++;
      }
    }
  } else if (connector == "or") {
    while (iter_1 != temp_table_1.end() || iter_2 != temp_table_2.end()) {
      if (iter_1 == temp_table_1.end() || (*iter_2 < *iter_1)) {
        context->internal_result_rid_.insert(*iter_2);
        iter_2++;
      } else if (iter_2 == temp_table_2.end() || (*iter_1 < *iter_2)) {
        context->internal_result_rid_.insert(*iter_1);
        iter_1++;
      } else if (*iter_1 == *iter_2) {
        context->internal_result_rid_.insert(*iter_1);
        iter_1++;
        iter_2++;
      }
    }
  }
  return DB_SUCCESS;
}
