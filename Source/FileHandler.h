#ifndef TABLE_FILE_H
#define TABLE_FILE_H
struct FileHandlerError {
	FileHandlerError(const std::string& message, const std::string& error)
		: message(message), error(error), type(0) {}
	FileHandlerError(const std::string& message, int64_t line_num, int64_t at, int64_t from)
		: message(message), line_num(line_num), at(at), from(from), type(1) {}

	std::string message;
	std::string error;
	int64_t line_num;
	int64_t at;
	int64_t from;
	int8_t type;
};

struct FileHandler {
	FileHandler(const fs::path& file_path, const fs::path& output_directory):
		file_path(file_path),output_directory(output_directory) {load();}

public:
	void load() {
		std::thread t([&]() {
			std::scoped_lock<std::mutex> lock(operation_mutex);

			try{
				loaded = false;
				errors.clear();

				std::ifstream file(file_path, std::ios::ate);

				if (!file.is_open()) {
					errors.emplace_back("Failed to open file",std::system_category().message(errno));
					return;
				}

				std::streamsize size = file.tellg();
				file.seekg(0, std::ios::beg);
				file_content_buffer = std::make_unique<char[]>(size+1);

				file.read(file_content_buffer.get(), size);
				auto error_number = errno;
				file.close();

				if (error_number) {
					errors.emplace_back("Failed to read file",std::system_category().message(errno));
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
					if (file_content_buffer[mid] == 0)
						rside = mid;
					else
						lside = mid + 1;
				}

				file_content_size = lside+1; // +1 to "append" an extra 0x00 byte to make it a null-terminated string
				file_content = std::string_view(file_content_buffer.get(),file_content_size);

				loaded = true;
			} catch (const std::runtime_error& e) {
				errors.emplace_back("Runtime Error during Load",static_cast<std::string>(e.what()));
			}
		});
		t.detach();
	}

	void save(){}

private:
	bool loaded = false;

private:
	std::list<FileHandlerError> errors;

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
	uint8_t user_state = 0;

};
#endif // TABLE_FILE_H
