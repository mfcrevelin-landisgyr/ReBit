#ifndef TABLE_FILE_H
#define TABLE_FILE_H

namespace fs = std::filesystem;

struct TableFile {
	TableFile(const fs::path& path):file_path(path){}
	void save(){}
	fs::path file_path;
};
#endif // TABLE_FILE_H
