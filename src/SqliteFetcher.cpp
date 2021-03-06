/*
 * CppSqlParser.cpp
 *
 * Copyright (C) 2020 Taishi Ueda <taishi.ueda@gmail.com>
 *
 * Distributed under terms of the MIT license.
 * http://opensource.org/licenses/mit-license.php
 */

#include "SqliteFetcher.hpp"
#include <sstream>
#include <algorithm>

namespace sf{

    //##############################################################
    // Data
    //---------------------------------------------------------
    Data::Data(){}

    //---------------------------------------------------------
    Data::Data(sql_types::TypeStr_t type,
		    const KeyFlag_t& flg)
	:key_flg_(flg), type_str_(type){
	    setType(type);
    }

    //---------------------------------------------------------
    Data::Data(const Type_t& type, const KeyFlag_t& flg)
    :key_flg_(flg) ,type_(type) {
	setType(type);
    }
    //---------------------------------------------------------
    Data::Data(const std::string& value,
		    sql_types::TypeStr_t type, const KeyFlag_t& flg)
	:key_flg_(flg), type_str_(type){
	    setType(type);
	    set(type_, value);
	}

    //---------------------------------------------------------
    void Data::setType(sql_types::TypeStr_t type){
        //PRIMARY_KEY constraint can be set to only INTEGER type
	if((key_flg_ & PRIMARY_KEY) != 0u){
            type_str_ = "INTEGER";
	    type_ = INT64;
	    return;
	}
        
	//separate by white space
	std::istringstream iss(type);
	std::vector<std::string> words;
	std::string a_word;
	iss >> a_word;

	//delete cascade from type
	size_t cpos = a_word.find('(');
	if(cpos != std::string::npos){
	    a_word = a_word.substr(0u,cpos);
	}

	if(TypeDef.find(a_word) != TypeDef.end()){
	    type_ = TypeDef.at(a_word);
	}
	else{
	    type_ = BLOB;
	}
    }
    //---------------------------------------------------------
    void Data::setType(Type_t type){
	if((key_flg_ & PRIMARY_KEY) != 0u){
            type_str_ = "INTEGER";
	    type_ = INT64;
	    return;
	}
	switch(type){
	    case NONE:
		type_str_ = "NONE";
		break;
	    case INT8:
		type_str_ = "TYNYINT";
		break;
	    case INT16:
		type_str_ = "SMALLINT";
		break;
	    case INT32:
		type_str_ = "MEDIUMINT";
		break;
	    case INT64:
		type_str_ = "BIGINT";
		break;
	    case UINT64:
		type_str_ = "UINT64";
		break;
	    case FLOAT:
		type_str_ = "FLOAT";
		break;
	    case DOUBLE:
		type_str_ = "DOUBLE";
		break;
	    case BOOL:
		type_str_ = "BOOL";
		break;
	    case TEXT:
		type_str_ = "TEXT";
		break;
	    case BLOB:
		type_str_ = "BLOB";
		break;
	}
    }

    //---------------------------------------------------------
    sql_types::TypeStr_t Data::typeStr(const bool& print_flags) const{
	std::string ret = type_str_;

	if(print_flags){
	    if((key_flg_ & PRIMARY_KEY) != 0u){
		ret += " PRIMARY KEY";
	    }
	    if((key_flg_ & UNIQUE) != 0u){
		ret += " UNIQUE";
	    }
	    if((key_flg_ & AUTO_INCREMENT) != 0u){
		ret += " AUTO INCREMENT";
	    }
	    if((key_flg_ & NOT_NULL) != 0u){
		ret += " NOT NULL";
	    }
	    if((key_flg_ & DEFAULT) != 0u){
		ret += " DEFAULT " + str();
	    }
	}

	return ret;
    }

    
    const KeyFlag_t& Data::flags() const{
	return key_flg_;
    }

    //---------------------------------------------------------
    bool Data::get(void* value_ptr, const Type_t& type) const{
	if(type_ == type){
	    for(size_t k=0u; k<data_.size(); ++k){
		reinterpret_cast<uint8_t*>(value_ptr)[k] = data_[k];
	    }
	    return true;
	}
	else{
	    return false;
	}
    }

    //---------------------------------------------------------
    void Data::set(void* value_ptr, const Type_t& type, const uint32_t& size){
	this->data_.resize(size);
	for(size_t k=0u; k<data_.size(); ++k){
	    this->data_[k] = reinterpret_cast<uint8_t*>(value_ptr)[k];
	}
	this->type_ = type;
	
    }

