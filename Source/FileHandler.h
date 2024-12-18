#ifndef TABLE_FILE_H
#define TABLE_FILE_H

struct FileHandler {
	FileHandler(const fs::path& file_path, const fs::path& output_directory):
		file_path(file_path),output_directory(output_directory) {}

public:
	void load() {
		try{
			auto start = std::chrono::high_resolution_clock::now();

			std::ifstream file(file_path, std::ios::ate);

			if (!file.is_open()) {
				log("Failed to open file: \n\n" + std::system_category().message(errno) + "\n");
				load_failed = true;
				return;
			}

			std::streamsize size = file.tellg();
			file.seekg(0, std::ios::beg);
			file_content_buffer = std::make_unique<char[]>(size+1);

			bool read_load_failed = static_cast<bool>(file.read(file_content_buffer.get(), size));
			auto error_number = errno;
			file.close();

			if (error_number) {
				log("Failed to read file: " + std::to_string(error_number) + "\n\n" + std::system_category().message(error_number));
				load_failed = true;
				return;
			}

			/*
					The file system's block size or alignment often results in the buffer being slightly larger than the actual
				content of the file. This discrepancy is due to the `std::ios::ate` mode, which retrieves the end position
				based on the allocated space rather than the exact number of bytes in the file.

					To accurately determine the size of the file's content, a binary search is used to locate the first null
				byte (0x00) in the buffer. By filling the buffer with null bytes at instantiation and then loading the file content
				ontop, the oversized portion of the buffer beyond the file's actual data will be filled with null bytes.

					This corrected size ensures that only the valid portion of the buffer is used for further processing.
				Additionally, the resulting size is incremented by 1 to "append" an extra null byte, facilitating safe
				parsing of the content as a null-terminated string in subsequent operations.
			*/
			size_t rside = size ;
			size_t lside = 0;

			while (lside < rside) {
				size_t mid = lside + ((rside - lside)>>1);
				if (file_content_buffer[mid] == 0) {
					rside = mid;
				} else {
					lside = mid + 1;
				}
			}

			file_content_size = lside+1; // +1 to "append" an extra 0x00 byte to make it a null-terminated string
			file_content = std::string_view(file_content_buffer.get(),file_content_size);

			auto end = std::chrono::high_resolution_clock::now();

			log_time("Successfully loaded file into memory.",start,end);

		} catch (const std::runtime_error& e) {
			log("Runtime Error during Load:\n\n" + static_cast<std::string>(e.what()));
			load_failed = true;
		}   
	}

	void save(){}

public:
	std::queue<std::pair<std::string,uint64_t>> message_log;
	std::queue<std::pair<std::string,uint64_t>> error_log;

private:
	size_t file_content_size;
	std::string_view file_content;
	std::unique_ptr<char[]> file_content_buffer;
	std::vector<uint64_t> file_line_indices = {0};

private:
	const fs::path& output_directory;
	fs::path file_path;
	
private:
	std::mutex operation_mutex;

};
#endif // TABLE_FILE_H
