#ifndef TABLE_FILE_H
#define TABLE_FILE_H

namespace fs = std::filesystem;

struct FileHandler {
	FileHandler(const fs::path& file_path, const fs::path& output_directory):
		file_path(file_path),output_directory(output_directory) {}

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

private:
	const fs::path& output_directory;
	fs::path file_path;
};
#endif // TABLE_FILE_H