    //---------------------------------------------------------
    bool Data::change(void* value_ptr, const Type_t& type, const uint32_t& size){
	if(this->type_ == type){
	    this->set(value_ptr, type, size);
	    return true;
	}
	else{
	    return false;
	}

    }
    //---------------------------------------------------------
    template<>
    bool Data::get(int8_t& value) const{
	return get(&value, INT8);
    }
    //---------------------------------------------------------
    template<>
    bool Data::get(int16_t& value) const{
	return get(&value, INT16);
    }
    //---------------------------------------------------------
    template<>
    bool Data::get(int32_t& value) const{
	return get(&value, INT32);
    }
    //---------------------------------------------------------
    template<>
    bool Data::get(int64_t& value) const{
	return get(&value, INT64);
    }
    //---------------------------------------------------------
    template<>
    bool Data::get(uint64_t& value) const{
	return get(&value, UINT64);
    }
    //---------------------------------------------------------
    template<>
    bool Data::get(float& value) const{
	return get(&value, FLOAT);
    }
    //---------------------------------------------------------
    template<>
    bool Data::get(double& value) const{
	return get(&value, DOUBLE);
    }
    //---------------------------------------------------------
    template<>
    bool Data::get(std::string& value) const{
	if(this->type_ == TEXT){
	    value.clear();
	    for(size_t k=0u; k<this->data_.size(); ++k){
		value.push_back(static_cast<char>(this->data_[k]));
	    }
	    return true;
	}
	else{
	    return false;
	}
    }
    //---------------------------------------------------------
    template<>
    bool Data::get(bool& value) const{
	if(this->type_ == BOOL){
	    for(size_t k=0u; k<data_.size(); ++k){
		reinterpret_cast<uint8_t*>(&value)[k] = data_[k];
	    }
	    return true;
	}
	else{
	    return false;
	}
    }
    //---------------------------------------------------------
    template<>
    bool Data::get(Binary_t& value) const{
	if(this->type_ == BLOB){
	    value = this->data_;
	    return true;
	}
	else{
	    return false;
	}
    }

    //---------------------------------------------------------
    template<>
    void Data::set(int8_t value){
	set(&value, INT8, sizeof(value));
    }
    //---------------------------------------------------------
    template<>
    void Data::set(int16_t value){
	set(&value, INT16, sizeof(value));
    }
    //---------------------------------------------------------
    template<>
    void Data::set(int32_t value){
	set(&value, INT32, sizeof(value));
    }
    //---------------------------------------------------------
    template<>
    void Data::set(int64_t value){
	set(&value, INT64, sizeof(value));
    }
    //---------------------------------------------------------
    template<>
    void Data::set(uint64_t value){
	set(&value, UINT64, sizeof(value));
    }
    //---------------------------------------------------------
    template<>
    void Data::set(bool value){
	set(&value, BOOL, sizeof(value));
    }
    //---------------------------------------------------------
    template<>
    void Data::set(float value){
	set(&value, FLOAT, sizeof(value));
    }
    //---------------------------------------------------------
    template<>
    void Data::set(double value){
	set(&value, DOUBLE, sizeof(value));
    }
    //---------------------------------------------------------
    template<>
    void Data::set(std::string value){
	this->data_.clear();
	for(size_t k=0u; k<value.size(); ++k){
	    this->data_.push_back(static_cast<uint8_t>(value[k]));
	}
	this->type_ = TEXT;
    }
    void Data::set(const char* value){
	std::string str(value);
	set(str);
    }

