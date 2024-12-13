#ifndef TABLE_FILE_H
#define TABLE_FILE_H

#include <boost/text/transcode_iterator.hpp>
#include <boost/text/transcode_view.hpp>

namespace fs = std::filesystem;

struct TableFile {
	TableFile(const fs::path& path):file_path(path){}
	void save(){}


	bool ParseFile(const fs::path& path) {

	    std::ifstream file(path, std::ios::binary);
	    if (!file.is_open())
	        return false;

	    char ch;
	    while (file.get(ch)) {
	        if (ch < 32 || 126 < ch ) {
	            file.close();
	            return false;
	        }
	    }

	    file.close();
	    return true;
	}
	fs::path file_path;
private:

};
#endif // TABLE_FILE_H
