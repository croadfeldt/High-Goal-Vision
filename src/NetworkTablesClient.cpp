/*
 * NetworkTables.cpp
 *
 *  Created on: Apr 23, 2017
 *      Author: ubuntu
 */

#include "NetworkTablesClient.h"

NetworkTablesClient::NetworkTablesClient() {
	// Create the NetworkTables object in client mode.
	NetworkTable::SetClientMode();
	NetworkTable::SetTeam(team_number);
	this->table = this->getTable();
}

NetworkTablesClient::~NetworkTablesClient() {
	// TODO Auto-generated destructor stub
}

std::shared_ptr<NetworkTable> NetworkTablesClient::getTable() {
	return NetworkTable::GetTable(table_name);
}

void NetworkTablesClient::putData(llvm::StringRef data_name, llvm::ArrayRef<double> data) {
	table->PutNumberArray(data_name, data);
}

void NetworkTablesClient::setTableName(std::string tn) {
	this->table_name = tn;
	NetworkTable::GetTable(this->getTableName());
}

bool NetworkTablesClient::GetBoolean(llvm::StringRef name) {
	return(table->GetBoolean(name, false));
}

void NetworkTablesClient::PutBoolean(llvm::StringRef name, bool value) {
	table->PutBoolean(name, value);
}

double NetworkTablesClient::getData(llvm::StringRef data_name) {
	return(table->GetNumber(data_name,-1));
}

void NetworkTablesClient::putRaw(llvm::StringRef name, llvm::StringRef data){
	table->PutRaw(name, data);
}