    //---------------------------------------------------------
    template<>
    void Data::set(Binary_t value){
	this->data_ = value;
	this->type_ = BLOB;
    }
    //---------------------------------------------------------
    template<>
    bool Data::change(const int8_t& value){
	if(this->type_ == INT8){
	    this->set(value);
	    return true;
	}
	else{
	    return false;
	}
    }
    //---------------------------------------------------------
    template<>
    bool Data::change(const int16_t& value){
	if(this->type_ == INT16){
	    this->set(value);
	    return true;
	}
	else{
	    return false;
	}
    }
    //---------------------------------------------------------
    template<>
    bool Data::change(const int32_t& value){
	if(this->type_ == INT32){
	    this->set(value);
	    return true;
	}
	else{
	    return false;
	}
    }
    //---------------------------------------------------------
    template<>
    bool Data::change(const int64_t& value){
	if(this->type_ == INT64){
	    this->set(value);
	    return true;
	}
	else{
	    return false;
	}
    }
    //---------------------------------------------------------
    template<>
    bool Data::change(const uint64_t& value){
	if(this->type_ == UINT64){
	    this->set(value);
	    return true;
	}
	else{
	    return false;
	}
    }
    //---------------------------------------------------------
    template<>
    bool Data::change(const float& value){
	if(this->type_ == FLOAT){
	    this->set(value);
	    return true;
	}
	else{
	    return false;
	}
    }
    //---------------------------------------------------------
    template<>
    bool Data::change(const double& value){
	if(this->type_ == DOUBLE){
	    this->set(value);
	    return true;
	}
	else{
	    return false;
	}
    }
    //---------------------------------------------------------
    template<>
    bool Data::change(const bool& value){
	if(this->type_ == BOOL){
	    this->set(value);
	    return true;
	}
	else{
	    return false;
	}
    }
    //---------------------------------------------------------
    template<>
    bool Data::change(const std::string& value){
	if(this->type_ == TEXT){
	    this->set(value);
	    return true;
	}
	else{
	    return false;
	}
    }

    bool Data::change(const char* value){
	std::string str(value);
	return change(str);
    }

    //---------------------------------------------------------
    template<>
    bool Data::change(const Binary_t& value){
	if(this->type_ == BLOB){
	    this->set(value);
	    return true;
	}
	else{
	    return false;
	}
    }


    //---------------------------------------------------------
    std::string Data::str()const{
	std::string ret;
	switch(type_){
	    case NONE:{
			  ret = "";
			  break;
		      }
	    case BOOL:{
			  bool value_bool = 0;
			  this->get(value_bool);
			  ret = std::to_string(value_bool);
			  break;
		      }
	    case INT8:{
			  int8_t value_int8 = 0;
			  this->get(value_int8);
			  ret = std::to_string(value_int8);
			  break;
		      }
	    case INT16:{
			   int16_t value_int16 = 0;
			   this->get(value_int16);
			   ret = std::to_string(value_int16);
			   break;
		       }
	    case INT32:{
			   int32_t value_int32 = 0;
			   this->get(value_int32);
			   ret = std::to_string(value_int32);
			   break;
		       }
	    case INT64:{
			   int64_t value_int64 = 0;
			   this->get(value_int64);
			   ret = std::to_string(value_int64);
			   break;
		       }
	    case UINT64:{
			   uint64_t value_uint64 = 0;
			   this->get(value_uint64);
			   ret = std::to_string(value_uint64);
			   break;
		       }
	    case FLOAT:{
			  float value_float = 0.0f;
			  this->get(value_float);
			  ret = std::to_string(value_float);
			  break;
		      }
	    case DOUBLE:{
			  double value_real = 0.0;
			  this->get(value_real);
			  ret = std::to_string(value_real);
			  break;
		      }
	    case TEXT:{
			  std::string value_text;
			  this->get(value_text);
			  ret = "'" + value_text + "'";
			  break;
		      }
	    case BLOB:{
			  Binary_t value_blob;
			  this->get(value_blob);
			  char buff[2*value_blob.size()+1];
			  for(size_t k=0u; k<value_blob.size(); ++k){
			      sprintf(&buff[2u*k],"%02x",value_blob[k]);
			  }
			  buff[2*value_blob.size()] = '\0';
			  ret = "X'" + std::string(buff) + "'";
			  break;
		      }
	}


	return ret;
    }

    const Type_t& Data::type() const{
	return type_;
    }

