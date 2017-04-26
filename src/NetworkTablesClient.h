/*
 * NetworkTables.h
 *
 *  Created on: Apr 23, 2017
 *      Author: ubuntu
 */

#ifndef NETWORKTABLESCLIENT_H_
#define NETWORKTABLESCLIENT_H_

#include <ntcore.h>
#include "networktables/NetworkTable.h"
#include <string>

class NetworkTablesClient {
public:
	NetworkTablesClient();
	virtual ~NetworkTablesClient();
	std::shared_ptr<NetworkTable> getTable();
	void putData(llvm::StringRef, llvm::ArrayRef<double>);
	void setTableName(std::string);
	llvm::StringRef getTableName() {return llvm::StringRef(table_name);}
	double getData(llvm::StringRef data_name);
	void putRaw(llvm::StringRef data);

private:
	std::string table_name = "Vision";
	int team_number = 4607;
	std::shared_ptr<NetworkTable> table;
};

#endif /* NETWORKTABLESCLIENT_H_ */
