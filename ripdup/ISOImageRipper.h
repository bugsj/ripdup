#pragma once

#include <Windows.h>
#include <vector>

class ISOImageRipper
{
private:
	HANDLE m_isofile = nullptr;
	std::vector<unsigned char> m_headbuf;
	
	std::vector<unsigned long long> m_filerec;
	std::vector<long long> m_filelink;
	std::vector<unsigned int> m_filecrc32;
	std::vector<std::vector<char>> m_filefullname;

	long long m_filesize = 0;
	long long m_filecount = 0;

public:
	~ISOImageRipper() {
		closefile();
	}
	int open(const TCHAR* filename);
	int scanDir(unsigned long long lba, unsigned long long size, std::vector<char>* fullname);
	int scan();
	int checkDup();
	int write(const TCHAR* filename);
	unsigned long long getFileLBA(unsigned long long offset) {
		return *(unsigned int*)(m_headbuf.data() + offset + 2);
	}	
	unsigned long long getFileSize(unsigned long long offset) {
		return *(unsigned int*)(m_headbuf.data() + offset + 10);
	}
	void getFileName(unsigned long long offset, std::vector<char>* filename) {
		unsigned char* buf = m_headbuf.data();
		unsigned char isDir = buf[offset + 25] & 0x2;
		long long filenameLength = buf[offset + 32];
		if (buf[offset + 33 + filenameLength - 1] == ';') {
			filenameLength -= 2;
		}
		filename->resize(filenameLength + 1);
		memcpy(filename->data(), buf + offset + 33, filenameLength);

		if ((!isDir) && (filename->data()[0] == '_')) {
			filename->data()[0] = '.';
		}
		if ((!isDir) && (filename->data()[filenameLength - 1] == '.')) {
			filename->data()[filenameLength - 1] = '\0';
		}
		else {
			filename->data()[filenameLength] = '\0';
		}
	}
	unsigned int crc32(unsigned long long lba, unsigned long long size);

	void closefile() {
		if (m_isofile != nullptr && m_isofile != INVALID_HANDLE_VALUE) {
			CloseHandle(m_isofile);
		}
		m_isofile = nullptr;
	}
};