    //---------------------------------------------------------
    void Data::set(const Type_t& type, const std::string& dflt_str){
	switch(type){
	    case NONE:{
			  break;
		      }
	    case BOOL:{
			  bool value_bool 
			      = static_cast<bool>(std::stoi(dflt_str));
			  this->set(value_bool);
			  break;
		      }
	    case INT8:{
			  int8_t value_int8_t 
			      = static_cast<int8_t>(std::stoi(dflt_str));
			  this->set(value_int8_t);
			  break;
		      }
	    case INT16:{
			  int16_t value_int16_t 
			      = static_cast<int16_t>(std::stoi(dflt_str));
			  this->set(value_int16_t);
			   break;
		       }
	    case INT32:{
			  int32_t value_int32_t 
			      = static_cast<int32_t>(std::stoi(dflt_str));
			  this->set(value_int32_t);
			   break;
		       }
	    case INT64:{
			  int64_t value_int64_t 
			      = static_cast<int64_t>(std::stoi(dflt_str));
			  this->set(value_int64_t);
			   break;
		       }
	    case UINT64:{
			  uint64_t value_uint64_t 
			      = static_cast<uint64_t>(std::stoi(dflt_str));
			  this->set(value_uint64_t);
			   break;
		       }
	    case FLOAT:{
			  float value_float 
			      = static_cast<float>(std::stof(dflt_str));
			  this->set(value_float);
			  break;
		      }
	    case DOUBLE:{
			  double value_double 
			      = static_cast<double>(std::stod(dflt_str));
			  this->set(value_double);
			  break;
		      }
	    case TEXT:{
			  this->set(dflt_str);
			  break;
		      }
	    case BLOB:{
			  Binary_t value_blob;
			  for(size_t k=0u; 2u*k+3u < dflt_str.length()-1u; ++k){
			      uint8_t elm;
			      std::string elm_str;
			      elm_str.push_back(dflt_str[2u*k+2u]);
			      elm_str.push_back(dflt_str[2u*k+3u]);
			      elm = static_cast<uint8_t>(std::stoi(elm_str, nullptr, 16));
			      value_blob.push_back(elm);
			  }
			  this->set(value_blob);
			  break;
		      }
	}
    }


    //########################################################################
    // Fetcher
    // Constructor
    Fetcher::Fetcher(){}

    //-------------------------------------------------------------------
    // Constructor. Open database.
    Fetcher::Fetcher(const std::string& db_name){
       int32_t retval = sqlite3_open(db_name.c_str(), &db_ptr_);
       if(retval != SQLITE_OK){
	   last_err_ = sqlite3_errstr(retval);
       }
       else{
	   is_opened_ = true;
	   last_table_info_ = getTableInfo(last_err_);
       }
    } 

    //-------------------------------------------------------------------
    // Open database.
    int32_t Fetcher::open(const std::string& db_name, std::string& err_msg,
	    const int32_t& flags, const char* zVfs){
       int32_t retval 
	   = sqlite3_open_v2(db_name.c_str(), &db_ptr_, flags, zVfs);
       if(retval != SQLITE_OK){
	   this->last_err_ = sqlite3_errstr(retval);
       }
       else{
	   this->is_opened_ = true;
	   last_table_info_ = getTableInfo(last_err_);
       }
       return retval;
    }

    //-------------------------------------------------------------------
    int32_t Fetcher::close(std::string err_msg){
	err_msg = "";
	int32_t retval 
	    = sqlite3_close(&db_ptr_);
	if(retval != SQLITE_OK){
	    this->last_err_ = sqlite3_errstr(retval);
	    err_msg = this->last_err_;
	}
	return retval;
    }

    //-------------------------------------------------------------------
    static int execCallback(void *result_ptr, int argc, char **argv, char **col_name){
	ResultElement_t a_res;
        for(int k=0; k<argc; ++k){
	    if(argv[k] == nullptr || argv[k][0] == '\0'){
		a_res[col_name[k]] = "";
	    }
	    else{
		a_res[col_name[k]] = argv[k];
	    }
	}
	reinterpret_cast<ExecResult_t*>(result_ptr)->result.push_back(a_res);
	return 0;
    }

    //-------------------------------------------------------------------
    // Execute SQLite query
    ExecResult_t Fetcher::exec(const std::string& query, std::string& err_msg){
	err_msg.clear();
	char *err_char = 0;
	last_exec_result_.in_sql = query;
	last_exec_result_.result.clear();
        int32_t ret = sqlite3_exec(db_ptr_, query.c_str(), 
		execCallback, &last_exec_result_, &err_char);
	if(ret != SQLITE_OK){
	    err_msg = err_char;
	    sqlite3_free(err_char);
	}

	if(to_info_update_){
	    to_info_update_ = false;
	    last_table_info_ = getTableInfo(last_err_);
	}
	return last_exec_result_;
    }

