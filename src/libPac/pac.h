#pragma once
#include "defines.h"

#include <list>
#include <memory>

#include "filesourcebase.h"
#include <map>

namespace lib_pac
{
	class pac_archive
	{
	private:
		std::map<std::string, std::shared_ptr<file_source_base>> m_entries;

	public:
		class iterator : public std::iterator<std::output_iterator_tag, std::string>
		{
		private:
			std::map<std::string, std::shared_ptr<file_source_base>>::iterator m_iter;
		public:
			explicit iterator(std::map<std::string, std::shared_ptr<file_source_base>>::iterator& base);
			EXPORTS std::string operator*() const;
			EXPORTS iterator& operator++();
			EXPORTS bool operator!=(const iterator& rhs) const;
		};

		struct progress_info
		{
			progress_info(int cur_file, int num_files, const std::string& file_name, uint32_t raw_size,
			              uint32_t compressed_size);

			int cur_file;
			int num_files;
			const std::string& file_name;
			uint32_t raw_size;
			uint32_t compressed_size;
		};

		struct archive_info
		{
			uint32_t header_size = 0;
			uint32_t total_files = 0;
			uint32_t original_size = 0;
			uint32_t compressed_size = 0;
		};

		typedef void (*progress_callback)(const progress_info& info);

		EXPORTS size_t num_files() const;
		EXPORTS iterator begin();
		EXPORTS iterator end();

		EXPORTS bool remove(const std::string& file);
		EXPORTS void insert(const std::string& virt_path, std::shared_ptr<file_source_base> ptr);
		EXPORTS std::shared_ptr<file_source_base> get(const std::string& file);

		EXPORTS explicit pac_archive(std::wstring file);
		EXPORTS pac_archive();
		EXPORTS archive_info save(std::wstring file, progress_callback callback = nullptr) const;
	};
}