    //-------------------------------------------------------------------
    // Execute SQLite query
    std::list<ExecResult_t> Fetcher::execSeparate(const std::string& query, std::string& err_msg){
	err_msg.clear();
	std::list<ExecResult_t> ret_list;
	size_t i_begin = 0u;
	size_t i_dlm = 0u;
	while(i_begin < query.size()){
	    i_dlm = query.find(";",i_begin);
	    if(i_dlm == std::string::npos){
		i_dlm=query.size()-1u;
	    }
	    auto a_query = query.substr(i_begin,i_dlm-i_begin);
	    auto a_res = exec(a_query, err_msg);
	    if(!err_msg.empty()){
		break;
	    }
	    else{
		ret_list.push_back(a_res);
	    }
	    i_begin = i_dlm+1u;
	}
	return ret_list;
    }

    //-------------------------------------------------------------------
    // Dump result of exec function into string.
    std::string Fetcher::dump(const ExecResult_t& res) const{
	std::string ret = "{\"Input\":\"" + res.in_sql + "\",\n";
	auto i_res = res.result.begin();
	auto i_res_end = res.result.end();
	ret += "\"results\":[";
	bool is_first1 = false;
	for(;i_res != i_res_end; ++i_res){
	    if(!is_first1){
		ret += ",\n ";
	    }else{
		is_first1 = false;
	    }
	    ret += "{";
	    auto i_elem = i_res->begin();
	    auto i_elem_end = i_res->end();
	    bool is_first = true;
	    for(;i_elem != i_elem_end; ++i_elem){
		if(!is_first){
		    ret += ", ";
		}else{
		    is_first = false;
		}
		ret += "\"" + i_elem->first + "\":\"" + i_elem->second + "\"";
	    }
	    ret += "}";
	}
	ret += "]}\n";
	return ret;
    }

    //-------------------------------------------------------------------
    //! Dump result of exec function into string.
    std::string Fetcher::dump(const std::list<ExecResult_t>& res_list) const{
	auto i_res_end = res_list.end();
	std::string ret = "{\"exec_result\": [";
	bool is_first=true;
	for(auto i_res =res_list.begin(); i_res != i_res_end; ++i_res){
	    if(!is_first){
		ret += ",";
	    }
	    else{
		is_first = false;
	    }
	    ret += dump(*i_res);
	}
	ret += "]}\n";
	return ret;
    }

    //-------------------------------------------------------------------
    // Fetch column list from result of executed query for SELECT.
    ColumnList_t Fetcher::fetchColumn(const std::string& query, std::string& err_msg){
	ColumnList_t col;
	err_msg.clear();
	
	//separate query by white space
	std::istringstream iss(query);
	std::vector<std::string> words;
	std::string a_word;
	while (iss >> a_word) {
	    //delete comma
	    size_t comma_pos = a_word.find(",");
	    if(comma_pos != std::string::npos){
		a_word.erase(comma_pos,1);
	    }
	    size_t colon_pos = a_word.find(";");
	    if(colon_pos != std::string::npos){
		a_word.erase(colon_pos,1);
	    }
	    words.push_back(a_word);
	}

	//get table name from query
	auto i_select = std::find_if(words.begin(),words.end(),
		[](const std::string w){ return ( w == "select" || w == "SELECT");});
	if(i_select == words.end()){
	    err_msg = "Query doen't include 'FROM' statement";
	    return col;
	}
	//get table name from query
	auto i_from = std::find_if(words.begin(),words.end(),
		[](const std::string w){ return ( w == "from" || w == "FROM");});
	if(i_from == words.end()){
	    err_msg = "Query doen't include 'FROM' statement";
	    return col;
	}

	auto i_table_info = last_table_info_.find(*std::next(i_from));
	if(i_table_info == last_table_info_.end()){
	    err_msg = "No such a table: " + *std::next(i_from);
	    return col;
	}

	const Column_t& table_col = i_table_info->second;

	//if all rows are selected
	auto i_all = std::find_if(i_select, i_from,
		[](const std::string w){ return  w == "*";});

	//create new query to quate BLOB data
	std::string new_query = "";
	for(auto i_word=words.begin(); i_word != std::next(i_select); ++i_word){
	    new_query += *i_word + " ";
	}

	bool is_changed = false;
	if(i_all != i_from){
	    auto i_col_end = table_col.end();
	    for(auto i_col = table_col.begin(); i_col != i_col_end; ++i_col){
		if(i_col->second.type() == BLOB){
		    new_query += "quote(" + i_col->first + ")";
		    is_changed = true;
		}
		else{
		    new_query += i_col->first;
		}

		if(std::next(i_col) != i_col_end){
		    new_query += ", ";
		}
	    }
	}
	//If specific rows are selected
	else{
	    for(auto i_word=std::next(i_select); i_word != i_from; ++i_word){
		auto i_row = table_col.find(*i_word);
		if(i_row != table_col.end()){
		    if(i_row->second.type() == BLOB){
			new_query += "quote(" + i_row->first + ")";
			is_changed = true;
		    }
		    else{
			new_query += i_row->first;
		    }

		    if(std::next(i_word) != i_from){
			new_query += ", ";
		    }
		}
		else{
		    err_msg = "Coudn't find " + i_row->first + " in table " +  *std::next(i_from);
		    return col;
		}
	    }
	}

	if(!is_changed){
	    new_query = query;
	}
	else{
	    //add rest of queries
	    auto i_words_end = words.end();
	    for(auto i_word = i_from; i_word != i_words_end; ++i_word){
		new_query += " " + *i_word;
	    }
	}
	new_query += ";";

	ExecResult_t res = exec(new_query, err_msg);
	if(!err_msg.empty()){
	    return col;
	}
	
	auto i_res_end = res.result.end();
	for(auto i_res = res.result.begin(); i_res != i_res_end; ++i_res){
	    auto i_elm_end = i_res->end();
	    Column_t a_col;
	    for(auto i_elm = i_res->begin(); i_elm != i_elm_end; ++i_elm){
		//remove quote()
		std::string keyword;
		if(i_elm->first.find("quote(") != std::string::npos){
		    keyword = i_elm->first.substr(6u,i_elm->first.length()-7u);
		}
		else{
		    keyword = i_elm->first;
		}
		auto i_data = table_col.find(keyword);
		if(i_data == table_col.end()){
		    err_msg = "Coudn't find " + keyword + " in table " +  *std::next(i_from);
		}
		a_col[keyword] = Data(i_elm->second, 
			i_data->second.typeStr(), i_data->second.flags());
	    }
	    col.push_back(a_col);
	}

	return col;
    }

    //-------------------------------------------------------------------
    // Get master table.
    TableInfo_t Fetcher::getTableInfo(std::string& err_msg){
	err_msg.clear();
	ExecResult_t res = exec("SELECT * FROM sqlite_master;",err_msg);
	TableInfo_t table_info;
	if(!err_msg.empty()){
	    return table_info;
	}
	auto i_res_end = res.result.end();
	for(auto i_res = res.result.begin(); i_res != i_res_end; ++i_res){
	    if(i_res->at("type") == "table"){
		std::string tbl_name = i_res->at("name");
		Column_t a_col = getTableInfo(tbl_name, err_msg);
		if(!err_msg.empty()){
		    break;
		}
		else{
		    table_info[tbl_name] = a_col;
		}
	    }
	}
	return table_info;
    }
    
    //-------------------------------------------------------------------
    Column_t Fetcher::getTableInfo(const std::string& table_name, std::string& err_msg){
	err_msg.clear();
	ExecResult_t res = exec("PRAGMA table_info(" + table_name + ");", err_msg);
	Column_t table_info;
	if(!err_msg.empty()){
	    return table_info;
	}
	auto i_res_end = res.result.end();
	bool has_dflt = false;
	KeyFlag_t flg = NORMAL;
	for(auto i_res = res.result.begin(); i_res != i_res_end; ++i_res){
	    has_dflt = false;
	    flg = NORMAL;
	    if(i_res->at("dflt_value") != ""){
		has_dflt = true;
	    }
	    if(i_res->at("pk") == "1"){
		flg |= PRIMARY_KEY;
	    }
	    if(i_res->at("notnull") == "1"){
		flg |= NOT_NULL;
	    }

	    if(has_dflt){
		table_info[i_res->at("name")]
		    = Data(i_res->at("dflt_value"), i_res->at("type"),  flg);
	    }
	    else{
		table_info[i_res->at("name")]
		    = Data(i_res->at("type"), flg);
	    }
	}
	return table_info;
    }

    //-------------------------------------------------------------------
    // Generate queries to create table from a table info.
    std::string Fetcher::genQueryCreate(const TableInfo_t& table_info, std::string& err_msg){
	err_msg.clear();
	std::string ret;
	auto i_table_end = table_info.end();
	for(auto i_table=table_info.begin(); i_table != i_table_end; ++i_table){
	    //If this is a new table, update last_table_info_
	    if(last_table_info_.find(i_table->first) == i_table_end){
		last_table_info_[i_table->first] = i_table->second;
	    }
	    ret += "CREATE TABLE " + i_table->first + "(";
	    auto i_col_end = i_table->second.end();
	    bool is_first = true;
	    for(auto i_col = i_table->second.begin(); i_col!=i_col_end; ++i_col){
		if(!is_first){
		    ret += ", ";
		}
		else{
		    is_first = false;
		}
		ret += i_col->first + " " + i_col->second.typeStr();
	    }
	    ret += "); ";
	}
	to_info_update_ = true;
	return ret;
    }

    //-------------------------------------------------------------------
    // Generate queries to create table from a table info.
    std::string Fetcher::genQueryCreate(const Table_t& table, std::string& err_msg){
	err_msg.clear();
	std::string ret;
	TableInfo_t a_table_info;
	auto i_table_end = table.end();
	for(auto i_table=table.begin(); i_table!=i_table_end; ++i_table){
	    TableInfo_t a_table_info;
	    a_table_info[i_table->first] = i_table->second.front();
	    ret += genQueryCreate(a_table_info, err_msg);
	    if(!err_msg.empty()){
		break;
	    }
	    else{
		ret += genQueryInsert(i_table->first, i_table->second, err_msg);
	    }

	    if(!err_msg.empty()){
		break;
	    }
	}

	return ret;
    }

    //-------------------------------------------------------------------
    // Generate a query to create table from a column list.
    std::string Fetcher::genQueryCreate(const std::string& name,
	    const ColumnList_t& column_list, std::string& err_msg){
	err_msg.clear();
	std::string ret;
	Table_t a_table;
	a_table[name] = column_list;
	return genQueryCreate(a_table, err_msg);
    }


    //-------------------------------------------------------------------
    // Generate a query to create table from a column list.
    std::string Fetcher::genQueryCreate(const std::string& name,
	    const Column_t& column, std::string& err_msg){
	err_msg.clear();
	std::string ret;
	TableInfo_t a_table_info;
	a_table_info[name] = column;
	ret = genQueryCreate(a_table_info, err_msg);
	ret += genQueryInsert(name, column, err_msg);
	return ret;
    }

    //-------------------------------------------------------------------
    // Generate a query to insert a column.
    std::string Fetcher::genQueryInsert(const std::string& table_name,
	    const Column_t& col, std::string& err_msg){
	err_msg.clear();
	std::string ret = "INSERT INTO " + table_name + "(";
	auto i_col_end = col.end();
	bool is_first = true;
	for(auto i_col = col.begin(); i_col != i_col_end; ++i_col){
	    if(!is_first){
		ret += ", ";
	    }
	    else{
		is_first = false;
	    }
	    ret += i_col->first;
        }
	ret += ") VALUES(";
	is_first = true;
	for(auto i_col = col.begin(); i_col != i_col_end; ++i_col){
	    if(!is_first){
		ret += ", ";
	    }
	    else{
		is_first = false;
	    }
	    ret += i_col->second.str();
        }
	ret += "); ";
	return ret;
    }

    //-------------------------------------------------------------------
    //! Generate a query to insert a column.
    std::string Fetcher::genQueryInsert(const std::string& table_name,
	    const ColumnList_t& col, std::string& err_msg){
	err_msg.clear();
	auto i_col_end = col.end();
	std::string ret;
	for(auto i_col = col.begin(); i_col != i_col_end; ++i_col){
	    ret += genQueryInsert(table_name, *i_col, err_msg);
	    if(!err_msg.empty()){
		break;
	    }
	}
	return ret;

    }

    //-------------------------------------------------------------------
    // Generate a query to update a column.
    std::string Fetcher::genQueryUpdate(const std::string& table_name,
	    const Column_t& col, std::string& err_msg){
	err_msg.clear();
	std::string ret = "UPDATE "+ table_name +" SET ";
	bool has_id=false;
	bool is_first = true;
	auto i_col_end = col.end();
	std::string key_str;
	for(auto i_col=col.begin(); i_col != i_col_end; ++i_col){
	    if((i_col->second.flags() & PRIMARY_KEY) != 0u){
		key_str = " WHERE " + i_col->first
		    + " = " + i_col->second.str();
		has_id = true;
	    }
	    else{
		if(!is_first){
		    ret += ", ";
		}
		else{
		    is_first = false;
		}
		ret += i_col->first + " = " + i_col->second.str();
	    }
	}

	if(has_id){
	    ret += key_str;
	}

	return ret;
    }
}
